package dev.jitscope.mcp;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.CountDownLatch;

import io.modelcontextprotocol.json.McpJsonMapper;
import io.modelcontextprotocol.json.jackson3.JacksonMcpJsonMapperSupplier;
import io.modelcontextprotocol.server.McpServer;
import io.modelcontextprotocol.server.McpServerFeatures;
import io.modelcontextprotocol.server.McpSyncServer;
import io.modelcontextprotocol.server.transport.StdioServerTransportProvider;
import io.modelcontextprotocol.spec.McpSchema;

import dev.jitscope.core.analysis.DiffEngine;
import dev.jitscope.core.analysis.RecordingAnalyzer;
import dev.jitscope.core.model.DiffFinding;
import dev.jitscope.core.model.PathologyFinding;
import dev.jitscope.core.model.PathologyType;
import dev.jitscope.core.model.RecordingAnalysis;
import dev.jitscope.core.model.Severity;
import dev.jitscope.core.report.JsonRenderer;
import dev.jitscope.core.report.MarkdownRenderer;

/**
 * Thin STDIO MCP server. Exactly five read-only tools. All diagnostic logic in jitscope-core.
 *
 * <p>STDIO discipline: {@code System.out} is strictly reserved for JSON-RPC frames; all logging and
 * diagnostics go to {@code System.err}. Rejects nonexistent paths cleanly with structured tool
 * errors; never accepts arbitrary shell commands or JVM flags.
 */
public final class JitScopeMcp {

    public static final String SERVER_NAME = "jitscope";
    public static final String SERVER_VERSION = "0.1.0-SNAPSHOT";

    public static final List<String> TOOL_NAMES = List.of(
            "check_recording_config",
            "analyze_recording",
            "inspect_method",
            "find_pathologies",
            "diff_recordings");

    private JitScopeMcp() {}

    public static void main(String[] args) {
        try {
            run();
        } catch (Throwable t) {
            System.err.println("[jitscope-mcp] fatal: " + t);
            System.exit(1);
        }
    }

    public static void run() throws InterruptedException {
        McpJsonMapper mapper = new JacksonMcpJsonMapperSupplier().get();
        StdioServerTransportProvider transport =
                new StdioServerTransportProvider(mapper, System.in, System.out);

        McpSyncServer server = McpServer.sync(transport)
                .serverInfo(SERVER_NAME, SERVER_VERSION)
                .capabilities(McpSchema.ServerCapabilities.builder().tools(true).build())
                .tools(tools(mapper))
                .build();

        System.err.println("[jitscope-mcp] " + SERVER_NAME + " " + SERVER_VERSION + " ready; tools=" + TOOL_NAMES);
        // block main thread; transport reads stdin asynchronously
        new CountDownLatch(1).await();
    }

