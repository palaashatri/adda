package com.aurora.format.pptx;

import com.aurora.platform.core.AuroraDocumentJson;
import com.aurora.platform.core.AuroraJson;
import org.apache.poi.sl.usermodel.TextParagraph;
import org.apache.poi.xslf.usermodel.XMLSlideShow;
import org.apache.poi.xslf.usermodel.XSLFSlide;
import org.apache.poi.xslf.usermodel.XSLFTextBox;
import org.apache.poi.xslf.usermodel.XSLFTextShape;

import java.awt.Rectangle;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

/**
 * Basic PPTX import/export for Aurora Deck MVP payloads.
 */
public final class PptxFormatService {
    /**
     * Exports Deck JSON to a single-slide PPTX.
     */
    public void exportDeck(Path target, String deckDocumentJson) throws IOException {
        Files.createDirectories(target.toAbsolutePath().getParent());
        String title = AuroraJson.readStringField(deckDocumentJson, "title").orElse("Untitled Slide");
        String body = AuroraJson.readStringField(deckDocumentJson, "body").orElse("");

        try (XMLSlideShow show = new XMLSlideShow();
             OutputStream output = Files.newOutputStream(target)) {
            show.setPageSize(new java.awt.Dimension(960, 540));
            XSLFSlide slide = show.createSlide();
            XSLFTextBox titleBox = slide.createTextBox();
            titleBox.setAnchor(new Rectangle(80, 70, 800, 80));
            titleBox.setText(title);
            titleBox.getTextParagraphs().forEach(paragraph -> paragraph.setTextAlign(TextParagraph.TextAlign.CENTER));

            XSLFTextBox bodyBox = slide.createTextBox();
            bodyBox.setAnchor(new Rectangle(120, 180, 720, 260));
            bodyBox.setText(body);
            show.write(output);
        }
    }

    /**
     * Imports the first slide's text into Deck JSON.
     */
    public String importDeck(Path source) throws IOException {
        try (InputStream input = Files.newInputStream(source);
             XMLSlideShow show = new XMLSlideShow(input)) {
            if (show.getSlides().isEmpty()) {
                return AuroraDocumentJson.deck("Untitled Slide", "");
            }
            XSLFSlide slide = show.getSlides().get(0);
            List<String> textRuns = new ArrayList<>();
            slide.getShapes().forEach(shape -> {
                if (shape instanceof XSLFTextShape textShape) {
                    String text = textShape.getText();
                    if (text != null && !text.isBlank()) {
                        textRuns.add(text);
                    }
                }
            });
            String title = textRuns.isEmpty() ? "Untitled Slide" : textRuns.get(0);
            String body = textRuns.size() < 2 ? "" : String.join("\n", textRuns.subList(1, textRuns.size()));
            return AuroraDocumentJson.deck(title, body);
        }
    }
}
