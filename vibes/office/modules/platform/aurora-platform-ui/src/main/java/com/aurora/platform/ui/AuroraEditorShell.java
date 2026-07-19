package com.aurora.platform.ui;

import javafx.scene.layout.BorderPane;
import javafx.scene.layout.VBox;

/**
 * Complete editor shell with toolbar, navigator, canvas, inspector, and status bar.
 */
public class AuroraEditorShell extends BorderPane {
    private final AuroraWorkspace workspace;
    private final AuroraStatusBar statusBar;

    public AuroraEditorShell(AuroraWorkspace workspace) {
        this.workspace = workspace;
        getStyleClass().add("aurora-editor-shell");

        AuroraDocumentTab tab = new AuroraDocumentTab(workspace.title(), workspace.kind().productName());
        AuroraToolbar toolbar = new AuroraToolbar(workspace.toolbarActions(), action -> {
            workspace.handleAction(action);
            statusBar().setStatus(action + " selected");
        });
        VBox top = new VBox(tab, toolbar);

        this.statusBar = new AuroraStatusBar(workspace.kind().documentLabel() + " ready");
        setTop(top);
        setLeft(new AuroraSidebar("Navigator", workspace.navigatorItems()));
        setCenter(new AuroraCanvas(workspace.canvas()));
        setRight(new AuroraInspector(workspace.inspectorSections()));
        setBottom(statusBar);
    }

    /**
     * Active workspace model.
     */
    public AuroraWorkspace workspace() {
        return workspace;
    }

    /**
     * Bottom status bar for external updates.
     */
    public AuroraStatusBar statusBar() {
        return statusBar;
    }
}
