# AGENTS.md — PyForge

## Project Overview

PyForge is a **local, offline, monolithic AI studio** built using **Python 3.10+ / Tkinter / customtkinter**. Provides Text→Image, Image→Image, Upscaling, Text→Video, Text→Audio, Speech→Text, Text→Speech, and LLM inference with model management + HuggingFace search.

---

## Repository Structure

```
pyforge/
    app.py              # App bootstrap, tabs wired, default model download modal
    main.py             # Entry point (11 lines)

    core/
        config.py       # ~/.pyforge dirs, DEFAULT_MODELS dict
        logger.py       # Stream handler logger
        device.py       # CUDA > MPS > CPU
        scheduler.py    # EngineRequest / EngineResponse dataclasses
        task_queue.py   # ThreadPoolExecutor, submit/cancel

    engines/
        engine_image.py # txt2img via diffusers (missing: img2img, inpainting, upscaling)
        engine_video.py # MOCK only (sleep 2s, returns fake path)
        engine_audio.py # Bark TTS via transformers
        engine_llm.py   # transformers text-generation pipeline
        engine_speech.py# Whisper ASR only (missing: TTS)

    models/
        registry.py     # get/set/list default + installed models
        downloader.py   # HF search + snapshot_download
        metadata.py     # ModelInfo dataclass + JSON I/O

    ui/
        theme.py        # Dark mode + blue theme
        main_window.py  # 6-tab responsive window (Image/Video/Audio/Speech/LLM/Downloader)
        tabs/           # One per modality + Downloader settings
        components/     # prompt_box, output_viewer, history_panel, model_selector, progress_bar

    scripts/
        benchmark.py    # STUB
        cleanup.py      # STUB
        package.py      # PyInstaller wrapper (Win .exe, macOS .app)

    assets/             # MISSING — icons/ and themes/ dirs don't exist
```

---

## Coding Rules

- Python 3.10+, no external UI except `tkinter` + `customtkinter`
- No global state except `config`
- All engines expose `def run(request: EngineRequest) -> EngineResponse`
- UI uses `grid()` only, no absolute positioning, reusable classes
- Dark mode via customtkinter theme
- Engines must never block UI thread — run through `task_queue`
- Engines must support cancellation, return `EngineResponse(success, output, metadata, error)`
- Models stored under `~/.pyforge/models/`
- Registry tracks: name, type, size, local_path, hf_id, version, tags
- PEP8, type hints everywhere, docstrings for every class/function
- No unused imports, no wildcard imports, no inline lambdas in UI code

---

## Development Commands

| Command | Description |
|---------|-------------|
| `pip install -r requirements.txt` | Install dependencies |
| `python main.py` | Run application |
| `python scripts/benchmark.py` | Run benchmarks (stub) |
| `python scripts/cleanup.py` | Clean cache (stub) |
| `python scripts/package.py` | Build distributable |

---

## Performance Improvements (Cross-Platform)

### Model Loading & Memory

| Improvement | Location | Impact |
|-------------|----------|--------|
| **Lazy model loading** — load models on first generate, not at tab init | All engines | Reduces startup time from ~30s to ~1s |
| **CPU offload** — `enable_model_cpu_offload()` on CUDA devices | `engine_image.py` | Reduces peak VRAM by 40-60% |
| **VAE tiling** — `enable_vae_tiling()` for high-res generation | `engine_image.py` | Enables >1024px images on 8GB VRAM |
| **VAE slicing** — `enable_vae_slicing()` in addition to attention slicing | `engine_image.py` | Further VRAM reduction |
| **Model cache LRU** — keep last 2 models in memory, evict oldest | `registry.py` | Avoid reloading when switching tabs |

### Engine Acceleration

