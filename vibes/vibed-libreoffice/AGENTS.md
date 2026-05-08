### AGENTS.MD

#### Overview
This document defines a complete, actionable, end‑to‑end plan to build a **macOS‑only SwiftUI office suite** that uses the **LibreOffice core as a headless engine** and a **native SwiftUI frontend**. It is written as a set of autonomous agents and concrete scripts you can feed to automation tools (Gemini CLI, Claude Code, or any orchestration runner). Each agent has a single responsibility, clear inputs, outputs, and exact commands to run. Follow the agents in order to go from zero to a signed macOS .app that uses LibreOffice as the document engine.

---

#### Agents Summary Table

| **Agent** | **Responsibility** | **Primary Output** |
|---|---:|---|
| **bootstrap** | Prepare macOS build environment and install dependencies | `env-setup.log` |
| **librecore_build** | Build LibreOffice core as headless engine and LibreOfficeKit (LOK) C API | `libreoffice-headless/lib` and `libreofficekit/liblok.dylib` |
| **lok_wrapper** | Build a small C wrapper around LibreOfficeKit exposing a stable C API for Swift | `lok_wrapper/liblok_wrapper.dylib` and headers |
| **swiftui_frontend** | Create SwiftUI app skeleton, integrate wrapper, render canvas, basic editor UI | Xcode project `OfficeSwiftUI.xcodeproj` |
| **render_bridge** | Implement rendering pipeline: render pages to CALayer/NSImage and map input events | `RenderBridge.swift` |
| **command_integration** | Implement command/undo system and mapping to LibreOffice core operations | `CommandSystem.swift` |
| **docx_exporter** | Provide DOCX import/export glue using LibreOffice core filters | `exporter/` scripts |
| **tests_ci** | Unit, integration, and UI tests; CI pipeline for macOS builds | `.github/workflows/ci.yml` |
| **packaging** | Code signing, notarization, and .app packaging scripts | `package.sh`, notarization artifacts |
| **docs_legal** | Generate contributor docs, license compliance checklist, and MPL compliance artifacts | `LEGAL.md`, `CONTRIBUTING.md` |

---

#### Prerequisites
- **macOS** 12+ (preferably latest macOS LTS for Xcode compatibility).  
- **Xcode** (latest stable) and **Xcode command line tools**.  
- **Homebrew** for installing dependencies.  
- **Git** and a GitHub (or GitLab) repo to host the workspace.  
- Familiarity with building C++ projects on macOS and Swift/SwiftUI development.  
- A developer Apple ID for signing and notarization.

**Install base packages** (run in Terminal):

```bash
# Install Homebrew if missing
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Developer tools
brew install git cmake pkg-config python3 autoconf automake libtool gettext

# Optional helpful tools
brew install ninja yasm nasm
```

---

#### Implementation Steps
Follow these steps in order. Each step is an agent you can automate.

---

##### 1 Bootstrap Agent
**Goal**: Create workspace, install dependencies, and record environment.

**Commands**:

```bash
mkdir -p ~/Projects/OfficeNative
cd ~/Projects/OfficeNative
git init
echo "workspace created $(date)" > env-setup.log
brew list > brew-packages.log
xcode-select -p >> env-setup.log
```

**Outputs**: `env-setup.log`, `brew-packages.log`

---

##### 2 LibreOffice Core Build Agent
**Goal**: Build LibreOffice headless core and LibreOfficeKit (LOK) for macOS.

**Notes**: Use LibreOfficeKit (LOK) which provides a C API for headless rendering and editing. Build with GUI disabled where possible and produce a shared library you can call from Swift.

**High‑level commands**:

```bash
cd ~/Projects/OfficeNative
git clone https://git.libreoffice.org/core libreoffice-core
cd libreoffice-core

# Prepare build environment (example; adapt to LibreOffice build docs)
./autogen.sh --with-system-cairo --with-system-libpng --with-system-libjpeg \
  --without-gui --enable-gtk3=no --enable-kde=no

# Build (this is heavy and may take hours)
make -j$(sysctl -n hw.ncpu)

# Build LibreOfficeKit if separate
cd libreofficekit
# follow LOK build instructions; produce liblok.dylib
make -j$(sysctl -n hw.ncpu)
```

**Outputs**: `libreoffice-core/program/` binaries, `libreofficekit/liblok.dylib`

**Agent output artifact**: `artifacts/libreoffice-headless.tar.gz` (pack built libs and headers)

---

