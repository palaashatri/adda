package com.aurora.launcher;

import com.aurora.deck.DeckWorkspaceFactory;
import com.aurora.format.docx.DocxFormatService;
import com.aurora.format.nativepkg.NativeFormatService;
import com.aurora.format.pdf.SimplePdfExporter;
import com.aurora.format.pptx.PptxFormatService;
import com.aurora.format.xlsx.XlsxFormatService;
import com.aurora.platform.ai.AiModelCatalog;
import com.aurora.platform.ai.AiProvider;
import com.aurora.platform.ai.DisabledAiProvider;
import com.aurora.platform.ai.HttpAiProvider;
import com.aurora.platform.core.AuroraDocumentKind;
import com.aurora.platform.core.AuroraVersion;
import com.aurora.platform.plugin.PluginAction;
import com.aurora.platform.plugin.PluginRegistry;
import com.aurora.platform.storage.AuroraSettings;
import com.aurora.platform.storage.AuroraSettingsStore;
import com.aurora.platform.storage.AuroraUserPaths;
import com.aurora.platform.storage.AutosaveStore;
import com.aurora.platform.storage.RecentDocumentsStore;
import com.aurora.platform.ui.AuroraCommandPalette;
import com.aurora.platform.ui.AuroraEditorShell;
import com.aurora.platform.ui.AuroraTheme;
import com.aurora.platform.ui.AuroraWindow;
import com.aurora.platform.ui.AuroraWorkspace;
import com.aurora.sheet.SheetWorkspaceFactory;
import com.aurora.write.WriteWorkspaceFactory;
import javafx.animation.KeyFrame;
import javafx.animation.Timeline;
import javafx.application.Application;
import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.Button;
import javafx.scene.control.ButtonType;
import javafx.scene.control.CheckBox;
import javafx.scene.control.ChoiceBox;
import javafx.scene.control.Label;
import javafx.scene.control.Menu;
import javafx.scene.control.MenuBar;
import javafx.scene.control.MenuItem;
import javafx.scene.control.SeparatorMenuItem;
import javafx.scene.control.TextField;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyCodeCombination;
import javafx.scene.input.KeyCombination;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.VBox;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import javafx.util.Duration;

import java.io.File;
import java.nio.file.Path;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.function.Consumer;

/**
 * JavaFX entry point for Aurora Office.
 */
public final class AuroraLauncherApp extends Application {
    private final ExecutorService ioExecutor = Executors.newSingleThreadExecutor(task -> {
        Thread thread = new Thread(task, "aurora-io");
        thread.setDaemon(true);
        return thread;
    });
    private final AuroraUserPaths userPaths = AuroraUserPaths.defaults();
    private final AuroraSettingsStore settingsStore = new AuroraSettingsStore(userPaths.settingsFile());
    private final RecentDocumentsStore recentDocumentsStore = new RecentDocumentsStore(userPaths.recentDocumentsFile());
    private final AutosaveStore autosaveStore = new AutosaveStore(userPaths.autosaveDirectory());
    private final NativeFormatService nativeFormatService = new NativeFormatService();
    private final SimplePdfExporter pdfExporter = new SimplePdfExporter();
    private final DocxFormatService docxFormatService = new DocxFormatService();
    private final XlsxFormatService xlsxFormatService = new XlsxFormatService();
    private final PptxFormatService pptxFormatService = new PptxFormatService();

    private Stage stage;
    private AuroraWorkspace currentWorkspace;
    private AuroraEditorShell currentShell;
    private Path currentFile;
    private AuroraSettings settings = AuroraSettings.defaults();
    private PluginRegistry pluginRegistry = new PluginRegistry();
    private Timeline autosaveTimeline;

    public static void main(String[] args) {
        launch(args);
    }

    @Override
    public void start(Stage primaryStage) {
        this.stage = primaryStage;
        this.pluginRegistry = PluginRegistry.loadInstalledPlugins();
        stage.setTitle(AuroraVersion.PRODUCT_NAME);
        showLauncher();
        runIo(settingsStore::load, loaded -> {
            settings = loaded;
            AuroraTheme.apply(stage.getScene(), settings.theme());
        });
        startAutosave();
        stage.show();
    }

    @Override
    public void stop() {
        if (autosaveTimeline != null) {
            autosaveTimeline.stop();
        }
        pluginRegistry.deactivateAll();
        ioExecutor.shutdownNow();
    }

