package com.aurora.sheet.model;

/**
 * Typed spreadsheet cell value.
 */
public final class CellValue {
    public enum Type {
        NUMBER,
        TEXT,
        BOOLEAN,
        BLANK,
        ERROR
    }

    private static final CellValue BLANK = new CellValue(Type.BLANK, null);

    private final Type type;
    private final Object value;

    private CellValue(Type type, Object value) {
        this.type = type;
        this.value = value;
    }

    /**
     * Creates a numeric value.
     */
    public static CellValue number(double value) {
        return new CellValue(Type.NUMBER, value);
    }

    /**
     * Creates a text value.
     */
    public static CellValue text(String value) {
        return new CellValue(Type.TEXT, value == null ? "" : value);
    }

    /**
     * Creates a boolean value.
     */
    public static CellValue bool(boolean value) {
        return new CellValue(Type.BOOLEAN, value);
    }

    /**
     * Returns a blank value.
     */
    public static CellValue blank() {
        return BLANK;
    }

    /**
     * Creates an error value.
     */
    public static CellValue error(String message) {
        return new CellValue(Type.ERROR, message == null ? "Error" : message);
    }

    public Type type() {
        return type;
    }

    /**
     * Returns this value as a number for formula evaluation.
     */
    public double asNumber() {
        return switch (type) {
            case NUMBER -> (double) value;
            case BOOLEAN -> (boolean) value ? 1.0 : 0.0;
            case TEXT -> {
                try {
                    yield Double.parseDouble((String) value);
                } catch (NumberFormatException exception) {
                    yield 0.0;
                }
            }
            case BLANK, ERROR -> 0.0;
        };
    }

    /**
     * Returns this value as a boolean for formula evaluation.
     */
    public boolean asBoolean() {
        return switch (type) {
            case BOOLEAN -> (boolean) value;
            case NUMBER -> Math.abs((double) value) > 0.0000001;
            case TEXT -> !((String) value).isBlank();
            case BLANK, ERROR -> false;
        };
    }

    /**
     * Returns this value as display text.
     */
    public String asText() {
        return switch (type) {
            case NUMBER -> {
                double number = (double) value;
                if (Math.rint(number) == number) {
                    yield Long.toString((long) number);
                }
                yield Double.toString(number);
            }
            case TEXT, ERROR -> (String) value;
            case BOOLEAN -> Boolean.toString((boolean) value);
            case BLANK -> "";
        };
    }
}
