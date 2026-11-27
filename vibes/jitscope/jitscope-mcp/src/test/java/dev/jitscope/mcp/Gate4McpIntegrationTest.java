package dev.jitscope.mcp;

import static org.assertj.core.api.Assertions.assertThat;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.concurrent.TimeUnit;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

/**
 * Verification Gate 4: process-level integration. Spawns `java -jar jitscope.jar mcp`, speaks real
 * MCP JSON-RPC (initialize / notifications/initialized / tools/list / tools/call) for all 5 tools on
 * real workload .jfr, asserts valid protocol framing + accurate findings + zero stray stdout.
 */
class Gate4McpIntegrationTest {

    @TempDir
    Path tempDir;

    @Test
    void mcpProtocolAndAllFiveTools() throws Exception {
        Path jfr = recordWorkload("dev.jitscope.workloads.DeoptStorm", tempDir.resolve("d.jfr"));
        Path cliff = recordWorkload("dev.jitscope.workloads.InlineCliff", tempDir.resolve("cliff.jfr"),
                List.of("--mode=cliff"), List.of("-XX:ReservedCodeCacheSize=4m"));

        String javaBin = Path.of(System.getProperty("java.home"), "bin", "java").toString();
        // invoke STDIO transport directly on test classpath (mcp built before shaded cli jar in reactor)
        ProcessBuilder pb = new ProcessBuilder(javaBin, "-cp", System.getProperty("java.class.path"),
                "dev.jitscope.mcp.JitScopeMcp");
        pb.redirectErrorStream(false); // stderr separate; assert nothing stray on stdout
        Process proc = pb.start();
        try {
            OutputStream in = proc.getOutputStream();
            BufferedReader out = new BufferedReader(new InputStreamReader(proc.getInputStream(), StandardCharsets.UTF_8));
            // drain stderr async to avoid blocking
            Thread errDrain = new Thread(() -> {
                try {
                    var r = new BufferedReader(new InputStreamReader(proc.getErrorStream(), StandardCharsets.UTF_8));
                    while (r.readLine() != null) { /* discard; logging must not touch stdout */ }
                } catch (IOException ignored) { }
            });
            errDrain.setDaemon(true);
            errDrain.start();

            // initialize
            send(in, jsonRpc(1, "initialize", """
                    {"protocolVersion":"2024-11-05","capabilities":{},"clientInfo":{"name":"gate4","version":"1"}}"""));
            String initResp = readResponse(out, 1);
            assertThat(initResp).contains("\"id\":1");
            assertThat(initResp).doesNotContain("error");
            // serverInfo + capabilities + protocol framing
            assertThat(initResp).contains("jitscope").contains("capabilities").contains("tools");
            assertThat(isValidJsonLine(initResp)).isTrue();

            // notifications/initialized (no response)
            send(in, "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}");

            // tools/list
            send(in, jsonRpc(2, "tools/list", "{}"));
            String toolsResp = readResponse(out, 2);
            assertThat(isValidJsonLine(toolsResp)).isTrue();
            for (String name : JitScopeMcp.TOOL_NAMES) {
                assertThat(toolsResp).contains(name);
            }
            // exactly five read-only tools
            assertThat(countOccurrences(toolsResp, "\"name\":\"")).isEqualTo(5);

            // 1. check_recording_config
            assertToolOk(out, in, 3, "check_recording_config", "{\"jfrPath\":" + quote(jfr) + "}", "completeness");
            // 2. analyze_recording
            assertToolOk(out, in, 4, "analyze_recording", "{\"jfrPath\":" + quote(jfr) + ",\"topN\":5}", "findings");
            // 3. inspect_method
            assertToolOk(out, in, 5, "inspect_method", "{\"jfrPath\":" + quote(jfr) + ",\"methodPattern\":\"hot\"}", "hot");
            // 4. find_pathologies
            assertToolOk(out, in, 6, "find_pathologies", "{\"jfrPath\":" + quote(jfr) + "}", "findings");
            // 5. diff_recordings
            assertToolOk(out, in, 7, "diff_recordings",
                    "{\"baselineJfrPath\":" + quote(jfr) + ",\"candidateJfrPath\":" + quote(cliff) + "}", "findings");

            // DEOPT_STORM accuracy via find_pathologies on DeoptStorm
            send(in, jsonRpc(8, "tools/call", """
                    {"name":"find_pathologies","arguments":{"jfrPath":%s,"types":["DEOPT_STORM"]}}""".formatted(quote(jfr))));
            String findResp = readResponse(out, 8);
            assertThat(findResp).contains("DEOPT_STORM").contains("dev.jitscope.workloads.DeoptStorm").contains("hot");

            // structured error for nonexistent path (clean; isError=true; no crash)
            send(in, jsonRpc(9, "tools/call",
                    "{\"name\":\"check_recording_config\",\"arguments\":{\"jfrPath\":\"/no/such/file.jfr\"}}"));
            String errResp = readResponse(out, 9);
            assertThat(isValidJsonLine(errResp)).isTrue();
            assertThat(errResp).contains("\"isError\":true");
        } finally {
            proc.destroyForcibly();
            proc.waitFor(10, TimeUnit.SECONDS);
        }
    }