    private void showLauncher() {
        currentWorkspace = null;
        currentShell = null;
        currentFile = null;

        Label title = new Label(AuroraVersion.PRODUCT_NAME);
        title.getStyleClass().add("aurora-launcher-title");
        Label subtitle = new Label("Java-native documents, spreadsheets, presentations, and local-first AI.");
        subtitle.getStyleClass().add("aurora-launcher-subtitle");

        Button newDocument = launcherButton("New Document", true);
        newDocument.setOnAction(event -> showWorkspace(WriteWorkspaceFactory.create(createAiProvider()), null));
        Button newSpreadsheet = launcherButton("New Spreadsheet", false);
        newSpreadsheet.setOnAction(event -> showWorkspace(SheetWorkspaceFactory.create(createAiProvider()), null));
        Button newPresentation = launcherButton("New Presentation", false);
        newPresentation.setOnAction(event -> showWorkspace(DeckWorkspaceFactory.create(createAiProvider()), null));
        Button openFile = launcherButton("Open File", false);
        openFile.setOnAction(event -> openDocument());
        Button aiSettings = launcherButton("AI Settings", false);
        aiSettings.setOnAction(event -> showAiSettings());

        VBox actions = new VBox(10, newDocument, newSpreadsheet, newPresentation, openFile, aiSettings);
        actions.setAlignment(Pos.CENTER);
        VBox recentDocuments = new VBox(8);
        recentDocuments.setAlignment(Pos.CENTER);

        VBox content = new VBox(18, title, subtitle, actions, recentDocuments);
        content.getStyleClass().add("aurora-launcher");
        content.setAlignment(Pos.CENTER);
        content.setPadding(new Insets(48));

        setRoot(content, 940, 660);
        loadRecentDocuments(recentDocuments);
    }

    private Button launcherButton(String text, boolean primary) {
        Button button = new Button(text);
        button.setMinWidth(240);
        button.setPrefHeight(38);
        if (primary) {
            button.getStyleClass().add("aurora-primary-action");
        }
        return button;
    }

    private void showWorkspace(AuroraWorkspace workspace, Path path) {
        currentWorkspace = workspace;
        currentFile = path;
        currentShell = new AuroraEditorShell(workspace);
        currentShell.setOnKeyPressed(event -> {
            if (event.isShortcutDown() && event.getCode() == KeyCode.K) {
                showCommandPalette();
                event.consume();
            }
        });
        stage.setTitle(workspace.title() + " - " + AuroraVersion.PRODUCT_NAME);
        setRoot(currentShell, 1240, 780);
        currentShell.requestFocus();
    }

    private void showAiSettings() {
        currentWorkspace = null;
        currentShell = null;
        currentFile = null;

        Label title = new Label("AI Settings");
        title.getStyleClass().add("aurora-page-title");
        Label active = new Label("Active provider: " + settings.aiProvider());
        active.getStyleClass().add("aurora-body-text");

        ChoiceBox<String> provider = new ChoiceBox<>();
        provider.getItems().setAll(AiModelCatalog.providers());
        provider.getSelectionModel().select(settings.aiProvider());
        if (provider.getSelectionModel().isEmpty()) {
            provider.getSelectionModel().selectFirst();
        }
        provider.getSelectionModel().selectedItemProperty().addListener((observable, oldValue, newValue) ->
                active.setText("Active provider: " + newValue));

        TextField endpoint = new TextField();
        endpoint.setPromptText("Endpoint URL");
        endpoint.setText(settings.aiEndpoint());
        TextField model = new TextField();
        model.setPromptText("Model name");
        model.setText(settings.aiModel());
        TextField apiKey = new TextField();
        apiKey.setPromptText("API key for user-supplied API, stored locally");
        apiKey.setText(settings.aiApiKey());
        CheckBox modelDownloads = new CheckBox("Allow Gemma 4 model download/install after explicit confirmation");
        modelDownloads.setSelected(settings.allowModelDownloads());
        CheckBox redactSecrets = new CheckBox("Redact secrets before sending selected text");
        redactSecrets.setSelected(settings.redactSecrets());
        ChoiceBox<String> theme = new ChoiceBox<>();
        theme.getItems().setAll("macos-light", "macos-dark", "windows-light", "windows-dark", "linux-light", "linux-dark");
        theme.getSelectionModel().select(settings.theme());

        Label preview = new Label("Gemma 4 local uses Ollama with the public gemma4:e4b model after consent.");
        preview.getStyleClass().add("aurora-body-text");
        Button reviewPolicy = new Button("Review AI Policy");
        reviewPolicy.setOnAction(event -> {
            preview.setText("Aurora never downloads models or sends selected document text unless you choose a provider and confirm the action.");
        });
        Button saveSettings = new Button("Save Settings");
        saveSettings.setOnAction(event -> {
            AuroraSettings candidate = new AuroraSettings(
                    provider.getValue(),
                    endpoint.getText(),
                    model.getText(),
                    apiKey.getText(),
                    modelDownloads.isSelected(),
                    redactSecrets.isSelected(),
                    theme.getValue()
            );
            AuroraSettings updated = confirmModelConsentIfNeeded(candidate);
            runIo(() -> {
                settingsStore.save(updated);
                return updated;
            }, saved -> {
                settings = saved;
                AuroraTheme.apply(stage.getScene(), settings.theme());
                preview.setText("Settings saved locally.");
            });
        });

        GridPane form = new GridPane();
        form.setHgap(12);
        form.setVgap(12);
        form.addRow(0, new Label("AI Provider"), provider);
        form.addRow(1, new Label("Endpoint"), endpoint);
        form.addRow(2, new Label("Model"), model);
        form.addRow(3, new Label("API Key"), apiKey);
        form.addRow(4, new Label("Theme"), theme);
        form.add(modelDownloads, 1, 5);
        form.add(redactSecrets, 1, 6);
        form.add(reviewPolicy, 1, 7);
        form.add(saveSettings, 1, 8);
        form.add(preview, 1, 9);

        Button back = new Button("Back");
        back.setOnAction(event -> showLauncher());
        VBox panel = new VBox(16, title, active, form, back);
        panel.setPadding(new Insets(32));
        panel.setMaxWidth(680);

        BorderPane content = new BorderPane(panel);
        content.setPadding(new Insets(32));
        BorderPane.setAlignment(panel, Pos.TOP_CENTER);
        setRoot(content, 940, 660);
    }