##### 3 LOK Wrapper Agent
**Goal**: Create a thin, stable C wrapper around LibreOfficeKit to simplify Swift bridging and to enforce a small ABI surface.

**Why**: Swift interop with C is straightforward; wrapping LOK into a small, well-documented C API reduces coupling and isolates future LOK changes.

**Wrapper design**:
- `lok_wrapper.h` exposes:
  - `lok_open_document(const char* path) -> void*`
  - `lok_render_page(void* doc, int page, unsigned char** out_png, int* size)`
  - `lok_apply_command(void* doc, const char* cmd_json)`
  - `lok_save_document(void* doc, const char* path, const char* format)`
  - `lok_close_document(void* doc)`

**Build commands**:

```bash
cd ~/Projects/OfficeNative
mkdir -p lok_wrapper && cd lok_wrapper

# Create wrapper sources (lok_wrapper.c, lok_wrapper.h)
# Example compile
clang -shared -o liblok_wrapper.dylib lok_wrapper.c -L../libreofficekit -llok -I../libreofficekit/include
```

**Outputs**: `lok_wrapper/liblok_wrapper.dylib`, `lok_wrapper/lok_wrapper.h`

---

##### 4 SwiftUI Frontend Agent
**Goal**: Create Swift package and Xcode project, integrate the C wrapper, and scaffold the SwiftUI app.

**Project layout**:

```
OfficeSwiftUI/
 ├─ Package.swift
 ├─ Sources/
 │   ├─ AppMain/
 │   └─ RenderBridge/
 ├─ Resources/
 └─ ThirdParty/
     └─ lok_wrapper/ (lib + headers)
```

**Create project**:

```bash
cd ~/Projects/OfficeNative
swift package init --type executable --name OfficeSwiftUI
open Package.swift
```

**Add C library bridging**:
- Copy `liblok_wrapper.dylib` and `lok_wrapper.h` into `ThirdParty/lok_wrapper`.
- In `Package.swift`, add a `systemLibrary` or `binaryTarget` to link the dylib.
- Add a bridging header or use Swift’s C interop by creating a module map.

**Minimal Swift bridging example** (`lok_bridge.h`):

```c
// lok_bridge.h
#include "lok_wrapper.h"
```

**Swift usage** (`LokBridge.swift`):

```swift
import Foundation

// C functions are available directly if module map is configured
func openDoc(path: String) -> UnsafeMutableRawPointer? {
    return lok_open_document(path)
}
```

**UI skeleton**:
- Main window with three panes: left thumbnails, center canvas, right inspector.
- Canvas uses `NSViewRepresentable` or `CALayer` backed view to display rendered pages.

**Build**:

```bash
swift build
open Package.swift # or open Xcode project and run
```

**Outputs**: `OfficeSwiftUI.app` (debug)

---

##### 5 Render Bridge Agent
**Goal**: Implement `RenderBridge.swift` that:
- Calls `lok_render_page` to get PNG bytes
- Converts bytes to `NSImage`
- Displays `NSImage` in SwiftUI canvas with zoom/pan
- Maps mouse/keyboard events to document coordinates and forwards to `lok_apply_command`

**Key code sketch**:

```swift
import SwiftUI
import AppKit

struct PageCanvas: NSViewRepresentable {
    let docPtr: UnsafeMutableRawPointer

    func makeNSView(context: Context) -> NSImageView {
        let iv = NSImageView()
        iv.imageScaling = .scaleProportionallyUpOrDown
        return iv
    }

    func updateNSView(_ nsView: NSImageView, context: Context) {
        // call C wrapper
        var size: Int32 = 0
        var pngPtr: UnsafeMutablePointer<UInt8>?
        lok_render_page(docPtr, 0, &pngPtr, &size)
        if let pngPtr = pngPtr {
            let data = Data(bytes: pngPtr, count: Int(size))
            nsView.image = NSImage(data: data)
            lok_free_buffer(pngPtr) // if wrapper provides free
        }
    }
}
```

**Outputs**: `RenderBridge.swift` integrated into app

---

##### 6 Command Integration Agent
**Goal**: Implement a command/undo/redo system in Swift that maps UI actions to JSON commands sent to the wrapper (`lok_apply_command`).

**Design**:
- `Command` protocol with `apply()` and `undo()` implemented by the frontend.
- Commands are serialized to a small JSON DSL that the C wrapper translates into UNO operations.

**Example command JSON**:

```json
{ "op": "insert_text", "pos": 123, "text": "Hello" }
```

**Agent script**: create `CommandSystem.swift` and wire toolbar actions to commands.

