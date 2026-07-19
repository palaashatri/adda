package com.aurora.format.docx;

import com.aurora.platform.core.AuroraDocumentJson;
import com.aurora.platform.core.AuroraJson;
import org.apache.poi.xwpf.usermodel.XWPFDocument;
import org.apache.poi.xwpf.usermodel.XWPFParagraph;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;

/**
 * Basic DOCX import/export for Aurora Write MVP payloads.
 */
public final class DocxFormatService {
    /**
     * Exports Write JSON to a DOCX document with plain paragraphs.
     */
    public void exportWrite(Path target, String writeDocumentJson) throws IOException {
        Files.createDirectories(target.toAbsolutePath().getParent());
        String body = AuroraJson.readStringField(writeDocumentJson, "body").orElse("");
        try (XWPFDocument document = new XWPFDocument();
             OutputStream output = Files.newOutputStream(target)) {
            String[] paragraphs = body.split("\\R", -1);
            for (String paragraphText : paragraphs) {
                XWPFParagraph paragraph = document.createParagraph();
                paragraph.createRun().setText(paragraphText);
            }
            document.write(output);
        }
    }

    /**
     * Imports a DOCX document into Write JSON.
     */
    public String importWrite(Path source) throws IOException {
        try (InputStream input = Files.newInputStream(source);
             XWPFDocument document = new XWPFDocument(input)) {
            StringBuilder body = new StringBuilder();
            boolean first = true;
            for (XWPFParagraph paragraph : document.getParagraphs()) {
                if (!first) {
                    body.append('\n');
                }
                first = false;
                body.append(paragraph.getText());
            }
            return AuroraDocumentJson.write(body.toString());
        }
    }
}