    private AuroraSettings confirmModelConsentIfNeeded(AuroraSettings candidate) {
        if (!AiModelCatalog.requiresDownloadConsent(candidate.aiProvider()) || candidate.allowModelDownloads()) {
            return candidate;
        }
        Alert alert = new Alert(Alert.AlertType.CONFIRMATION);
        alert.setTitle("AI Model Consent");
        alert.setHeaderText("Allow Gemma 4 model download/install prompts?");
        alert.setContentText("Aurora will not download or install public Gemma 4 model files silently. "
                + "When a local Gemma 4 AI feature needs model files, Aurora will ask again before starting any download or install.");
        return alert.showAndWait()
                .filter(ButtonType.OK::equals)
                .map(ignored -> new AuroraSettings(
                        candidate.aiProvider(),
                        candidate.aiEndpoint(),
                        candidate.aiModel(),
                        candidate.aiApiKey(),
                        true,
                        candidate.redactSecrets(),
                        candidate.theme()
                ))
                .orElseGet(() -> new AuroraSettings(
                        AiModelCatalog.PROVIDER_NONE,
                        candidate.aiEndpoint(),
                        candidate.aiModel(),
                        candidate.aiApiKey(),
                        false,
                        candidate.redactSecrets(),
                        candidate.theme()
                ));
    }

    private AiProvider createAiProvider() {
        String provider = settings.aiProvider();
        if (AiModelCatalog.PROVIDER_NONE.equals(provider)) {
            return new DisabledAiProvider("AI is disabled. Open AI Settings to select Gemma 4 local or a user-supplied provider.");
        }
        if (AiModelCatalog.PROVIDER_GEMMA4_LOCAL.equals(provider) && !settings.allowModelDownloads()) {
            return new DisabledAiProvider("Gemma 4 local AI requires model download/install consent in AI Settings.");
        }

        String endpoint = settings.aiEndpoint().isBlank()
                ? AiModelCatalog.defaultEndpoint(provider)
                : settings.aiEndpoint();
        if (endpoint.isBlank()) {
            return new DisabledAiProvider("Configure an endpoint for " + provider + " in AI Settings.");
        }
        return new HttpAiProvider(
                provider,
                endpoint,
                settings.aiModel(),
                settings.aiApiKey(),
                AiModelCatalog.isOllamaNative(provider, endpoint)
        );
    }

