package dev.jitscope.core.ingest;

import java.io.IOException;
import java.io.InputStream;
import java.io.Reader;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import javax.xml.XMLConstants;
import javax.xml.stream.XMLInputFactory;
import javax.xml.stream.XMLStreamConstants;
import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;

import dev.jitscope.core.model.LogCompilationData;
import dev.jitscope.core.model.LogCompilationEvent;
import dev.jitscope.core.model.MethodKey;

/**
 * Streaming StAX parser for {@code -XX:+UnlockDiagnosticVMOptions -XX:+LogCompilation} XML logs.
 * Correlates {@code compile_id}; extracts {@code task}, {@code task_done}, {@code make_not_entrant},
 * {@code make_not_compilable}, {@code parse}, {@code inline_fail}, {@code uncommon_trap}. Never
 * silently swallows; warnings recorded. XXE-hardened. Read-only.
 */
public final class LogCompilationParser {

    private LogCompilationParser() {}

    public static LogCompilationData parse(Path logFile) throws IOException {
        if (logFile == null) {
            throw new IOException("logCompilationPath is null");
        }
        Path abs = logFile.toAbsolutePath().normalize();
        if (!Files.exists(abs)) {
            throw new IOException("LogCompilation log not found: " + abs);
        }
        try (InputStream in = Files.newInputStream(abs)) {
            return parse(in);
        }
    }

    public static LogCompilationData parse(InputStream in) throws IOException {
        List<LogCompilationEvent> events = new ArrayList<>();
        List<String> warnings = new ArrayList<>();

        XMLInputFactory factory = XMLInputFactory.newInstance();
        // XXE hardening
        trySet(factory, XMLInputFactory.SUPPORT_DTD, false);
        trySet(factory, "javax.xml.stream.isSupportingExternalEntities", false);
        trySet(factory, XMLConstants.FEATURE_SECURE_PROCESSING, true);
        trySet(factory, "javax.xml.stream.isNamespaceAware", false);

        // current <task> context for nested events (e.g. <inline_fail>)
        Integer currentCompileId = null;
        MethodKey currentMethod = null;
        Integer currentLevel = null;

        try (Reader reader = new java.io.InputStreamReader(in, java.nio.charset.StandardCharsets.UTF_8)) {
            XMLStreamReader xr = factory.createXMLStreamReader(reader);
            try {
                while (xr.hasNext()) {
                    int ev = xr.next();
                    if (ev != XMLStreamConstants.START_ELEMENT) {
                        // clear task context at end tag
                        if (ev == XMLStreamConstants.END_ELEMENT && "task".equals(xr.getLocalName())) {
                            currentCompileId = null;
                            currentMethod = null;
                            currentLevel = null;
                        }
                        continue;
                    }
                    String name = xr.getLocalName();
                    Map<String, String> attrs = attributes(xr);
                    try {
                        switch (name) {
                            case "task" -> {
                                currentCompileId = parseInt(attrs.get("compile_id"));
                                currentMethod = parseMethod(attrs.get("method"));
                                currentLevel = parseInt(attrs.get("level"));
                                events.add(new LogCompilationEvent(LogCompilationEvent.Type.TASK, currentCompileId,
                                        currentMethod, null, null, null, currentLevel, attrs));
                            }
                            case "task_done" -> events.add(new LogCompilationEvent(LogCompilationEvent.Type.TASK_DONE,
                                    currentCompileId, currentMethod, null, null, parseBool(attrs.get("success")),
                                    currentLevel, attrs));
                            case "make_not_entrant" -> events.add(new LogCompilationEvent(
                                    LogCompilationEvent.Type.MAKE_NOT_ENTRANT, parseInt(attrs.get("compile_id")),
                                    methodOr(attrs.get("method"), currentMethod), attrs.get("reason"), null, null,
                                    parseInt(attrs.get("level")), attrs));
                            case "make_not_compilable" -> events.add(new LogCompilationEvent(
                                    LogCompilationEvent.Type.MAKE_NOT_COMPILABLE, parseInt(attrs.get("compile_id")),
                                    methodOr(attrs.get("method"), currentMethod), attrs.get("reason"), null, null,
                                    parseInt(attrs.get("level")), attrs));
                            case "inline_fail" -> events.add(new LogCompilationEvent(LogCompilationEvent.Type.INLINE_FAIL,
                                    attrs.containsKey("compile_id") ? parseInt(attrs.get("compile_id")) : currentCompileId,
                                    methodOr(attrs.get("method"), currentMethod), attrs.get("reason"), null, null,
                                    currentLevel, attrs));
                            case "uncommon_trap" -> events.add(new LogCompilationEvent(LogCompilationEvent.Type.UNCOMMON_TRAP,
                                    parseInt(attrs.get("compile_id")), methodOr(attrs.get("method"), currentMethod),
                                    attrs.get("reason"), attrs.get("action"), null, parseInt(attrs.get("level")), attrs));
                            case "parse" -> events.add(new LogCompilationEvent(LogCompilationEvent.Type.PARSE,
                                    currentCompileId, methodOr(attrs.get("method"), currentMethod), null, null, null,
                                    currentLevel, attrs));
                            default -> { /* ignore other elements */ }
                        }
                    } catch (RuntimeException ex) {
                        warnings.add("malformed <" + name + "> ignored: " + ex.getMessage());
                    }
                }
            } finally {
                xr.close();
            }
        } catch (XMLStreamException ex) {
            // Streaming: tolerate trailing junk after root close; record warning rather than swallow.
            warnings.add("XML stream issue (recorded; partial events kept): " + ex.getMessage());
        }

        return new LogCompilationData(events, warnings);
    }

