# Aurora Office

Aurora Office is a Java 17, JavaFX-based office suite prototype with a macOS-first desktop shell.

## Run on macOS

```bash
./run_macos.sh
```

The Gradle task requested by the project brief also works:

```bash
./gradlew :modules:apps:aurora-launcher:run
```

The first run downloads JavaFX and test dependencies from Maven Central. The macOS runner and local `gradlew` script use a project-local Gradle cache by default so the app is not affected by stale global Gradle native binaries.

## Current Scope

- Gradle multi-module monorepo
- JavaFX launcher
- macOS-inspired editor shell
- Write, Sheet, and Deck workspaces
- Native ZIP package save/load for `.awrite`, `.asheet`, and `.adeck`
- Background autosave snapshots for open documents
- PDF export smoke path
- Persistent local AI/theme settings and recent documents
- Command-based text model with undo/redo
- Basic sparse worksheet formula engine
- Plugin API foundation
- Rendering node contracts
- AI settings for Gemma 4 local, Ollama, LM Studio, vLLM, llama.cpp, OpenAI-compatible, and custom HTTP endpoints
- Gemma 4 local defaults to Ollama's documented `gemma4:e4b` model profile
- Explicit Gemma 4 model download/install consent before local model setup is allowed
- Selected-text AI rewrite, Sheet formula helper, and Deck slide generation with user review before applying changes
- AI calls run off the JavaFX thread and document context is sent only after explicit user confirmation
- Basic DOCX import/export for Write plain paragraphs
- Basic XLSX import/export for Sheet sparse cells and formulas
- Basic PPTX import/export for Deck title/body slides
- Theme CSS for macOS, Windows, and Linux variants
- Focused storage, AI provider, command, rendering, plugin, formula, native/Office format, PDF, and large-document smoke tests

## MVP Boundaries

DOCX, XLSX, and PPTX compatibility is basic and tested, not fidelity-complete. Current support covers plain Write paragraphs, sparse Sheet cells/formulas, and single-slide Deck title/body content. Rich Office layout fidelity, tracked changes, charts, animations, advanced formatting, and full collaborative editing remain roadmap work.