    private void openDocument() {
        FileChooser chooser = new FileChooser();
        chooser.setTitle("Open Aurora Document");
        chooser.getExtensionFilters().add(new FileChooser.ExtensionFilter(
                "Aurora Documents", "*.awrite", "*.asheet", "*.adeck"));
        chooser.getExtensionFilters().add(new FileChooser.ExtensionFilter(
                "Microsoft Office", "*.docx", "*.xlsx", "*.pptx"));
        chooser.getExtensionFilters().add(new FileChooser.ExtensionFilter("All Files", "*.*"));
        File selected = chooser.showOpenDialog(stage);
        if (selected == null) {
            return;
        }

        openPath(selected.toPath());
    }

    private void openPath(Path path) {
        runIo(() -> importAnySupported(path), loaded -> {
            AuroraWorkspace workspace = workspaceFor(loaded.kind());
            workspace.loadDocumentJson(loaded.documentJson());
            showWorkspace(workspace, loaded.nativePackage() ? path : null);
            recordRecent(path);
            setStatus("Opened " + path.getFileName());
        });
    }

    private ImportedDocument importAnySupported(Path path) throws Exception {
        String fileName = path.getFileName().toString().toLowerCase();
        if (fileName.endsWith(".docx")) {
            return new ImportedDocument(AuroraDocumentKind.WRITE, docxFormatService.importWrite(path), false);
        }
        if (fileName.endsWith(".xlsx")) {
            return new ImportedDocument(AuroraDocumentKind.SHEET, xlsxFormatService.importSheet(path), false);
        }
        if (fileName.endsWith(".pptx")) {
            return new ImportedDocument(AuroraDocumentKind.DECK, pptxFormatService.importDeck(path), false);
        }
        var loaded = nativeFormatService.load(path);
        return new ImportedDocument(loaded.kind(), loaded.documentJson(), true);
    }

    private AuroraWorkspace workspaceFor(AuroraDocumentKind kind) {
        return switch (kind) {
            case WRITE -> WriteWorkspaceFactory.create(createAiProvider());
            case SHEET -> SheetWorkspaceFactory.create(createAiProvider());
            case DECK -> DeckWorkspaceFactory.create(createAiProvider());
        };
    }

    private void saveCurrent(boolean chooseLocation) {
        if (currentWorkspace == null) {
            return;
        }

        Path target = currentFile;
        if (chooseLocation || target == null) {
            FileChooser chooser = new FileChooser();
            chooser.setTitle("Save " + currentWorkspace.kind().documentLabel());
            chooser.getExtensionFilters().add(new FileChooser.ExtensionFilter(
                    currentWorkspace.kind().productName(), "*." + currentWorkspace.kind().extension()));
            chooser.setInitialFileName(defaultFileName(currentWorkspace.kind()));
            File selected = chooser.showSaveDialog(stage);
            if (selected == null) {
                return;
            }
            target = withExtension(selected.toPath(), currentWorkspace.kind().extension());
        }

        Path saveTarget = target;
        AuroraWorkspace workspace = currentWorkspace;
        setStatus("Saving...");
        runIo(() -> {
            nativeFormatService.save(saveTarget, workspace.kind(), workspace.title(), workspace.documentJson());
            return saveTarget;
        }, saved -> {
            currentFile = saved;
            recordRecent(saved);
            setStatus("Saved " + saved.getFileName());
        });
    }

    private void exportCurrentPdf() {
        if (currentWorkspace == null) {
            return;
        }
        FileChooser chooser = new FileChooser();
        chooser.setTitle("Export PDF");
        chooser.getExtensionFilters().add(new FileChooser.ExtensionFilter("PDF", "*.pdf"));
        chooser.setInitialFileName(currentWorkspace.title().replaceAll("[^A-Za-z0-9._-]+", " ") + ".pdf");
        File selected = chooser.showSaveDialog(stage);
        if (selected == null) {
            return;
        }
        Path target = withExtension(selected.toPath(), "pdf");
        AuroraWorkspace workspace = currentWorkspace;
        setStatus("Exporting PDF...");
        runIo(() -> {
            pdfExporter.exportText(target, workspace.title(), workspace.documentJson());
            return target;
        }, exported -> setStatus("Exported " + exported.getFileName()));
    }

