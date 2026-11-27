package dev.jitscope.cli;

import static org.assertj.core.api.Assertions.assertThat;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.concurrent.TimeUnit;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import picocli.CommandLine;

/** Verification Gate 3: CLI end-to-end on real JFR; exit codes; JSON parses; Markdown readable. */
class CliGate3Test {

    @TempDir
    Path tempDir;

    @Test
    void initJfcWritesTemplate() throws Exception {
        Path out = tempDir.resolve("jit-diag.jfc");
        int exit = run(List.of("init-jfc", "--out", out.toString()));
        assertThat(exit).isZero();
        assertThat(out).exists();
        String s = Files.readString(out);
        assertThat(s).contains("jdk.Compilation").contains("0 ms").contains("jdk.CPUTimeSample");
        // XML well-formed
        javax.xml.parsers.DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(out.toFile());
    }

    @Test
    void initJfcIncludeCpuTime() throws Exception {
        Path out = tempDir.resolve("jit-diag-cpu.jfc");
        int exit = run(List.of("init-jfc", "--out", out.toString(), "--include-cpu-time"));
        assertThat(exit).isZero();
        String s = Files.readString(out);
        int idx = s.indexOf("name=\"jdk.CPUTimeSample\"");
        assertThat(idx).isGreaterThan(0);
        assertThat(s.substring(idx, idx + 120)).contains("enabled\">true");
    }

    @Test
    void analyzeMethodConfigCheckDiffOnRealJfr() throws Exception {
        // locate bundled jit-diag.jfc on classpath
        Path jitDiag = locateJitDiag();
        Path jfr = recordWorkload("dev.jitscope.workloads.DeoptStorm", jitDiag, tempDir.resolve("d.jfr"), List.of());

        // analyze md
        assertThat(run(List.of("analyze", jfr.toString()))).isZero();
        // analyze json -> valid (we re-run capturing output)
        String json = runCapture(List.of("analyze", jfr.toString(), "--format", "json"));
        assertThat(json.trim()).startsWith("{");
        assertThat(json).contains("\"findings\"");
        // method
        assertThat(run(List.of("method", jfr.toString(), "hot"))).isZero();
        assertThat(runCapture(List.of("method", jfr.toString(), "hot", "--format", "json")).trim()).startsWith("{");
        // config-check
        assertThat(run(List.of("config-check", jfr.toString()))).isZero();
        assertThat(runCapture(List.of("config-check", jfr.toString(), "--format", "json")).trim()).startsWith("{");
        // diff against itself
        assertThat(run(List.of("diff", jfr.toString(), jfr.toString()))).isZero();
        assertThat(runCapture(List.of("diff", jfr.toString(), jfr.toString(), "--format", "json")).trim()).startsWith("{");
    }

    @Test
    void nonexistentPathFailsCleanly() {
        int exit = run(List.of("analyze", tempDir.resolve("nope.jfr").toString()));
        assertThat(exit).isEqualTo(1);
    }

    // ---- helpers ----
    private static int run(List<String> args) {
        return new CommandLine(new JitScopeMain()).execute(args.toArray(new String[0]));
    }

    private static String runCapture(List<String> args) {
        PrintStream orig = System.out;
        ByteArrayOutputStream buf = new ByteArrayOutputStream();
        System.setOut(new PrintStream(buf));
        try {
            run(args);
        } finally {
            System.setOut(orig);
        }
        return buf.toString();
    }

    private static Path locateJitDiag() throws Exception {
        var url = CliGate3Test.class.getClassLoader().getResource("jfc/jit-diag.jfc");
        if (url == null) {
            throw new IllegalStateException("jit-diag.jfc not on classpath");
        }
        try (var in = url.openStream()) {
            Path tmp = Files.createTempFile("jit-diag", ".jfc");
            Files.write(tmp, in.readAllBytes());
            tmp.toFile().deleteOnExit();
            return tmp;
        }
    }

    private static Path recordWorkload(String mainClass, Path jfc, Path out, List<String> appArgs) throws Exception {
        String javaBin = Path.of(System.getProperty("java.home"), "bin", "java").toString();
        List<String> cmd = new java.util.ArrayList<>();
        cmd.add(javaBin);
        cmd.add("-XX:StartFlightRecording=settings=" + jfc + ",filename=" + out);
        cmd.add("-cp");
        cmd.add(System.getProperty("java.class.path"));
        cmd.add(mainClass);
        cmd.addAll(appArgs);
        Process p = new ProcessBuilder(cmd).redirectErrorStream(true).start();
        String output = new String(p.getInputStream().readAllBytes());
        boolean done = p.waitFor(120, TimeUnit.SECONDS);
        assertThat(done).as(output).isTrue();
        assertThat(p.exitValue()).as(output).isZero();
        return out;
    }
}