| Improvement | Location | Impact |
|-------------|----------|--------|
| **`torch.compile`** — `model = torch.compile(model)` on CUDA 8.0+ | All engines | 15-30% inference speedup |
| **SDPA (scaled dot-product attention)** — native in PyTorch 2.0+, works on CUDA/MPS | All diffusers engines | 2x attention speedup vs eager |
| **xformers** — `enable_xformers_memory_efficient_attention()` (CUDA only, fallback to SDPA) | `engine_image.py` | Memory-efficient attention |
| **Flash Attention 2** — pass `attn_implementation="flash_attention_2"` in `from_pretrained` | `engine_image.py` + `engine_llm.py` | 2-3x faster attention on Ampere+ GPUs |
| **Use `llama.cpp`/`llama-cpp-python`** — replace `transformers` pipeline for LLM | `engine_llm.py` | 5-10x faster CPU inference, 4x less RAM |
| **Use `faster-whisper` (CTranslate2)** — replace transformers whisper for ASR | `engine_speech.py` | 4x faster transcription, half the RAM |
| **Use `bark.cpp` / bark with `bettertransformer`** | `engine_audio.py` | 2-3x faster audio generation |

### Task Queue

| Improvement | Location | Impact |
|-------------|----------|--------|
| **Dynamic worker count** — `max_workers = cpu_count()`, cap at 4 | `core/task_queue.py` | Utilizes multi-core without thrashing |
| **Cancellation tokens** — `threading.Event`-based, checked in inference loop | `core/task_queue.py` + all engines | Real cancellation instead of `future.cancel()` |
| **ETA estimation** — track per-task avg time, emit via callback | `core/task_queue.py` | User-facing progress ETA |
| **Priority queue** — short tasks (ASR) before long ones (video) | `core/task_queue.py` | Better UX responsiveness |

### Downloader

| Improvement | Location | Impact |
|-------------|----------|--------|
| **Resume downloads** — `snapshot_download(resume_download=True)` | `models/downloader.py` | Survive network interruption |
| **Parallel downloads** — download default models concurrently | `app.py:check_default_models` | 3x faster first-launch setup |
| **Checksum verification** — validate SHA256 after download | `models/downloader.py` | Data integrity |
| **Real progress** — track per-file byte progress | `models/downloader.py` | Accurate progress bar |

### UI Responsiveness

| Improvement | Location | Impact |
|-------------|----------|--------|
| **Image downscaling for preview** — resize to 512x512 before CTkImage | `ui/components/output_viewer.py` | Smooth gallery scrolling |
| **Deferred model list refresh** — debounce registry scans | `ui/components/model_selector.py` | No UI freeze on model switch |
| **Async search with abort** — cancel in-flight HF search on new query | `ui/tabs/tab_settings.py` | No stale results |
| **File picker dialog** — use `filedialog.askopenfilename` instead of manual path entry | `ui/tabs/tab_speech.py` | Better UX, fewer errors |

### Device-Aware Dispatch

| Improvement | Location | Impact |
|-------------|----------|--------|
| **VRAM detection** — `torch.cuda.get_device_properties(0).total_memory` | `core/device.py` | Auto-select model size / dtype |
| **Dtype by VRAM** — <4GB → `float32`, 4-8GB → `float16`, >8GB → `float16` + compile | `core/device.py` | Best quality within VRAM budget |
| **MPS fallback** — `float32` on MPS when certain ops unsupported (e.g., `layer_norm` in bf16) | `core/device.py` | Stability on Apple Silicon |
| **CPU fallback** — `num_threads` tuning via `torch.set_num_threads` | `core/device.py` | Optimized CPU path |

---

## Functional Milestones

### M1 — Core Stable & Runnable

