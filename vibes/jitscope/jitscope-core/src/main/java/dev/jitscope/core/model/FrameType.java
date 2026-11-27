package dev.jitscope.core.model;

/**
 * Execution type reported per {@code RecordedFrame} via {@code RecordedFrame.getType()}.
 * HotSpot reports "Interpreted", "JIT compiled", "Inlined", "Native".
 */
public enum FrameType {
    INTERPRETED("Interpreted"),
    JIT_COMPILED("JIT compiled"),
    INLINED("Inlined"),
    NATIVE("Native"),
    UNKNOWN("Unknown");

    private final String label;

    FrameType(String label) {
        this.label = label;
    }

    public String label() {
        return label;
    }

    public static FrameType fromString(String s) {
        if (s == null) {
            return UNKNOWN;
        }
        return switch (s) {
            case "Interpreted" -> INTERPRETED;
            case "JIT compiled" -> JIT_COMPILED;
            case "Inlined" -> INLINED;
            case "Native" -> NATIVE;
            default -> UNKNOWN;
        };
    }
}
