# AGENTS.md — Java Cross-Platform Office Suite

## Project Name

**Aurora Office**

A fully Java-based, open-source, high-performance, cross-platform office suite for macOS, Windows, and Linux.

Aurora Office includes:

* **Aurora Write** — Word/Pages-style document editor
* **Aurora Sheet** — Excel/Numbers-style spreadsheet editor
* **Aurora Deck** — PowerPoint/Keynote-style presentation editor
* **Aurora AI** — integrated AI assistant using OpenAI-compatible APIs, Ollama, LM Studio, and local model servers

Primary design goal: **native-feeling macOS-first UX**, while remaining cross-platform.

---

# 1. Core Requirements

## 1.1 Technology Constraints

Use:

* Java 17 minimum
* Gradle
* JavaFX for UI
* Skia/Vulkan/Metal-backed rendering if needed later
* No Electron
* No web app shell
* No mandatory cloud backend

Allowed dependencies:

* JavaFX
* Apache POI for Office import/export
* PDFBox for PDF export/import support
* Jackson for JSON
* SQLite JDBC for local metadata/history
* Lucene for local search
* ICU4J for text handling
* Hunspell bindings or pure-Java spellchecking
* JNA/JNI only where native OS integration is unavoidable

Avoid:

* Swing for primary UI
* heavyweight reflection frameworks
* server-first architecture
* hard dependency on any paid AI provider

---

# 2. Product Vision

Aurora Office should feel like:

* Apple Pages / Numbers / Keynote on macOS
* Microsoft Office in power
* Obsidian/Notion in speed and modernity
* VS Code in extensibility
* LibreOffice in openness

The app should support:

* Native documents
* Microsoft Office file compatibility
* PDF export
* AI-assisted writing, formulas, summaries, charts, slides, editing, translation
* Offline-first usage
* Fast startup
* Large document handling
* Plugin architecture

---

# 3. Top-Level Architecture

Use a modular monorepo.

```text
aurora-office/
  AGENTS.md
  settings.gradle
  build.gradle

  modules/
    platform/
      aurora-platform-core/
      aurora-platform-ui/
      aurora-platform-rendering/
      aurora-platform-docmodel/
      aurora-platform-storage/
      aurora-platform-ai/
      aurora-platform-plugins/
      aurora-platform-collaboration/

    apps/
      aurora-launcher/
      aurora-write/
      aurora-sheet/
      aurora-deck/

    formats/
      aurora-format-native/
      aurora-format-docx/
      aurora-format-xlsx/
      aurora-format-pptx/
      aurora-format-pdf/
      aurora-format-odf/

    tests/
      aurora-testkit/
      aurora-golden-tests/
      aurora-performance-tests/
```

---

# 4. Native File Format

Use a ZIP-based package format.

```text
example.awrite
example.asheet
example.adeck
```

Internal structure:

```text
/document.json
/styles.json
/assets/
/metadata.json
/history/
/embeddings/
```

Use JSON for MVP.

Later versions may move hot document structures to binary columnar storage.

---

# 5. Application UI Design

## 5.1 macOS-First Visual Language

The UI should look close to Apple iWork:

* floating inspector panel on the right
* soft translucent sidebars
* large canvas-centered workspace
* native macOS menu bar
* rounded controls
* document tabs
* minimal toolbar
* contextual formatting controls
* command palette
* clean typography
* keyboard-first workflows

## 5.2 Main Window Layout

```text
┌──────────────────────────────────────────────────────────────┐
│ macOS titlebar / traffic lights / document title              │
├──────────────────────────────────────────────────────────────┤
│ Toolbar: Insert | Format | Table | Chart | Media | AI          │
├──────────────┬───────────────────────────────────┬────────────┤
│ Navigator    │                                   │ Inspector  │
│              │        Main Document Canvas        │            │
│ Pages/Sheets │                                   │ Style      │
│ Slides       │                                   │ Layout     │
│ Outline      │                                   │ AI         │
├──────────────┴───────────────────────────────────┴────────────┤
│ Status bar: page/sheet/slide info | zoom | sync | AI status    │
└──────────────────────────────────────────────────────────────┘
```

## 5.3 Aurora Write UI