    private static List<McpServerFeatures.SyncToolSpecification> tools(McpJsonMapper mapper) {
        List<McpServerFeatures.SyncToolSpecification> list = new ArrayList<>();

        list.add(tool("check_recording_config",
                "Audit a .jfr recording's JFR settings; report missing events/thresholds.",
                schema(props("jfrPath", str("Path to .jfr recording")), List.of("jfrPath")),
                (ex, req) -> checkRecordingConfig(mapper, args(req))));

        list.add(tool("analyze_recording",
                "Full recording analysis: audit + top hot methods + prioritized pathology findings.",
                schema(props("jfrPath", str("Path to .jfr recording"),
                                "logCompilationPath", str("Optional -XX:+LogCompilation XML log"),
                                "topN", integer("Top N hot methods (default 20)")),
                        List.of("jfrPath")),
                (ex, req) -> analyzeRecording(mapper, args(req))));

        list.add(tool("inspect_method",
                "Full per-method compilation lifecycle for matching methods.",
                schema(props("jfrPath", str("Path to .jfr recording"),
                                "methodPattern", str("Substring/case-insensitive match on method fqn"),
                                "logCompilationPath", str("Optional -XX:+LogCompilation XML log")),
                        List.of("jfrPath", "methodPattern")),
                (ex, req) -> inspectMethod(mapper, args(req))));

        list.add(tool("find_pathologies",
                "Prioritized pathology findings (HOT_UNDEROPTIMIZED/DEOPT_STORM/INLINING_ALLOCATION_CLIFF/CODE_CACHE_PRESSURE).",
                schema(props("jfrPath", str("Path to .jfr recording"),
                                "types", array("Filter pathology types"),
                                "minSeverity", str("Min severity INFO|LOW|MEDIUM|HIGH|CRITICAL"),
                                "logCompilationPath", str("Optional -XX:+LogCompilation XML log")),
                        List.of("jfrPath")),
                (ex, req) -> findPathologies(mapper, args(req))));

        list.add(tool("diff_recordings",
                "Structured regression diff baseline vs candidate (tier/inlining/deopt/hotness + version/sampling).",
                schema(props("baselineJfrPath", str("Baseline .jfr"),
                                "candidateJfrPath", str("Candidate .jfr"),
                                "minSampleShareDelta", number("Min normalized sample-share delta (default 0.01)")),
                        List.of("baselineJfrPath", "candidateJfrPath")),
                (ex, req) -> diffRecordings(mapper, args(req))));

        return list;
    }

    private static McpServerFeatures.SyncToolSpecification tool(String name, String description, Map<String, Object> inputSchema,
            java.util.function.BiFunction<io.modelcontextprotocol.server.McpSyncServerExchange, McpSchema.CallToolRequest, McpSchema.CallToolResult> handler) {
        McpSchema.Tool t = McpSchema.Tool.builder(name).description(description).inputSchema(inputSchema).build();
        return new McpServerFeatures.SyncToolSpecification(t, (ex, req) -> {
            try {
                return handler.apply(ex, req);
            } catch (Throwable err) {
                return error(mapperOf(ex), "tool " + name + " failed: " + safeMsg(err));
            }
        });
    }

    private static McpJsonMapper mapperOf(io.modelcontextprotocol.server.McpSyncServerExchange ex) {
        return new JacksonMcpJsonMapperSupplier().get();
    }

    // ---------------- tools ----------------
    private static McpSchema.CallToolResult checkRecordingConfig(McpJsonMapper mapper, Map<String, Object> args) {
        Optional<Path> jfr = requireFile(args, "jfrPath");
        if (jfr.isEmpty()) {
            return error(mapper, "jfrPath is required and must be an existing .jfr file");
        }
        try {
            RecordingAnalysis a = RecordingAnalyzer.analyze(jfr.get());
            var audit = a.metadata().configAudit();
            String json = JsonRenderer.configCheck(audit);
            return ok(mapper, MarkdownRenderer.configCheck(audit), json);
        } catch (Exception e) {
            return error(mapper, "failed to read recording: " + safeMsg(e));
        }
    }

    private static McpSchema.CallToolResult analyzeRecording(McpJsonMapper mapper, Map<String, Object> args) {
        Optional<Path> jfr = requireFile(args, "jfrPath");
        if (jfr.isEmpty()) {
            return error(mapper, "jfrPath is required and must be an existing .jfr file");
        }
        Path logCompilation = optionalFile(args, "logCompilationPath");
        int topN = intArg(args, "topN", 20);
        try {
            RecordingAnalysis a = RecordingAnalyzer.analyze(jfr.get(), logCompilation);
            String json = JsonRenderer.analysis(a);
            return ok(mapper, MarkdownRenderer.analysis(a, topN), json);
        } catch (Exception e) {
            return error(mapper, "failed to analyze recording: " + safeMsg(e));
        }
    }

