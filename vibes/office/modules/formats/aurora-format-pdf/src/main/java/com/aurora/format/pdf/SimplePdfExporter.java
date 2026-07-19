package com.aurora.format.pdf;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

/**
 * Small dependency-free PDF text exporter for MVP smoke exports.
 */
public final class SimplePdfExporter {
    /**
     * Exports a one-page PDF containing the supplied title and text.
     */
    public void exportText(Path target, String title, String text) throws IOException {
        Files.createDirectories(target.toAbsolutePath().getParent());
        Files.write(target, createPdf(title, text));
    }

    private byte[] createPdf(String title, String text) throws IOException {
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        List<Integer> offsets = new ArrayList<>();
        write(output, "%PDF-1.4\n");
        offsets.add(output.size());
        write(output, "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n");
        offsets.add(output.size());
        write(output, "2 0 obj\n<< /Type /Pages /Kids [3 0 R] /Count 1 >>\nendobj\n");
        offsets.add(output.size());
        write(output, "3 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] "
                + "/Resources << /Font << /F1 4 0 R >> >> /Contents 5 0 R >>\nendobj\n");
        offsets.add(output.size());
        write(output, "4 0 obj\n<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>\nendobj\n");

        String content = contentStream(title, text);
        offsets.add(output.size());
        write(output, "5 0 obj\n<< /Length " + content.getBytes(StandardCharsets.US_ASCII).length + " >>\nstream\n");
        write(output, content);
        write(output, "endstream\nendobj\n");

        int xref = output.size();
        write(output, "xref\n0 6\n0000000000 65535 f \n");
        for (int offset : offsets) {
            write(output, "%010d 00000 n \n".formatted(offset));
        }
        write(output, "trailer\n<< /Size 6 /Root 1 0 R >>\nstartxref\n" + xref + "\n%%EOF\n");
        return output.toByteArray();
    }

    private String contentStream(String title, String text) {
        StringBuilder builder = new StringBuilder();
        builder.append("BT\n/F1 18 Tf\n72 740 Td\n(").append(escape(title)).append(") Tj\n");
        builder.append("/F1 11 Tf\n0 -28 Td\n");
        int lineCount = 0;
        for (String line : wrap(text == null ? "" : text, 86)) {
            if (lineCount++ >= 34) {
                break;
            }
            builder.append("(").append(escape(line)).append(") Tj\n0 -16 Td\n");
        }
        builder.append("ET\n");
        return builder.toString();
    }

    private List<String> wrap(String text, int width) {
        String normalized = text.replace('\r', '\n');
        List<String> lines = new ArrayList<>();
        for (String rawLine : normalized.split("\n")) {
            String remaining = rawLine;
            while (remaining.length() > width) {
                lines.add(remaining.substring(0, width));
                remaining = remaining.substring(width);
            }
            lines.add(remaining);
        }
        return lines;
    }

    private static void write(ByteArrayOutputStream output, String text) throws IOException {
        output.write(text.getBytes(StandardCharsets.US_ASCII));
    }

    private static String escape(String value) {
        String ascii = value == null ? "" : value.replaceAll("[^\\x20-\\x7E]", "?");
        return ascii.replace("\\", "\\\\").replace("(", "\\(").replace(")", "\\)");
    }
}
