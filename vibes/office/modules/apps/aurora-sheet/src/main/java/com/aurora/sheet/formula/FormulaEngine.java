package com.aurora.sheet.formula;

import com.aurora.sheet.model.CellValue;
import com.aurora.sheet.model.Worksheet;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.regex.Pattern;

/**
 * MVP spreadsheet formula engine supporting common scalar functions and ranges.
 */
public final class FormulaEngine {
    private static final Pattern CELL_REF = Pattern.compile("[A-Z]+[0-9]+");
    private static final Pattern RANGE_REF = Pattern.compile("[A-Z]+[0-9]+:[A-Z]+[0-9]+");

    private final Map<String, FormulaFunction> functions = new HashMap<>();

    public FormulaEngine() {
        registerDefaults();
    }

    /**
     * Registers or replaces a formula function.
     */
    public void register(String name, FormulaFunction function) {
        functions.put(normalizeFunction(name), function);
    }

    /**
     * Evaluates a formula or literal expression.
     */
    public CellValue evaluate(String expression, Worksheet worksheet) {
        return evaluateExpression(expression, worksheet, new HashSet<>());
    }

    private void registerDefaults() {
        register("SUM", args -> CellValue.number(args.stream().mapToDouble(CellValue::asNumber).sum()));
        register("AVERAGE", args -> args.isEmpty()
                ? CellValue.number(0)
                : CellValue.number(args.stream().mapToDouble(CellValue::asNumber).average().orElse(0)));
        register("MIN", args -> CellValue.number(args.stream().mapToDouble(CellValue::asNumber).min().orElse(0)));
        register("MAX", args -> CellValue.number(args.stream().mapToDouble(CellValue::asNumber).max().orElse(0)));
        register("COUNT", args -> CellValue.number(args.stream().filter(value -> value.type() == CellValue.Type.NUMBER).count()));
        register("IF", args -> {
            if (args.isEmpty()) {
                return CellValue.blank();
            }
            if (args.get(0).asBoolean()) {
                return args.size() > 1 ? args.get(1) : CellValue.bool(true);
            }
            return args.size() > 2 ? args.get(2) : CellValue.bool(false);
        });
        register("CONCAT", args -> {
            StringBuilder builder = new StringBuilder();
            args.forEach(value -> builder.append(value.asText()));
            return CellValue.text(builder.toString());
        });
        register("ROUND", args -> {
            if (args.isEmpty()) {
                return CellValue.number(0);
            }
            double value = args.get(0).asNumber();
            int scale = args.size() > 1 ? (int) args.get(1).asNumber() : 0;
            double factor = Math.pow(10, scale);
            return CellValue.number(Math.round(value * factor) / factor);
        });
    }

    private CellValue evaluateExpression(String expression, Worksheet worksheet, Set<String> visiting) {
        String trimmed = expression == null ? "" : expression.trim();
        if (trimmed.isEmpty()) {
            return CellValue.blank();
        }
        if (trimmed.startsWith("=")) {
            trimmed = trimmed.substring(1).trim();
        }
        if (trimmed.startsWith("-") && trimmed.length() > 1) {
            CellValue positive = evaluateExpression(trimmed.substring(1), worksheet, visiting);
            return CellValue.number(-positive.asNumber());
        }
        if (trimmed.startsWith("\"") && trimmed.endsWith("\"") && trimmed.length() >= 2) {
            return CellValue.text(trimmed.substring(1, trimmed.length() - 1));
        }
        if ("TRUE".equalsIgnoreCase(trimmed) || "FALSE".equalsIgnoreCase(trimmed)) {
            return CellValue.bool(Boolean.parseBoolean(trimmed));
        }
        if (CELL_REF.matcher(trimmed.toUpperCase(Locale.ROOT)).matches()) {
            return evaluateCell(trimmed, worksheet, visiting);
        }
        if (trimmed.contains("(") && trimmed.endsWith(")")) {
            return evaluateFunction(trimmed, worksheet, visiting);
        }
        try {
            return CellValue.number(Double.parseDouble(trimmed));
        } catch (NumberFormatException ignored) {
            return CellValue.text(trimmed);
        }
    }

    private CellValue evaluateCell(String address, Worksheet worksheet, Set<String> visiting) {
        String normalized = address.toUpperCase(Locale.ROOT);
        if (!visiting.add(normalized)) {
            return CellValue.error("Circular reference: " + normalized);
        }
        try {
            return evaluateExpression(worksheet.rawCell(normalized), worksheet, visiting);
        } finally {
            visiting.remove(normalized);
        }
    }

    private CellValue evaluateFunction(String expression, Worksheet worksheet, Set<String> visiting) {
        int paren = expression.indexOf('(');
        String name = normalizeFunction(expression.substring(0, paren));
        FormulaFunction function = functions.get(name);
        if (function == null) {
            return CellValue.error("Unknown function: " + name);
        }
        String body = expression.substring(paren + 1, expression.length() - 1);
        List<CellValue> args = new ArrayList<>();
        for (String arg : splitArguments(body)) {
            String trimmed = arg.trim();
            if (RANGE_REF.matcher(trimmed.toUpperCase(Locale.ROOT)).matches()) {
                for (String address : expandRange(trimmed)) {
                    args.add(evaluateCell(address, worksheet, visiting));
                }
            } else {
                args.add(evaluateExpression(trimmed, worksheet, visiting));
            }
        }
        return function.evaluate(args);
    }

    private static List<String> splitArguments(String body) {
        if (body.isBlank()) {
            return List.of();
        }
        List<String> args = new ArrayList<>();
        int depth = 0;
        boolean inString = false;
        int start = 0;
        for (int index = 0; index < body.length(); index++) {
            char current = body.charAt(index);
            if (current == '"') {
                inString = !inString;
            } else if (!inString && current == '(') {
                depth++;
            } else if (!inString && current == ')') {
                depth--;
            } else if (!inString && depth == 0 && current == ',') {
                args.add(body.substring(start, index));
                start = index + 1;
            }
        }
        args.add(body.substring(start));
        return args;
    }

    private static List<String> expandRange(String range) {
        String[] parts = range.toUpperCase(Locale.ROOT).split(":");
        CellAddress start = CellAddress.parse(parts[0]);
        CellAddress end = CellAddress.parse(parts[1]);
        List<String> addresses = new ArrayList<>();
        for (int row = Math.min(start.row, end.row); row <= Math.max(start.row, end.row); row++) {
            for (int column = Math.min(start.column, end.column); column <= Math.max(start.column, end.column); column++) {
                addresses.add(CellAddress.format(column, row));
            }
        }
        return addresses;
    }

    private static String normalizeFunction(String name) {
        if (name == null || name.isBlank()) {
            throw new IllegalArgumentException("Function name is required");
        }
        return name.trim().toUpperCase(Locale.ROOT);
    }

    private record CellAddress(int column, int row) {
        static CellAddress parse(String address) {
            int split = 0;
            while (split < address.length() && Character.isLetter(address.charAt(split))) {
                split++;
            }
            int column = 0;
            for (int index = 0; index < split; index++) {
                column = column * 26 + (address.charAt(index) - 'A' + 1);
            }
            int row = Integer.parseInt(address.substring(split));
            return new CellAddress(column, row);
        }

        static String format(int column, int row) {
            StringBuilder letters = new StringBuilder();
            int value = column;
            while (value > 0) {
                value--;
                letters.insert(0, (char) ('A' + value % 26));
                value /= 26;
            }
            return letters + Integer.toString(row);
        }
    }
}