    private static McpSchema.CallToolResult inspectMethod(McpJsonMapper mapper, Map<String, Object> args) {
        Optional<Path> jfr = requireFile(args, "jfrPath");
        if (jfr.isEmpty()) {
            return error(mapper, "jfrPath is required and must be an existing .jfr file");
        }
        String pattern = stringArg(args, "methodPattern", null);
        if (pattern == null || pattern.isBlank()) {
            return error(mapper, "methodPattern is required");
        }
        Path logCompilation = optionalFile(args, "logCompilationPath");
        try {
            RecordingAnalysis a = RecordingAnalyzer.analyze(jfr.get(), logCompilation);
            String needle = pattern.toLowerCase(Locale.ROOT);
            var matches = a.lifecycles().stream()
                    .filter(l -> l.method().fqn().toLowerCase(Locale.ROOT).contains(needle)
                            || l.method().className().toLowerCase(Locale.ROOT).contains(needle)
                            || l.method().methodName().toLowerCase(Locale.ROOT).contains(needle))
                    .toList();
            String json = JsonRenderer.methodList(matches);
            return ok(mapper, MarkdownRenderer.method(matches, pattern), json);
        } catch (Exception e) {
            return error(mapper, "failed to inspect method: " + safeMsg(e));
        }
    }

    private static McpSchema.CallToolResult findPathologies(McpJsonMapper mapper, Map<String, Object> args) {
        Optional<Path> jfr = requireFile(args, "jfrPath");
        if (jfr.isEmpty()) {
            return error(mapper, "jfrPath is required and must be an existing .jfr file");
        }
        Path logCompilation = optionalFile(args, "logCompilationPath");
        Set<String> typeFilter = stringSet(args.get("types"));
        int minSeverity = severityRank(stringArg(args, "minSeverity", "INFO"));
        try {
            RecordingAnalysis a = RecordingAnalyzer.analyze(jfr.get(), logCompilation);
            List<PathologyFinding> findings = a.findings().stream()
                    .filter(f -> typeFilter.isEmpty() || typeFilter.contains(f.type().name()))
                    .filter(f -> severityRank(f.severity().name()) >= minSeverity)
                    .toList();
            String json = JsonRenderer.findings(findings);
            String md = "# Findings\n\n" + findings.stream().map(MarkdownRenderer::finding).reduce("", (x, y) -> x + y);
            return ok(mapper, md, json);
        } catch (Exception e) {
            return error(mapper, "failed to find pathologies: " + safeMsg(e));
        }
    }

    private static McpSchema.CallToolResult diffRecordings(McpJsonMapper mapper, Map<String, Object> args) {
        Optional<Path> base = requireFile(args, "baselineJfrPath");
        Optional<Path> cand = requireFile(args, "candidateJfrPath");
        if (base.isEmpty()) {
            return error(mapper, "baselineJfrPath is required and must be an existing .jfr file");
        }
        if (cand.isEmpty()) {
            return error(mapper, "candidateJfrPath is required and must be an existing .jfr file");
        }
        double minDelta = doubleArg(args, "minSampleShareDelta", DiffEngine.DEFAULT_MIN_SAMPLE_SHARE_DELTA);
        try {
            RecordingAnalysis b = RecordingAnalyzer.analyze(base.get());
            RecordingAnalysis c = RecordingAnalyzer.analyze(cand.get());
            List<DiffFinding> diffs = DiffEngine.diff(b, c, minDelta);
            String json = JsonRenderer.diff(diffs, "normalized shares; raw cross-recording counts not compared");
            return ok(mapper, MarkdownRenderer.diff(diffs, "baseline=" + base.get() + " candidate=" + cand.get()), json);
        } catch (Exception e) {
            return error(mapper, "failed to diff recordings: " + safeMsg(e));
        }
    }

    // ---------------- result helpers ----------------
    private static McpSchema.CallToolResult ok(McpJsonMapper mapper, String markdown, String json) {
        Object structured = tryParse(mapper, json);
        return McpSchema.CallToolResult.builder()
                .addTextContent(markdown)
                .structuredContent(structured == null ? json : structured)
                .isError(false)
                .build();
    }

    private static McpSchema.CallToolResult error(McpJsonMapper mapper, String message) {
        Map<String, Object> err = new LinkedHashMap<>();
        err.put("error", message);
        return McpSchema.CallToolResult.builder()
                .addTextContent(message)
                .structuredContent(err)
                .isError(true)
                .build();
    }

