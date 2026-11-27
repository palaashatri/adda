package dev.jitscope.core.report;

import java.util.Collection;
import java.util.Map;

/** Minimal JSON writer (no external deps). Escapes correctly; emits valid JSON. */
public final class JsonWriter {

    private final StringBuilder sb = new StringBuilder();

    private JsonWriter() {}

    public static String escape(String s) {
        if (s == null) {
            return "";
        }
        StringBuilder b = new StringBuilder(s.length() + 8);
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '"' -> b.append("\\\"");
                case '\\' -> b.append("\\\\");
                case '\n' -> b.append("\\n");
                case '\r' -> b.append("\\r");
                case '\t' -> b.append("\\t");
                case '\b' -> b.append("\\b");
                case '\f' -> b.append("\\f");
                default -> {
                    if (c < 0x20) {
                        b.append(String.format("\\u%04x", (int) c));
                    } else {
                        b.append(c);
                    }
                }
            }
        }
        return b.toString();
    }

    public static String str(String s) {
        return "\"" + escape(s) + "\"";
    }

    /** Convenience: build an object from alternating key/value pairs (values pre-rendered JSON). */
    public static String obj(Object... kv) {
        StringBuilder b = new StringBuilder("{");
        for (int i = 0; i + 1 < kv.length; i += 2) {
            if (i > 0) {
                b.append(',');
            }
            b.append(str(String.valueOf(kv[i]))).append(':').append(String.valueOf(kv[i + 1]));
        }
        return b.append('}').toString();
    }

    public static String arr(Collection<String> renderedItems) {
        return "[" + String.join(",", renderedItems) + "]";
    }

    public static String num(double v) {
        if (Double.isNaN(v) || Double.isInfinite(v)) {
            return "null";
        }
        return Double.toString(v);
    }

    public static String bool(boolean v) {
        return Boolean.toString(v);
    }

    public static String stringMap(Map<String, String> m) {
        StringBuilder b = new StringBuilder("{");
        boolean first = true;
        for (var e : m.entrySet()) {
            if (!first) {
                b.append(',');
            }
            first = false;
            b.append(str(e.getKey())).append(':').append(str(e.getValue()));
        }
        return b.append('}').toString();
    }

    public StringBuilder buffer() {
        return sb;
    }
}