**Outputs**: `CommandSystem.swift`, `UndoRedoManager`

---

##### 7 DOCX Exporter Agent
**Goal**: Use LibreOffice core filters to export/import DOCX. Provide a CLI wrapper script `office-export` that calls the headless core to convert formats.

**Example script**:

```bash
#!/usr/bin/env bash
# office-export
INPUT="$1"
OUTPUT="$2"
FORMAT="$3" # e.g., docx, pdf

# Use LibreOffice headless conversion
/opt/libreoffice/program/soffice --headless --convert-to ${FORMAT} --outdir "$(dirname "$OUTPUT")" "$INPUT"
```

**Outputs**: `exporter/office-export` CLI

---

##### 8 Tests and CI Agent
**Goal**: Add unit tests for wrapper, integration tests for rendering, and UI tests for SwiftUI flows. Create GitHub Actions workflow for macOS runners.

**CI file** `.github/workflows/ci.yml` (sketch):

```yaml
name: macOS CI
on: [push, pull_request]
jobs:
  build:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install deps
        run: brew bundle --file=Brewfile || true
      - name: Build LibreOffice headless (optional cached)
        run: ./scripts/build-libreoffice.sh
      - name: Build Swift app
        run: swift build -c release
      - name: Run tests
        run: swift test
```

**Outputs**: CI status, test artifacts

---

##### 9 Packaging Agent
**Goal**: Produce signed, notarized `.app` and `.pkg` for distribution.

**Steps**:

1. **Code sign** the app:

```bash
codesign --deep --force --options runtime --sign "Developer ID Application: Your Name (TEAMID)" \
  path/to/OfficeSwiftUI.app
```

2. **Create a notarization ZIP**:

```bash
ditto -c -k --sequesterRsrc --keepParent OfficeSwiftUI.app OfficeSwiftUI.zip
xcrun altool --notarize-app -f OfficeSwiftUI.zip --primary-bundle-id com.yourorg.OfficeSwiftUI -u "appleid" -p "app-specific-password"
# Poll for notarization status, then staple
xcrun stapler staple OfficeSwiftUI.app
```

3. **Create installer** (optional):

```bash
pkgbuild --component OfficeSwiftUI.app /Applications --sign "Developer ID Installer: Your Name (TEAMID)" OfficeSwiftUI.pkg
```

**Outputs**: `OfficeSwiftUI.app`, `OfficeSwiftUI.pkg`, notarization receipts

---

##### 10 Docs and Legal Agent
**Goal**: Produce `LEGAL.md` and `CONTRIBUTING.md` that explain MPL obligations, how to keep LibreOffice files unchanged, and how to license your SwiftUI code.

**Key points to include**:
- LibreOffice core files remain under MPL 2.0; any modifications to those files must be released under MPL.
- Your SwiftUI frontend can be MIT/Apache‑2.0 dual licensed.
- Provide a clear separation of directories: `third_party/libreoffice/` (MPL) vs `frontend/` (MIT).
- Include build instructions to reproduce the LibreOffice build (source, commit hash).
- Provide a contributor license and a patent pledge if desired.

**Outputs**: `LEGAL.md`, `CONTRIBUTING.md`

---

#### Automation Scripts and Agent Prompts
Below are concrete agent scripts and the exact prompts you can feed to an automation runner. Each agent is idempotent and writes logs to `./artifacts/agentname.log`.

---

##### Agent Script Template
Save as `agents/<agentname>.sh` and make executable.

```bash
#!/usr/bin/env bash
set -euo pipefail
LOG=./artifacts/<agentname>.log
mkdir -p artifacts
echo "Running <agentname> at $(date)" > $LOG

# Commands for the agent go here
# Example:
# git clone ...
# ./configure ...
# make -j$(sysctl -n hw.ncpu) >> $LOG 2>&1

echo "<agentname> completed at $(date)" >> $LOG
```

Replace `<agentname>` with each agent name from the Agents Summary Table and fill the commands from the Implementation Steps.

---

##### Example Agent Prompts for Gemini CLI or Claude Code
Use these as the agent prompt body for each automation tool. They are intentionally prescriptive.

**bootstrap agent prompt**:
```
Task: Prepare macOS build environment for OfficeNative workspace.
Steps:
1. Create ~/Projects/OfficeNative and initialize git.
2. Install Homebrew packages: git, cmake, pkg-config, python3, autoconf, automake, libtool, gettext, ninja.
3. Record environment to artifacts/env-setup.log.
Produce: artifacts/env-setup.log
```