Left sidebar:

* page thumbnails
* document outline
* comments
* search results

Center:

* paginated document canvas
* ruler
* margins
* headers/footers
* floating object handles

Right inspector:

* text style
* paragraph style
* page layout
* image properties
* table properties
* AI suggestions

Toolbar:

```text
Insert | Text | Table | Chart | Shape | Media | Comment | AI Rewrite
```

## 5.4 Aurora Sheet UI

Left sidebar:

* sheets list
* named ranges
* pivot tables
* data sources

Center:

* virtualized spreadsheet grid
* formula bar
* frozen rows/columns
* chart canvas
* table regions

Right inspector:

* cell format
* number format
* conditional formatting
* chart options
* formulas
* AI formula helper

Toolbar:

```text
Table | Chart | Formula | Sort | Filter | Pivot | AI Analyze
```

## 5.5 Aurora Deck UI

Left sidebar:

* slide thumbnails
* sections
* presenter notes

Center:

* slide canvas
* snap guides
* object alignment
* animation timeline

Right inspector:

* slide layout
* object style
* transitions
* animations
* speaker notes
* AI slide assistant

Toolbar:

```text
Theme | Insert | Shape | Media | Animate | Present | AI Generate
```

---

# 6. UI Implementation Rules

Use JavaFX.

Create reusable UI primitives:

```text
AuroraWindow
AuroraToolbar
AuroraSidebar
AuroraInspector
AuroraCanvas
AuroraCommandPalette
AuroraDocumentTab
AuroraStatusBar
AuroraPopover
AuroraSheetGrid
AuroraSlideNavigator
```

Use CSS themes:

```text
themes/
  macos-light.css
  macos-dark.css
  windows-light.css
  windows-dark.css
  linux-light.css
  linux-dark.css
```

Support:

* dark mode
* accent color
* high contrast
* keyboard navigation
* screen readers
* HiDPI
* touchpad gestures
* native file dialogs
* native menu bar on macOS

---

# 7. Rendering Engine

Implement a scene-graph-like document renderer.

Core concepts:

```java
interface RenderNode {
    Bounds layout(LayoutContext context);
    void paint(PaintContext context);
    List<HitResult> hitTest(Point2D point);
}
```

Document objects:

```text
TextRunNode
ParagraphNode
PageNode
TableNode
ImageNode
ShapeNode
ChartNode
SpreadsheetCellNode
SlideNode
GroupNode
```

Rendering goals:

* incremental layout
* dirty-region repainting
* virtualized pages/sheets/slides
* separate model and view
* zoom-independent rendering
* stable print/PDF output

---

# 8. Document Model

Use immutable-ish command-based editing.

```java
interface DocumentCommand {
    void apply(DocumentModel model);
    void undo(DocumentModel model);
}
```

Examples:

```text
InsertTextCommand
DeleteTextCommand
ApplyStyleCommand
InsertTableCommand
ResizeColumnCommand
InsertSlideCommand
MoveObjectCommand
```

Benefits:

* undo/redo
* collaboration later
* autosave
* crash recovery
* AI changes as reviewable patches

---

# 9. Aurora Write Model

Core model:

```text
Document
  Sections
    Pages
      Blocks
        Paragraph
        Table
        Image
        Shape
```

Features:

* rich text
* styles
* headers/footers
* footnotes
* comments
* track changes
* tables
* images
* shapes
* page layout
* export to PDF/DOCX

MVP features:

* paragraphs
* headings
* bold/italic/underline
* lists
* page canvas
* save/load
* PDF export
* DOCX import/export basic

---

# 10. Aurora Sheet Model

Core model:

```text
Workbook
  Sheets
    Cells
    Formulas
    Tables
    Charts
```

Formula engine:

```java
interface FormulaFunction {
    CellValue evaluate(EvaluationContext context, List<CellValue> args);
}
```

MVP formula support:

```text
SUM
AVERAGE
MIN
MAX
COUNT
IF
CONCAT
ROUND
VLOOKUP later
```

Performance requirements:

* virtualized grid
* sparse cell storage
* dependency graph for formulas
* incremental recalculation
* background calculation thread

---

