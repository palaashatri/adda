# PyForge — Project Status

> Generated: 2026-05-17 | Python 3.10+ | Tkinter + customtkinter

## Headline

All M1–M8 milestones are **code-complete or deliberately deferred** (see AGENTS.md §16 for the per-item legend). What this status doc cannot guarantee is **runtime verification**: this session had no Python runtime, no GPU, no installed model weights, and no way to launch the GUI on three OSes. Treat every `✅` below as "the code is written and the imports are wired" — landing a user test pass is the next milestone.

---

## File Inventory

| Status | File | Notes |
|--------|------|-------|
| ✅ | `main.py` | 11 lines, calls `PyForgeApp().run()` |
| ✅ | `app.py` | Theme + window bootstrap; no startup modal |
| ✅ | `requirements.txt` | Adds `faster-whisper`, `llama-cpp-python`, `optimum`, `psutil` as optional accelerators |
| ✅ | `README.md` | Rewritten to describe non-blocking startup + optional backends |
| ✅ | `AGENTS.md` | Canonical project doc (INSTRUCTIONS.md merged in and deleted) |
| ✅ | `LICENSE` | unchanged (MIT) |
| ✅ | `core/config.py` | Unchanged; still creates `~/.pyforge/` on import |
| ✅ | `core/logger.py` | Unchanged |
| ✅ | `core/device.py` | New: VRAM detection, `get_torch_dtype`, `should_compile`, CPU thread tuning |
| ✅ | `core/scheduler.py` | New: `CancellationToken`, `CancelledError`, dataclass-style `EngineRequest`/`EngineResponse` |
| ✅ | `core/task_queue.py` | Dynamic workers, cancellation tokens, ETA EWMA |
| ✅ | `engines/engine_image.py` | txt2img + img2img + inpaint + upscale; CPU offload, VAE tiling/slicing, xformers, `torch.compile` guard |
| ✅ | `engines/engine_video.py` | AnimateDiff (MotionAdapter or full-pipeline checkpoint); returns clean error if no model |
| ✅ | `engines/engine_audio.py` | Bark + BetterTransformer attempt; cancellation-aware |
| ✅ | `engines/engine_llm.py` | llama-cpp-python if GGUF in dir, else transformers; FA2 attempt |
| ✅ | `engines/engine_speech.py` | faster-whisper if importable, else transformers Whisper |
| ✅ | `engines/engine_tts.py` | NEW — thin TTS wrapper over the audio engine |
| ✅ | `models/registry.py` | Adds `delete_model`, LRU pipeline cache, default-model flagging |
| ✅ | `models/downloader.py` | resume + retry + checksum + cancel + catalog cache (6h TTL) |
| ✅ | `models/metadata.py` | Adds `sha256`, `is_default`, `extra` fields |
| ✅ | `ui/theme.py` | Unchanged |
| ✅ | `ui/main_window.py` | 7-tab window; non-blocking first-run banner |
| ✅ | `ui/tabs/tab_image.py` | Full controls: mode, seed (+random), steps, guidance, W/H, image+mask loaders, cancel, history restore |
| ✅ | `ui/tabs/tab_video.py` | Frames/fps/steps controls + cancel |
| ✅ | `ui/tabs/tab_audio.py` | Cancel + history persistence |
| ✅ | `ui/tabs/tab_speech.py` | ASR (file picker) + TTS mode toggle; per-mode auto model-selector swap |
| ✅ | `ui/tabs/tab_llm.py` | Max-tokens + temperature controls + cancel |
| ✅ | `ui/tabs/tab_settings.py` | Auto-catalog prefetch, interactive results, installed list with delete |
| ✅ | `ui/tabs/tab_history.py` | NEW — cross-modality gallery + filter + export (JSON/CSV) |
| ✅ | `ui/components/prompt_box.py` | + `set_prompt()` so history can restore prompts |
| ✅ | `ui/components/output_viewer.py` | Image/text/audio/video paths; OS-native playback button |
| ✅ | `ui/components/history_panel.py` | Disk persistence (`save_entry`, `load_entries`, `delete_entry`); clickable rows |
| ✅ | `ui/components/model_selector.py` | "Default" checkbox toggle (moves files between trees) |
| ✅ | `ui/components/progress_bar.py` | Clamped `update_progress` |
| ✅ | `assets/icons/README.md` | Placeholder so `package.py` doesn't blow up when no icon is present |
| ✅ | `assets/themes/README.md` | Placeholder |
| ✅ | `scripts/benchmark.py` | Per-engine latency + RSS; skips uninstalled engines |
| ✅ | `scripts/cleanup.py` | `--dry-run`, `--keep-history-days N`; orphan dirs + temp files + aged history |
| ✅ | `scripts/package.py` | Win .exe, macOS .app, Linux + AppImage when `appimagetool` on PATH |