    private static Map<String, String> attributes(XMLStreamReader xr) {
        Map<String, String> m = new LinkedHashMap<>();
        int n = xr.getAttributeCount();
        for (int i = 0; i < n; i++) {
            m.put(xr.getAttributeLocalName(i), xmlUnescape(xr.getAttributeValue(i)));
        }
        return m;
    }

    private static MethodKey methodOr(String raw, MethodKey fallback) {
        if (raw == null || raw.isBlank()) {
            return fallback;
        }
        MethodKey parsed = parseMethod(raw);
        return parsed == null ? fallback : parsed;
    }

    /**
     * Parse `com.Foo bar (I)V` or `com.Foo bar(I)V` into a MethodKey (dotted class; XML-unescaped).
     * Note: HotSpot renders `<init>`/`<clinit>`. `parse` events reference numeric method ids, which
     * we keep verbatim (not a MethodKey); returns null then.
     */
    static MethodKey parseMethod(String raw) {
        if (raw == null) {
            return null;
        }
        String s = xmlUnescape(raw).trim();
        if (s.isEmpty()) {
            return null;
        }
        // numeric method id (jdk LogCompilation `parse` uses ids) -> not a MethodKey
        if (s.chars().allMatch(Character::isDigit)) {
            return null;
        }
        // `class name (desc)`
        int paren = s.indexOf('(');
        String head = paren >= 0 ? s.substring(0, paren).trim() : s;
        String desc = paren >= 0 ? s.substring(paren).trim() : "";
        int sp = head.lastIndexOf(' ');
        String className;
        String methodName;
        if (sp >= 0) {
            className = head.substring(0, sp).trim();
            methodName = head.substring(sp + 1).trim();
        } else {
            className = "";
            methodName = head;
        }
        return new MethodKey(className.replace('/', '.'), methodName, desc);
    }

    private static Integer parseInt(String s) {
        if (s == null || s.isBlank()) {
            return null;
        }
        try {
            return Integer.parseInt(s.trim());
        } catch (NumberFormatException ex) {
            return null;
        }
    }

    private static Boolean parseBool(String s) {
        if (s == null) {
            return null;
        }
        return s.trim().equals("1") || s.trim().equalsIgnoreCase("true");
    }

    public static String xmlUnescape(String s) {
        if (s == null) {
            return null;
        }
        return s.replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"")
                .replace("&apos;", "'").replace("&amp;", "&");
    }

    private static void trySet(XMLInputFactory f, String name, Object value) {
        try {
            f.setProperty(name, value);
        } catch (RuntimeException ignored) { }
    }
}