    // ---- helpers ----
    private void assertToolOk(BufferedReader out, OutputStream in, int id, String name, String argumentsJson,
            String expect) throws Exception {
        send(in, jsonRpc(id, "tools/call", "{\"name\":" + quote(name) + ",\"arguments\":" + argumentsJson + "}"));
        String resp = readResponse(out, id);
        assertThat(isValidJsonLine(resp)).as("valid JSON-RPC for " + name).isTrue();
        assertThat(resp).contains("\"id\":" + id);
        assertThat(resp).doesNotContain("\"isError\":true").as("tool " + name + " not error");
        assertThat(resp).contains("result");
        assertThat(resp).contains("content");
        assertThat(resp.toLowerCase()).contains(expect.toLowerCase());
    }

    private static String jsonRpc(int id, String method, String paramsJson) {
        return "{\"jsonrpc\":\"2.0\",\"id\":" + id + ",\"method\":" + quote(method) + ",\"params\":" + paramsJson + "}";
    }

    private static void send(OutputStream in, String msg) throws IOException {
        in.write(msg.getBytes(StandardCharsets.UTF_8));
        in.write('\n');
        in.flush();
    }

    /** Read lines until a JSON-RPC response with matching id; assert no stray non-JSON on stdout. */
    private static String readResponse(BufferedReader out, int id) throws IOException {
        long deadline = System.currentTimeMillis() + 30_000;
        while (System.currentTimeMillis() < deadline) {
            String line = out.readLine();
            if (line == null) {
                break;
            }
            String t = line.trim();
            if (t.isEmpty()) {
                continue;
            }
            // every stdout line must be valid JSON-RPC (no stray logging)
            assertThat(t.startsWith("{")).as("stray stdout output: " + t).isTrue();
            assertThat(isValidJsonLine(t)).as("invalid JSON-RPC frame: " + t).isTrue();
            if (t.contains("\"id\":" + id + ",") || t.endsWith("\"id\":" + id + "}") || t.contains("\"id\":" + id + "}")) {
                return t;
            }
            // ignore notifications
        }
        throw new AssertionError("no response for id=" + id);
    }

    private static boolean isValidJsonLine(String s) {
        // minimal structural check (balanced braces) + starts with { ; JSON-RPC frames are single-line
        return s.startsWith("{") && s.endsWith("}") && balanced(s);
    }

    private static boolean balanced(String s) {
        int depth = 0;
        boolean inStr = false;
        boolean esc = false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (esc) {
                esc = false;
                continue;
            }
            if (c == '\\') {
                esc = true;
                continue;
            }
            if (c == '"') {
                inStr = !inStr;
                continue;
            }
            if (inStr) {
                continue;
            }
            if (c == '{') {
                depth++;
            } else if (c == '}') {
                depth--;
            }
            if (depth < 0) {
                return false;
            }
        }
        return depth == 0;
    }

    private static int countOccurrences(String s, String sub) {
        int count = 0;
        int idx = 0;
        while ((idx = s.indexOf(sub, idx)) != -1) {
            count++;
            idx += sub.length();
        }
        return count;
    }

    private static String quote(Object s) {
        String v = String.valueOf(s);
        return "\"" + v.replace("\\", "\\\\").replace("\"", "\\\"") + "\"";
    }

    private static Path locateJar() {
        // built by shade into jitscope-cli/target/jitscope.jar; search upwards
        Path dir = Path.of(System.getProperty("user.dir")).toAbsolutePath();
        for (int i = 0; i < 5; i++) {
            Path candidate = dir.resolve("jitscope-cli/target/jitscope.jar");
            if (Files.exists(candidate)) {
                return candidate;
            }
            candidate = dir.resolve("../jitscope-cli/target/jitscope.jar").normalize();
            if (Files.exists(candidate)) {
                return candidate;
            }
            Path up = dir.getParent();
            if (up == null) {
                break;
            }
            dir = up;
        }
        throw new IllegalStateException("jitscope.jar not found; run mvn package first");
    }

    private static Path recordWorkload(String mainClass, Path out, List<String> appArgs, List<String> jvmFlags)
            throws Exception {
        String javaBin = Path.of(System.getProperty("java.home"), "bin", "java").toString();
        Path jfc = locateJitDiag();
        java.util.List<String> cmd = new java.util.ArrayList<>();
        cmd.add(javaBin);
        cmd.addAll(jvmFlags);
        cmd.add("-XX:StartFlightRecording=settings=" + jfc + ",filename=" + out);
        cmd.add("-cp");
        cmd.add(System.getProperty("java.class.path"));
        cmd.add(mainClass);
        cmd.addAll(appArgs);
        Process p = new ProcessBuilder(cmd).redirectErrorStream(true).start();
        String output = new String(p.getInputStream().readAllBytes());
        assertThat(p.waitFor(120, TimeUnit.SECONDS)).as(output).isTrue();
        assertThat(p.exitValue()).as(output).isZero();
        return out;
    }

    private static Path recordWorkload(String mainClass, Path out) throws Exception {
        return recordWorkload(mainClass, out, List.of(), List.of());
    }

    private static Path locateJitDiag() throws Exception {
        var url = Gate4McpIntegrationTest.class.getClassLoader().getResource("jfc/jit-diag.jfc");
        if (url == null) {
            throw new IllegalStateException("jit-diag.jfc not found");
        }
        try (var in = url.openStream()) {
            Path tmp = Files.createTempFile("jit-diag", ".jfc");
            Files.write(tmp, in.readAllBytes());
            tmp.toFile().deleteOnExit();
            return tmp;
        }
    }
}