    private void exportCurrentOffice() {
        if (currentWorkspace == null) {
            return;
        }
        String extension = switch (currentWorkspace.kind()) {
            case WRITE -> "docx";
            case SHEET -> "xlsx";
            case DECK -> "pptx";
        };
        FileChooser chooser = new FileChooser();
        chooser.setTitle("Export " + extension.toUpperCase());
        chooser.getExtensionFilters().add(new FileChooser.ExtensionFilter(extension.toUpperCase(), "*." + extension));
        chooser.setInitialFileName(currentWorkspace.title().replaceAll("[^A-Za-z0-9._-]+", " ") + "." + extension);
        File selected = chooser.showSaveDialog(stage);
        if (selected == null) {
            return;
        }
        Path target = withExtension(selected.toPath(), extension);
        AuroraWorkspace workspace = currentWorkspace;
        setStatus("Exporting " + extension.toUpperCase() + "...");
        runIo(() -> {
            switch (workspace.kind()) {
                case WRITE -> docxFormatService.exportWrite(target, workspace.documentJson());
                case SHEET -> xlsxFormatService.exportSheet(target, workspace.documentJson());
                case DECK -> pptxFormatService.exportDeck(target, workspace.documentJson());
            }
            return target;
        }, exported -> setStatus("Exported " + exported.getFileName()));
    }

    private void setRoot(Node content, double width, double height) {
        AuroraWindow root = new AuroraWindow();
        root.setTop(createMenuBar());
        root.setContent(content);
        root.addEventFilter(javafx.scene.input.KeyEvent.KEY_PRESSED, event -> {
            if (event.isShortcutDown() && event.getCode() == KeyCode.K) {
                showCommandPalette();
                event.consume();
            }
        });

        Scene scene = stage.getScene();
        if (scene == null) {
            scene = new Scene(root, width, height);
            stage.setScene(scene);
        } else {
            scene.setRoot(root);
        }
        AuroraTheme.apply(scene, settings.theme());
    }

    private MenuBar createMenuBar() {
        MenuBar menuBar = new MenuBar();
        menuBar.setUseSystemMenuBar(true);

        Menu file = new Menu("File");
        MenuItem newDocument = new MenuItem("New Document");
        newDocument.setAccelerator(new KeyCodeCombination(KeyCode.N, KeyCombination.SHORTCUT_DOWN));
        newDocument.setOnAction(event -> showWorkspace(WriteWorkspaceFactory.create(createAiProvider()), null));
        MenuItem newSpreadsheet = new MenuItem("New Spreadsheet");
        newSpreadsheet.setOnAction(event -> showWorkspace(SheetWorkspaceFactory.create(createAiProvider()), null));
        MenuItem newPresentation = new MenuItem("New Presentation");
        newPresentation.setOnAction(event -> showWorkspace(DeckWorkspaceFactory.create(createAiProvider()), null));
        MenuItem open = new MenuItem("Open File");
        open.setAccelerator(new KeyCodeCombination(KeyCode.O, KeyCombination.SHORTCUT_DOWN));
        open.setOnAction(event -> openDocument());
        MenuItem save = new MenuItem("Save");
        save.setDisable(currentWorkspace == null);
        save.setAccelerator(new KeyCodeCombination(KeyCode.S, KeyCombination.SHORTCUT_DOWN));
        save.setOnAction(event -> saveCurrent(false));
        MenuItem saveAs = new MenuItem("Save As");
        saveAs.setDisable(currentWorkspace == null);
        saveAs.setAccelerator(new KeyCodeCombination(KeyCode.S, KeyCombination.SHORTCUT_DOWN, KeyCombination.SHIFT_DOWN));
        saveAs.setOnAction(event -> saveCurrent(true));
        MenuItem exportPdf = new MenuItem("Export PDF");
        exportPdf.setDisable(currentWorkspace == null);
        exportPdf.setOnAction(event -> exportCurrentPdf());
        MenuItem exportOffice = new MenuItem("Export Office Format");
        exportOffice.setDisable(currentWorkspace == null);
        exportOffice.setOnAction(event -> exportCurrentOffice());
        MenuItem back = new MenuItem("Launcher");
        back.setOnAction(event -> showLauncher());
        MenuItem quit = new MenuItem("Quit");
        quit.setOnAction(event -> Platform.exit());
        file.getItems().addAll(newDocument, newSpreadsheet, newPresentation, new SeparatorMenuItem(),
                open, save, saveAs, exportPdf, exportOffice, new SeparatorMenuItem(), back, quit);

        Menu ai = new Menu("AI");
        MenuItem settings = new MenuItem("AI Settings");
        settings.setOnAction(event -> showAiSettings());
        MenuItem commandPalette = new MenuItem("Command Palette");
        commandPalette.setAccelerator(new KeyCodeCombination(KeyCode.K, KeyCombination.SHORTCUT_DOWN));
        commandPalette.setOnAction(event -> showCommandPalette());
        ai.getItems().addAll(settings, commandPalette);

        Menu plugins = new Menu("Plugins");
        for (PluginAction action : pluginRegistry.menuActions()) {
            MenuItem item = new MenuItem(action.label());
            item.setOnAction(event -> action.execute());
            plugins.getItems().add(item);
        }
        plugins.setDisable(plugins.getItems().isEmpty());

        menuBar.getMenus().addAll(file, ai, plugins);
        return menuBar;
    }