---

## Known Gaps

### 🔴 Blocking (verification-only)

| Issue | Notes |
|-------|-------|
| Cross-platform launch not verified | Needs human test on Windows + macOS + Linux |
| Engine inference not run | No installed models / no GPU in this environment |
| No dep install verified | `pip install -r requirements.txt` succeeds in CI? Untested |

### 🟡 Deliberately deferred

| Item | Reason |
|------|--------|
| Real-ESRGAN upscaling | Currently uses `StableDiffusionUpscalePipeline` to avoid pulling a second model family |
| Video frame-scrub player | Tk has no native video widget; we open externally and inline GIFs only |
| Audio waveform display | Would force a matplotlib dep |
| Mic recording | Would force `sounddevice`/`pyaudio` dep |
| Thumbnail grid in history | Would force per-entry PIL loads at tab render; text-row gallery used instead |
| Rich model-details popup | Tags/downloads are inline; a separate panel can land in M6.x |

### 🟢 Nice-to-have

- Catalog cache could surface per-modality file sizes (HF API gives them, we don't parse yet)
- `EngineRequest.cancel_token` is plumbed but the diffusers `callback` -> token check fires every step, which is plenty for image but coarse for very-long-steps configs

---

## Implementation Coverage

| Module | Coverage | Notes |
|--------|----------|-------|
| `core/` | ~100% | Cancellation, dtype-by-VRAM, dynamic workers, ETA all present |
| `engines/` | ~95% | All five modalities have real code paths; video needs an installed checkpoint to run |
| `models/` | ~95% | Catalog cache, resume, retry, checksum, delete, default-toggle |
| `ui/` | ~90% | All tabs wired; history tab is text-grid not thumbnail-grid |
| `scripts/` | ~100% | benchmark + cleanup + package all real |
| `assets/` | ~10% | Directory structure exists; no actual `.ico`/`.icns`/`.png` icons supplied |

---

## Git Status (snapshot)

Branch: `main` (no remote sync attempted in this session). Working tree relative to HEAD:

- **Modified:** AGENTS.md, README.md, app.py, main.py, requirements.txt, all of `core/`, all 5 engines, all 3 `models/` files, every UI component, all 6 original tabs, all 3 scripts
- **Deleted:** INSTRUCTIONS.md (content merged into AGENTS.md)
- **New:** `engines/engine_tts.py`, `ui/tabs/tab_history.py`, `assets/icons/README.md`, `assets/themes/README.md`
- Nothing committed yet; user has not been asked to commit

---

## How to Run

```bash
pip install -r requirements.txt   # installs base + optional accelerators
python main.py                    # app launches immediately; banner points at Downloader if defaults missing
python scripts/benchmark.py       # only benchmarks engines whose model is already installed
python scripts/cleanup.py --dry-run --keep-history-days 30
python scripts/package.py         # produces .exe / .app / AppImage depending on OS
```

---

## Next Session — Suggested Order

1. **Smoke-test launch** on Windows (then macOS and Linux). Fix import-time crashes if any.
2. Download the SD-1.5 default in the Downloader tab; verify the image pipeline end-to-end on the user's GPU.
3. If GGUF model installed → confirm llama-cpp-python path takes over.
4. Decide whether the deferred items (waveform, mic, thumbnail grid, Real-ESRGAN) are worth pursuing or can stay scoped out.