- [ ] Install deps (`customtkinter`, `torchaudio`, `accelerate`, `soundfile`)
- [ ] Fix `check_default_models` to not block UI thread during download modal
- [ ] Add real cancellation tokens to task queue
- [ ] History panel saves/loads from `~/.pyforge/history/` as JSON
- [ ] Create `assets/icons/` and `assets/themes/` with placeholder files (so `package.py` doesn't crash)
- [ ] Verify app launches on Windows, macOS, Linux

**Est. effort: 2-3 days**

### M2 — Complete Image Engine

- [ ] Add img2img (input image + prompt → edited image)
- [ ] Add inpainting (image + mask → fill region)
- [ ] Add Real-ESRGAN upscaling (2x/4x)
- [ ] Add UI controls: seed, steps slider, guidance scale, image dimensions, image upload for img2img
- [ ] Show image metadata (resolution, seed, model used) in output viewer

**Est. effort: 3-4 days**

### M3 — Functional Video Engine

- [ ] Integrate real video model (AnimateDiff via diffusers, or Mochi)
- [ ] Memory-efficient pipeline: VAE tiling, CPU offload, frame-by-frame decoding
- [ ] Video output player in UI (frame scrub or play/pause)
- [ ] Video settings: num frames, fps, resolution

**Est. effort: 4-5 days**

### M4 — Engine Acceleration

- [ ] Add `torch.compile` to all engines (guard: CUDA compute 8.0+ only)
- [ ] Add SDPA / Flash Attention 2 to all transformer-based engines
- [ ] Add `enable_model_cpu_offload`, `enable_vae_tiling`, `enable_vae_slicing` to image engine
- [ ] Replace transformers LLM with `llama-cpp-python` (GGUF) — detect GGUF files in model dir
- [ ] Replace transformers whisper with `faster-whisper` (CTranslate2)
- [ ] Dynamic worker pool in task queue (based on CPU count)
- [ ] VRAM-aware dtype selection per device

**Est. effort: 5-7 days**

### M5 — Full Speech Pipeline (ASR + TTS)

- [ ] Add Text→Speech engine (TTS) — separate modality or merge into Speech tab
- [ ] File picker dialog (`filedialog.askopenfilename`) for input audio
- [ ] Audio waveform display in output viewer
- [ ] Playback button for generated audio
- [ ] Record microphone input (optional, `pyaudio` or `sounddevice`)

**Est. effort: 3-4 days**

### M6 — Model Management Complete

- [ ] Interactive search results (clickable items → auto-fill download)
- [ ] Resume/cancel/retry for downloads
- [ ] Checksum verification post-download
- [ ] Model deletion (right-click → delete)
- [ ] Model details panel (size, downloads, tags, last updated)
- [ ] "Set as Default" toggle in model selector

**Est. effort: 3-4 days**

### M7 — History Gallery & Persistence

- [ ] Disk-backed history with full metadata (prompt, seed, model, settings, timestamp, preview path)
- [ ] History gallery tab with thumbnail grid
- [ ] Click thumbnail → restore prompt/settings, re-run
- [ ] History search/filter by modality, model, date
- [ ] History export (JSON / CSV)

**Est. effort: 3-4 days**

### M8 — Packaging & Polish

- [ ] Windows .exe with icon (generate placeholder .ico)
- [ ] macOS .app with icon (generate placeholder .icns)
- [ ] Linux AppImage via `appimagetool` (or `briefcase`)
- [ ] First-run experience: download modal with per-model progress, ETA
- [ ] Graceful error handling for missing dependencies (e.g., no GPU, no torch)
- [ ] Benchmark script: per-engine latency + memory measurement
- [ ] Cleanup script: remove stale models, cache, temp files

**Est. effort: 3-5 days**

---

## Default Models

| Modality | Default Model | Size |
|----------|---------------|------|
| Image | `runwayml/stable-diffusion-v1-5` | ~2 GB |
| Video | `genmo/mochi-1-preview` | ~8 GB |
| Audio | `suno/bark` | ~2 GB |
| Speech | `openai/whisper-small` | ~500 MB |

Stored under `~/.pyforge/models/default/<modality>/<model-name>/`.

---

## Key Architecture Decisions

- **Engines are singletons** — `engine_image = ImageEngine()` at module level. Avoids re-init but complicates cleanup. Consider making them methods on a shared `EngineManager`.
- **`app.after()` used for thread→UI communication** — standard tkinter pattern. All UI updates from engine callbacks must use `master.after(0, ...)`.
- **`config.py` has side effects** — creates `~/.pyforge/` dirs at import time. Move to explicit `init()` if this causes issues.
- **No database** — pure filesystem + JSON for history and model metadata. Sufficient for single-user, no-SQL complexity.

---

## Common Pitfalls

- `future.cancel()` only works if the task hasn't started running — need `threading.Event` for real cancellation
- `snapshot_download` blocks the calling thread — always run in task queue
- Model loading is blocking and slow — always show progress indicator
- `customtkinter` `CTkImage` requires PIL `Image` — ensure proper image format conversions
- On macOS MPS, `float16` is not supported for all operations — fall back to `float32` when op fails