    private static Object tryParse(McpJsonMapper mapper, String json) {
        try {
            return mapper.readValue(json, Object.class);
        } catch (Exception e) {
            return null;
        }
    }

    // ---------------- arg parsing ----------------
    private static Map<String, Object> args(McpSchema.CallToolRequest req) {
        Map<String, Object> a = req.arguments();
        return a == null ? Map.of() : a;
    }

    private static Optional<Path> requireFile(Map<String, Object> args, String key) {
        String v = stringArg(args, key, null);
        if (v == null || v.isBlank()) {
            return Optional.empty();
        }
        try {
            Path p = Path.of(v).toAbsolutePath().normalize();
            if (!Files.exists(p) || !Files.isRegularFile(p)) {
                return Optional.empty();
            }
            return Optional.of(p);
        } catch (RuntimeException e) {
            return Optional.empty();
        }
    }

    private static Path optionalFile(Map<String, Object> args, String key) {
        String v = stringArg(args, key, null);
        if (v == null || v.isBlank()) {
            return null;
        }
        try {
            Path p = Path.of(v).toAbsolutePath().normalize();
            return Files.exists(p) && Files.isRegularFile(p) ? p : null;
        } catch (RuntimeException e) {
            return null;
        }
    }

    private static String stringArg(Map<String, Object> args, String key, String dflt) {
        Object v = args.get(key);
        return v == null ? dflt : v.toString();
    }

    private static int intArg(Map<String, Object> args, String key, int dflt) {
        Object v = args.get(key);
        if (v instanceof Number n) {
            return n.intValue();
        }
        if (v != null) {
            try {
                return Integer.parseInt(v.toString());
            } catch (NumberFormatException ignored) { }
        }
        return dflt;
    }

    private static double doubleArg(Map<String, Object> args, String key, double dflt) {
        Object v = args.get(key);
        if (v instanceof Number n) {
            return n.doubleValue();
        }
        if (v != null) {
            try {
                return Double.parseDouble(v.toString());
            } catch (NumberFormatException ignored) { }
        }
        return dflt;
    }

    private static Set<String> stringSet(Object v) {
        if (v == null) {
            return Set.of();
        }
        if (v instanceof List<?> list) {
            return list.stream().map(Object::toString).map(String::toUpperCase).collect(java.util.stream.Collectors.toSet());
        }
        return Set.of(v.toString().toUpperCase(Locale.ROOT));
    }

    private static int severityRank(String s) {
        return switch (s == null ? "INFO" : s.toUpperCase(Locale.ROOT)) {
            case "CRITICAL" -> 4;
            case "HIGH" -> 3;
            case "MEDIUM" -> 2;
            case "LOW" -> 1;
            default -> 0;
        };
    }

    private static String safeMsg(Throwable t) {
        String m = t.getMessage();
        return m == null ? t.getClass().getSimpleName() : m;
    }

    // ---------------- JSON schema builders ----------------
    private static Map<String, Object> schema(Map<String, Object> props, List<String> required) {
        Map<String, Object> s = new LinkedHashMap<>();
        s.put("type", "object");
        s.put("properties", props);
        s.put("required", required);
        return s;
    }

    private static Map<String, Object> props(Object... kv) {
        Map<String, Object> m = new LinkedHashMap<>();
        for (int i = 0; i + 1 < kv.length; i += 2) {
            m.put((String) kv[i], kv[i + 1]);
        }
        return m;
    }

    private static Map<String, Object> str(String description) {
        return Map.of("type", "string", "description", description);
    }

    private static Map<String, Object> integer(String description) {
        return Map.of("type", "integer", "description", description);
    }

    private static Map<String, Object> number(String description) {
        return Map.of("type", "number", "description", description);
    }

    private static Map<String, Object> array(String description) {
        return Map.of("type", "array", "items", Map.of("type", "string"), "description", description);
    }
}
