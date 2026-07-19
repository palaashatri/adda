package com.aurora.platform.ui;

import com.aurora.platform.core.AuroraDocumentKind;
import javafx.scene.Node;

import java.util.List;
import java.util.Objects;
import java.util.function.Consumer;
import java.util.function.Supplier;

/**
 * UI description for a single Aurora editor workspace.
 */
public final class AuroraWorkspace {
    private final AuroraDocumentKind kind;
    private final String title;
    private final List<String> toolbarActions;
    private final List<String> navigatorItems;
    private final List<String> inspectorSections;
    private final Node canvas;
    private final Supplier<String> documentWriter;
    private final Consumer<String> documentReader;
    private final Consumer<String> actionHandler;

    public AuroraWorkspace(
            AuroraDocumentKind kind,
            String title,
            List<String> toolbarActions,
            List<String> navigatorItems,
            List<String> inspectorSections,
            Node canvas,
            Supplier<String> documentWriter,
            Consumer<String> documentReader
    ) {
        this(kind, title, toolbarActions, navigatorItems, inspectorSections, canvas, documentWriter, documentReader, action -> {
        });
    }

    public AuroraWorkspace(
            AuroraDocumentKind kind,
            String title,
            List<String> toolbarActions,
            List<String> navigatorItems,
            List<String> inspectorSections,
            Node canvas,
            Supplier<String> documentWriter,
            Consumer<String> documentReader,
            Consumer<String> actionHandler
    ) {
        this.kind = Objects.requireNonNull(kind, "kind");
        this.title = title == null || title.isBlank() ? "Untitled " + kind.documentLabel() : title;
        this.toolbarActions = List.copyOf(toolbarActions == null ? List.of() : toolbarActions);
        this.navigatorItems = List.copyOf(navigatorItems == null ? List.of() : navigatorItems);
        this.inspectorSections = List.copyOf(inspectorSections == null ? List.of() : inspectorSections);
        this.canvas = Objects.requireNonNull(canvas, "canvas");
        this.documentWriter = Objects.requireNonNull(documentWriter, "documentWriter");
        this.documentReader = Objects.requireNonNull(documentReader, "documentReader");
        this.actionHandler = Objects.requireNonNull(actionHandler, "actionHandler");
    }

    /**
     * Native document family represented by this workspace.
     */
    public AuroraDocumentKind kind() {
        return kind;
    }

    /**
     * Document title shown in the shell.
     */
    public String title() {
        return title;
    }

    /**
     * Toolbar action labels for this editor mode.
     */
    public List<String> toolbarActions() {
        return toolbarActions;
    }

    /**
     * Navigator items shown in the left sidebar.
     */
    public List<String> navigatorItems() {
        return navigatorItems;
    }

    /**
     * Inspector sections shown in the right panel.
     */
    public List<String> inspectorSections() {
        return inspectorSections;
    }

    /**
     * Main editor canvas node.
     */
    public Node canvas() {
        return canvas;
    }

    /**
     * Serializes the editable document payload to JSON.
     */
    public String documentJson() {
        return documentWriter.get();
    }

    /**
     * Loads a serialized document payload into the workspace.
     */
    public void loadDocumentJson(String documentJson) {
        documentReader.accept(documentJson == null ? "{}" : documentJson);
    }

    /**
     * Handles a workspace-specific toolbar action.
     */
    public void handleAction(String action) {
        actionHandler.accept(action);
    }
}