**librecore_build agent prompt**:
```
Task: Build LibreOffice core headless and LibreOfficeKit for macOS.
Steps:
1. Clone LibreOffice core into libreoffice-core.
2. Run autogen.sh with headless options.
3. Build with make -jN.
4. Build LibreOfficeKit and produce liblok.dylib.
5. Package built libs and headers into artifacts/libreoffice-headless.tar.gz.
Produce: artifacts/libreoffice-headless.tar.gz
```

**lok_wrapper agent prompt**:
```
Task: Build C wrapper around LibreOfficeKit.
Steps:
1. Create lok_wrapper.c and lok_wrapper.h implementing the small C API described in AGENTS.MD.
2. Compile into liblok_wrapper.dylib linking against liblok.
3. Place header and dylib into ThirdParty/lok_wrapper.
Produce: ThirdParty/lok_wrapper/liblok_wrapper.dylib and lok_wrapper.h
```

Repeat similarly for other agents.

---

#### CI/CD and Release Automation
- Use GitHub Actions macOS runners to run `bootstrap`, `librecore_build` (or use prebuilt LOK artifacts cached), `lok_wrapper`, `swiftui_frontend`, `tests_ci`, and `packaging`.
- Cache LibreOffice build artifacts using GitHub Actions cache or an external artifact store because building LibreOffice is time consuming.
- Use secrets for Apple ID and app‑specific password for notarization.

**CI tips**:
- Split LibreOffice build into a separate workflow that runs less frequently and uploads artifacts.
- Keep the SwiftUI build and tests in a fast workflow that downloads the prebuilt LOK artifact.

---

#### Licensing and Compliance Checklist
- **Keep LibreOffice sources and any modified LibreOffice files under MPL 2.0**.
- **Document exactly which files are MPL** in `LEGAL.md`.
- **License your SwiftUI frontend** under MIT or MIT/Apache dual license.
- **Do not rebrand LibreOffice trademarks**; avoid using the LibreOffice name in product branding unless permitted.
- **Provide build reproducibility**: commit hashes, build flags, and environment logs for LibreOffice builds.

---

#### Timeline and Milestones (Suggested)
- **Week 0**: Environment setup, repo skeleton, bootstrap agent.
- **Weeks 1–4**: Build LibreOffice headless and LOK; create wrapper.
- **Weeks 3–8**: SwiftUI skeleton, render bridge, basic open/save.
- **Weeks 8–12**: Command system, undo/redo, basic editing flows.
- **Weeks 12–20**: DOCX import/export, templates, images, tables.
- **Weeks 20–28**: Tests, CI, packaging, notarization, polish.
- **Month 8+**: Beta testing, performance tuning, accessibility.

---

#### Risks and Mitigations
- **Risk**: LibreOffice build complexity and long compile times.  
  **Mitigation**: Use prebuilt LOK artifacts and CI caching; run builds on powerful macOS runners or dedicated macOS hardware.

- **Risk**: API mismatch between LOK and required editing features.  
  **Mitigation**: Keep wrapper small and add features incrementally; fallback to UNO bindings if needed.

- **Risk**: Licensing confusion.  
  **Mitigation**: Keep strict directory separation and document license boundaries in `LEGAL.md`.

- **Risk**: UI performance for large documents.  
  **Mitigation**: Render pages lazily, use tiled rendering, and keep a lightweight in‑memory cache of rendered pages.

---

#### Deliverables Produced by Agents
- `artifacts/env-setup.log`  
- `artifacts/libreoffice-headless.tar.gz`  
- `ThirdParty/lok_wrapper/liblok_wrapper.dylib` and headers  
- `OfficeSwiftUI/` Xcode project and Swift package  
- `exporter/office-export` CLI script  
- `.github/workflows/ci.yml`  
- `package.sh` and notarization receipts  
- `LEGAL.md` and `CONTRIBUTING.md`

---

#### Final Notes and Next Steps
1. **Run the agents in order**: `bootstrap`, `librecore_build`, `lok_wrapper`, `swiftui_frontend`, `render_bridge`, `command_integration`, `docx_exporter`, `tests_ci`, `packaging`, `docs_legal`. Each agent writes logs to `./artifacts/` for traceability.  
2. **Iterate quickly** on the wrapper API — start with read/render/save and add editing commands later.  
3. **Prioritize UX polish**: native menus, keyboard shortcuts, and macOS accessibility will make the app feel first‑class.  
4. **Keep legal separation** between MPL code and your MIT frontend; document everything.

