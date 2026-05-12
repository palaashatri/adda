# PyForge — Project Status

> Generated: 2026-05-12 | Python 3.10+ | Tkinter + customtkinter

## Overview

PyForge is a local, offline, monolithic AI studio. 31 Python files (~1,270 LOC) across 8 modules. Built against the spec in `INSTRUCTIONS.md`.

---

## File Inventory (vs INSTRUCTIONS.md spec)

| Status | File | Notes |
|--------|------|-------|
| ✅ | `main.py` | Entry point (11 lines) |
| ✅ | `app.py` | App bootstrap + tab wiring (83 lines) |
| ✅ | `requirements.txt` | 13 packages listed |
| ✅ | `README.md` | Basic setup docs |
| ✅ | `LICENSE` | MIT |
| ✅ | `core/config.py` | Creates `~/.pyforge/` dirs on import |
| ✅ | `core/logger.py` | Stream handler logging |
| ✅ | `core/device.py` | CUDA > MPS > CPU |
| ✅ | `core/scheduler.py` | `EngineRequest` / `EngineResponse` dataclasses |
| ✅ | `core/task_queue.py` | ThreadPoolExecutor (2 workers), submit/cancel |
| ⚠️ | `engines/engine_image.py` | **txt2img only** — missing img2img, inpainting, upscaling |
| ⚠️ | `engines/engine_video.py` | **Mock only** — no real model integration |
| ✅ | `engines/engine_audio.py` | Bark TTS via transformers |
| ✅ | `engines/engine_llm.py` | transformers text-generation pipeline |
| ⚠️ | `engines/engine_speech.py` | **ASR only** — no TTS |
| ✅ | `models/registry.py` | get/set/list default + installed models |
| ✅ | `models/downloader.py` | HF search + snapshot_download |
| ✅ | `models/metadata.py` | ModelInfo dataclass + JSON I/O |
| ✅ | `ui/theme.py` | Dark mode + blue theme |
| ✅ | `ui/main_window.py` | 6-tab responsive window |
| ✅ | `ui/tabs/tab_image.py` | txt2img UI wired |
| ✅ | `ui/tabs/tab_video.py` | Wired to mock |
| ✅ | `ui/tabs/tab_audio.py` | Wired to Bark |
| ✅ | `ui/tabs/tab_llm.py` | Wired to transformers |
| ✅ | `ui/tabs/tab_speech.py` | ASR only, no file picker |
| ✅ | `ui/tabs/tab_settings.py` | HF search + download UI |
| ✅ | `ui/components/prompt_box.py` | Reusable prompt widget |
| ✅ | `ui/components/output_viewer.py` | Image + text viewer |
| ⚠️ | `ui/components/history_panel.py` | **In-memory only** — no disk persistence |
| ✅ | `ui/components/model_selector.py` | Dropdown + active model |
| ✅ | `ui/components/progress_bar.py` | Wraps ctk progress |
| ❌ | `assets/icons/` | **Missing** — referenced by `package.py` |
| ❌ | `assets/themes/` | **Missing** — spec placeholder |
| ⚠️ | `scripts/benchmark.py` | **Stub** — no-op |
| ⚠️ | `scripts/cleanup.py` | **Stub** — no-op |
| ✅ | `scripts/package.py` | Windows .exe + macOS .app (bonus) |

---

## Known Gaps

### 🔴 Blocking

| Issue | File |
|-------|------|
| `customtkinter` not installed in current env | `requirements.txt` |
| All blocking engine calls lack cancellation | `engines/*.py` |

### 🟡 High Priority

| Issue | File |
|-------|------|
| Video engine is a mock (`time.sleep(2)`) | `engines/engine_video.py` |
| Image engine missing img2img, inpainting, upscaling | `engines/engine_image.py` |
| Speech engine has no TTS (text→speech) | `engines/engine_speech.py` |
| No history persistence to disk | `ui/components/history_panel.py` |

### 🟢 Low Priority

| Issue | File |
|-------|------|
| Downloader lacks resume, retry, checksum | `models/downloader.py` |
| No ETA estimation or cancellation tokens | `core/task_queue.py` |
| `assets/icons/` + `assets/themes/` don't exist | Root |
| Packaging lacks Linux AppImage target | `scripts/package.py` |
| Search results in Downloader tab are plain text (not interactive) | `ui/tabs/tab_settings.py` |
| Speech tab has no file-picker dialog | `ui/tabs/tab_speech.py` |
| `scripts/benchmark.py` and `scripts/cleanup.py` are no-ops | `scripts/` |

---

## Git Status

- Branch: `main` — all 42 files tracked, **no uncommitted changes**
- Up to date with `origin/main`
- `~/.pyforge/` does **not exist** (no models downloaded yet)

---

## How to Run

```bash
pip install -r requirements.txt    # install customtkinter + deps
python main.py                     # downloads default models on first launch
```

---

## Implementation Coverage

| Module | Coverage | Lines |
|--------|----------|-------|
| `core/` | 100% | ~107 |
| `engines/` | ~60% (image/audio/llm/speech functional; video mock) | ~239 |
| `models/` | ~85% (missing resume/retry/checksum) | ~172 |
| `ui/` | ~90% (components exist, history missing persistence) | ~647 |
| `scripts/` | ~30% (benchmark/cleanup stubs; package.py complete) | ~73 |
| `assets/` | 0% | 0 |