    private void showCommandPalette() {
        List<String> commands = currentWorkspace == null
                ? List.of("New Document", "New Spreadsheet", "New Presentation", "Open File", "AI Settings")
                : List.of("Save", "Save As", "New Document", "Open File", "AI Settings", "Launcher");

        AuroraCommandPalette.show(stage, commands, command -> {
            switch (command) {
                case "New Document" -> showWorkspace(WriteWorkspaceFactory.create(createAiProvider()), null);
                case "New Spreadsheet" -> showWorkspace(SheetWorkspaceFactory.create(createAiProvider()), null);
                case "New Presentation" -> showWorkspace(DeckWorkspaceFactory.create(createAiProvider()), null);
                case "Open File" -> openDocument();
                case "Save" -> saveCurrent(false);
                case "Save As" -> saveCurrent(true);
                case "AI Settings" -> showAiSettings();
                case "Launcher" -> showLauncher();
                default -> setStatus(command + " selected");
            }
        });
    }

    private void setStatus(String status) {
        if (currentShell != null) {
            currentShell.statusBar().setStatus(status);
        }
    }

    private void loadRecentDocuments(VBox container) {
        runIo(recentDocumentsStore::load, recent -> {
            container.getChildren().clear();
            if (recent.isEmpty()) {
                Label empty = new Label("No recent documents");
                empty.getStyleClass().add("aurora-launcher-subtitle");
                container.getChildren().add(empty);
                return;
            }
            Label heading = new Label("Recent Documents");
            heading.getStyleClass().add("aurora-panel-title");
            container.getChildren().add(heading);
            recent.stream().limit(5).forEach(path -> {
                Button button = launcherButton(path.getFileName().toString(), false);
                button.setOnAction(event -> openPath(path));
                container.getChildren().add(button);
            });
        });
    }

    private void recordRecent(Path path) {
        runIo(() -> {
            recentDocumentsStore.record(path);
            return path;
        }, ignored -> {
        });
    }

    private void startAutosave() {
        autosaveTimeline = new Timeline(new KeyFrame(Duration.seconds(60), event -> autosaveCurrent()));
        autosaveTimeline.setCycleCount(Timeline.INDEFINITE);
        autosaveTimeline.play();
    }

    private void autosaveCurrent() {
        AuroraWorkspace workspace = currentWorkspace;
        if (workspace == null) {
            return;
        }
        runIo(() -> autosaveStore.saveSnapshot(workspace.kind(), workspace.title(), workspace.documentJson()),
                snapshot -> setStatus("Autosaved " + snapshot.getFileName()));
    }

    private String defaultFileName(AuroraDocumentKind kind) {
        return "Untitled " + kind.documentLabel() + "." + kind.extension();
    }

    private Path withExtension(Path path, String extension) {
        String text = path.toString();
        if (text.toLowerCase().endsWith("." + extension)) {
            return path;
        }
        return Path.of(text + "." + extension);
    }

    private <T> void runIo(IoSupplier<T> supplier, Consumer<T> success) {
        CompletableFuture.supplyAsync(() -> {
            try {
                return supplier.get();
            } catch (Exception exception) {
                throw new CompletionException(exception);
            }
        }, ioExecutor).whenComplete((result, failure) -> Platform.runLater(() -> {
            if (failure != null) {
                Throwable cause = failure instanceof CompletionException && failure.getCause() != null
                        ? failure.getCause()
                        : failure;
                showError(cause);
            } else {
                success.accept(result);
            }
        }));
    }

    private void showError(Throwable failure) {
        Alert alert = new Alert(Alert.AlertType.ERROR);
        alert.setTitle(AuroraVersion.PRODUCT_NAME);
        alert.setHeaderText("Operation failed");
        alert.setContentText(failure.getMessage() == null ? failure.toString() : failure.getMessage());
        alert.showAndWait();
        setStatus("Operation failed");
    }

    @FunctionalInterface
    private interface IoSupplier<T> {
        T get() throws Exception;
    }

    private record ImportedDocument(AuroraDocumentKind kind, String documentJson, boolean nativePackage) {
    }
}