# 11. Aurora Deck Model

Core model:

```text
Deck
  Theme
  Slides
    Objects
      TextBox
      Image
      Shape
      Chart
      Table
```

MVP features:

* create slides
* text boxes
* images
* shapes
* themes
* presenter mode later
* PPTX import/export basic
* PDF export

---

# 12. AI Integration

Aurora AI should support:

* OpenAI-compatible APIs
* Ollama
* LM Studio
* Anthropic-compatible API later
* custom local endpoint

Provider interface:

```java
public interface AiProvider {
    String id();
    boolean supportsStreaming();
    AiResponse complete(AiRequest request);
    Flow.Publisher<AiToken> stream(AiRequest request);
}
```

Use cases:

## Aurora Write

* rewrite paragraph
* summarize document
* change tone
* grammar suggestions
* generate outline
* explain selected text
* translate
* create citations placeholder
* turn notes into report

## Aurora Sheet

* generate formulas
* explain formulas
* detect anomalies
* summarize table
* generate charts
* clean data
* classify rows

## Aurora Deck

* generate deck from outline
* rewrite slide
* speaker notes
* simplify slide
* generate title options
* convert document to slides

AI changes must be shown as patches before applying.

Never silently modify user content.

---

# 13. AI Privacy Rules

Default behavior:

* local-only unless user configures cloud API
* clearly show which provider is active
* show token usage where possible
* never send hidden document content without user action
* selection-based AI by default
* whole-document AI requires confirmation
* redact secrets option

Settings UI:

```text
AI Provider:
  None
  Ollama
  LM Studio
  OpenAI-compatible
  Custom HTTP endpoint
```

---

# 14. Performance Requirements

Startup:

* launcher visible under 1.5 seconds on modern hardware
* editor open under 3 seconds

Write:

* 500-page document should remain navigable
* only visible pages should render

Sheet:

* 1 million rows possible through virtualization
* sparse storage
* formula recalculation must be dependency-based

Deck:

* 500-slide deck navigable
* thumbnails generated asynchronously

General:

* no blocking UI thread
* all IO async
* rendering and layout jobs cancellable
* autosave incremental

---

# 15. Plugin System

Plugin API:

```java
public interface AuroraPlugin {
    String id();
    String name();
    void activate(AuroraPluginContext context);
    void deactivate();
}
```

Plugin capabilities:

* add menu item
* add toolbar action
* add AI command
* add import/export format
* add formula function
* add document inspector panel

Plugins loaded from:

```text
~/.aurora/plugins/
```

Use Java ServiceLoader for MVP.

---

# 16. Import/Export

MVP:

* native format
* PDF export
* DOCX import/export basic
* XLSX import/export basic
* PPTX import/export basic

Later:

* ODT/ODS/ODP
* EPUB
* HTML
* Markdown
* CSV
* LaTeX export

Compatibility should improve incrementally.

Never block MVP on perfect Microsoft Office compatibility.

---

# 17. Testing Strategy

Use:

* JUnit 5
* golden rendering tests
* property-based formula tests
* file roundtrip tests
* performance benchmarks
* UI smoke tests

Required tests:

```text
Document save/load roundtrip
Undo/redo correctness
DOCX basic import
XLSX formula import
PPTX slide import
PDF export smoke test
Large document scroll benchmark
Large sheet scroll benchmark
AI provider mock tests
```

---

# 18. Milestones

## Milestone 0 — Repo Skeleton

Deliver:

* Gradle multi-module project
* JavaFX launcher
* app shell
* theme system
* basic menus
* empty Write/Sheet/Deck apps

## Milestone 1 — Aurora Write MVP

Deliver:

* paginated editor
* text editing
* paragraph styles
* save/load native format
* PDF export
* basic DOCX import/export
* AI rewrite selected text

## Milestone 2 — Aurora Sheet MVP

Deliver:

* virtualized grid
* cell editing
* formula bar
* sparse workbook model
* basic formulas
* native save/load
* XLSX import/export
* AI formula helper

## Milestone 3 — Aurora Deck MVP

Deliver:

* slide navigator
* slide canvas
* text boxes
* images
* shapes
* themes
* native save/load
* PPTX import/export
* AI deck generation from outline

## Milestone 4 — Unified Polish

Deliver:

* command palette
* inspector polish
* autosave
* recent documents
* global search
* plugin API
* settings app
* dark mode
* keyboard shortcuts

## Milestone 5 — Performance

Deliver:

* large document benchmarks
* large workbook benchmarks
* async thumbnailing
* dirty-region rendering
* memory profiling
* startup profiling

---

# 19. Coding Rules for Codex Agents

All agents must follow these rules:

1. Prefer small, reviewable commits.
2. Never introduce global mutable state unless justified.
3. Keep model, rendering, and UI separate.
4. Never block the JavaFX Application Thread.
5. Every command must support undo/redo where applicable.
6. Every file format feature must have roundtrip tests.
7. AI features must work with mock providers in tests.
8. Do not hardcode OpenAI or Ollama as the only provider.
9. Never send user document contents to AI automatically.
10. Keep Java 17 compatibility.
11. Avoid native code unless absolutely necessary.
12. Favor performance-conscious data structures.
13. Add Javadoc for public APIs.
14. Use package-private classes for internals.
15. Every module must have a clear README.

---

# 20. Suggested Package Layout

```text
com.aurora.platform.app
com.aurora.platform.ui
com.aurora.platform.render
com.aurora.platform.model
com.aurora.platform.storage
com.aurora.platform.ai
com.aurora.platform.plugin

com.aurora.write
com.aurora.write.model
com.aurora.write.render
com.aurora.write.commands

com.aurora.sheet
com.aurora.sheet.model
com.aurora.sheet.formula
com.aurora.sheet.render

com.aurora.deck
com.aurora.deck.model
com.aurora.deck.render
```

---

# 21. Initial UI Mockup CSS Direction

Use soft macOS styling.

```css
.root {
    -fx-font-family: "SF Pro Text", "Inter", "System";
    -fx-background-color: #f5f5f7;
}

.aurora-toolbar {
    -fx-background-color: rgba(248, 248, 248, 0.86);
    -fx-border-color: rgba(0, 0, 0, 0.08);
    -fx-padding: 8 12 8 12;
}

.aurora-sidebar {
    -fx-background-color: rgba(245, 245, 247, 0.78);
    -fx-border-color: rgba(0, 0, 0, 0.08);
}

.aurora-inspector {
    -fx-background-color: rgba(250, 250, 252, 0.92);
    -fx-border-color: rgba(0, 0, 0, 0.08);
}

.aurora-canvas {
    -fx-background-color: #ececef;
}

.aurora-page {
    -fx-background-color: white;
    -fx-effect: dropshadow(gaussian, rgba(0,0,0,0.18), 20, 0.2, 0, 4);
}
```

---

# 22. MVP First Task for Codex

Implement Milestone 0.

Required output:

```text
- Java 17 Gradle multi-module project
- JavaFX launcher
- macOS-inspired shell
- three app modes: Write, Sheet, Deck
- left navigator
- center canvas
- right inspector
- toolbar
- status bar
- theme CSS
- placeholder AI settings screen
```

The first runnable command should be:

```bash
./gradlew :modules:apps:aurora-launcher:run
```

The first visible app should show:

```text
Aurora Office
New Document
New Spreadsheet
New Presentation
Open File
AI Settings
```

---

# 23. Non-Goals for MVP

Do not implement these first:

* realtime collaboration
* perfect DOCX/XLSX/PPTX compatibility
* cloud sync
* mobile apps
* marketplace
* complex animations
* VBA/macros
* full Excel formula compatibility
* OCR
* handwritten notes

---

# 24. Definition of Done

The project is acceptable when:

* it builds on Java 17
* the launcher opens
* each editor opens in a native-feeling shell
* the UI is responsive
* basic native save/load works
* tests pass
* AI provider settings exist
* mock AI provider works
* no document content is sent externally without explicit user action

---

# 25. Long-Term Direction

Aurora Office should eventually become:

* a real Java-native alternative to Microsoft Office
* fully offline-capable
* AI-native but privacy-respecting
* extensible
* beautiful on macOS
* reliable on Windows and Linux
* fast enough for large professional documents

