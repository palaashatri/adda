# AGENTS.md — PyForge

> Single source of truth for project rules, structure, milestones, and conventions.
> Supersedes the previous `INSTRUCTIONS.md` (deleted).

---

## 1. Project Overview

PyForge is a **local, offline, monolithic AI studio** built on **Python 3.10+ / Tkinter / customtkinter**.

Modalities:
- Text → Image
- Image → Image
- Image Upscaling
- Text → Video
- Text → Audio
- Speech → Text
- Text → Speech
- LLM inference
- Model management + HuggingFace search
- History gallery with metadata

UI is built with Tkinter + customtkinter, dark mode by default, minimalist layout.

---

## 2. Repository Structure

```text
pyforge/
    app.py              # App bootstrap, tabs wired
    main.py             # Entry point

    core/
        __init__.py
        config.py       # ~/.pyforge dirs, DEFAULT_MODELS dict
        logger.py       # Stream handler logger
        device.py       # CUDA > MPS > CPU
        scheduler.py    # EngineRequest / EngineResponse dataclasses
        task_queue.py   # ThreadPoolExecutor, submit/cancel

    engines/
        __init__.py
        engine_image.py # txt2img + img2img + inpaint + upscale via diffusers
        engine_video.py # AnimateDiff via diffusers (returns clean error if no model)
        engine_audio.py # Bark TTS via transformers (+ BetterTransformer if available)
        engine_llm.py   # llama-cpp-python (GGUF) or transformers fallback
        engine_speech.py# faster-whisper or transformers Whisper fallback
        engine_tts.py   # Text→Speech wrapper over the audio engine

    models/
        __init__.py
        registry.py     # get/set/list default + installed models
        downloader.py   # HF search + snapshot_download
        metadata.py     # ModelInfo dataclass + JSON I/O

    ui/
        __init__.py
        theme.py        # Dark mode + blue theme
        main_window.py  # 7-tab responsive window (Image/Video/Audio/Speech/LLM/Downloader/History)
        tabs/
            __init__.py
            tab_image.py
            tab_video.py
            tab_audio.py
            tab_speech.py     # ASR + TTS modes
            tab_llm.py
            tab_settings.py   # the "Downloader" tab
            tab_history.py    # cross-modality history gallery
        components/
            __init__.py
            prompt_box.py
            output_viewer.py
            history_panel.py
            model_selector.py
            progress_bar.py

    assets/
        icons/          # README placeholder; drop app.ico/app.icns/app.png here
        themes/         # README placeholder for custom customtkinter themes

    scripts/
        benchmark.py    # Per-engine latency + RSS measurement
        cleanup.py      # Orphan model dirs + temp files + aged history
        package.py      # PyInstaller: Win .exe, macOS .app, Linux .AppImage

    requirements.txt
    README.md
    LICENSE
```

---

## 3. Coding Rules (MANDATORY)

### 3.1 General
- Python 3.10+
- No external UI frameworks except `tkinter` and `customtkinter`
- No global state except `config`
- PEP8, type hints everywhere, docstrings for every class/function
- No unused imports, no wildcard imports, no inline lambdas in UI code

### 3.2 UI
- Use `grid()` only, no absolute positioning
- All UI components must be reusable classes
- Dark mode via customtkinter theme
- Window must be resizable and responsive

### 3.3 Engines
- All engines expose:
  ```python
  def run(request: EngineRequest) -> EngineResponse:
      ...
  ```
- Engines must never block the UI thread — run through `task_queue`
- Engines must support cancellation
- Engines must return:
  ```python
  class EngineResponse:
      success: bool
      output: Any
      metadata: dict
      error: Optional[str]
  ```

### 3.4 Model Registry
- All models stored under `~/.pyforge/models/`
- Registry tracks: name, type, size, local_path, hf_id, version, tags

---

## 4. Default Models

PyForge ships with **one default model per modality**:

| Modality | Default Model | Size |
|----------|---------------|------|
| Image    | `runwayml/stable-diffusion-v1-5` | ~2 GB |
| Video    | `genmo/mochi-1-preview` | ~8 GB |
| Audio    | `suno/bark` | ~2 GB |
| Speech   | `openai/whisper-small` (or `faster-whisper-small`) | ~500 MB |

Stored under `~/.pyforge/models/default/<modality>/<model-name>/`.

Default model rules:
- The app does **not** auto-download on launch (see §11 Startup Behavior).
- Users fetch defaults (or any other model) from the Downloader tab.
- Once present, defaults are pre-selected in their modality's selector.
- User can override with any installed model.

---

## 5. Dynamic Model Discovery (HuggingFace Search)

PyForge supports searching HuggingFace for any model.

### Requirements
- Search bar in the Downloader tab
- Query HuggingFace API for: model name, tags, size, downloads, last updated
- Categorize into: image, video, audio, speech, llm

### Download Rules
- Support: resume, retry, checksum, progress bar, cancellation
- Store under `~/.pyforge/models/<modality>/<model-name>/`

### Metadata File
Every downloaded model generates:
```json
{
  "name": "...",
  "type": "...",
  "hf_id": "...",
  "version": "...",
  "size": 0,
  "tags": ["..."],
  "local_path": "...",
  "download_date": "..."
}
```

---

## 6. Model Registry Behavior

Registry exposes:

```python
def get_default_model(modality: str) -> ModelInfo
def get_installed_models(modality: str) -> list[ModelInfo]
def search_huggingface(query: str, modality: str) -> list[ModelInfo]
def download_model(model: ModelInfo, progress_cb) -> bool
def set_active_model(modality: str, model: ModelInfo)
def get_active_model(modality: str) -> ModelInfo
```

Registry must:
- Track default models separately
- Allow switching active model
- Validate model compatibility
- Cache loaded models

---

## 7. UI Specification

### 7.1 Main Window Layout

```
+-------------------------------------------------------------+
|  Model Selector | Prompt Box | Output Preview               |
|                 |            |                              |
+-------------------------------------------------------------+
|  Settings Panel | Generate Button | History Strip           |
+-------------------------------------------------------------+
```

### 7.2 Tabs
Image • Video • Audio • Speech • LLM • Downloader

### 7.3 Model Selector UI
- Dropdown: default + installed models
- Button: "Search Models…"
- Toggle: "Set as Default"

### 7.4 Model Downloader UI
- Search bar
- Category filter
- Results list
- Download button
- Progress bar
- Model details panel

---

## 8. Engine Specifications

### 8.1 Image Engine
- Backend: `diffusers`
- Pipelines: txt2img, img2img, inpainting, upscaling (Real-ESRGAN)

### 8.2 Video Engine
- AnimateDiff, Mochi, Pyramid Flow

### 8.3 Audio Engine
- Bark, AudioCraft, RVC (optional)

### 8.4 LLM Engine
- llama.cpp (GGUF), transformers (HF models)

### 8.5 Speech Engine
- Whisper.cpp, Faster-Whisper

---

## 9. Task Queue

Must include:
- ThreadPoolExecutor
- Queue
- Cancellation tokens (`threading.Event`-based)
- Progress callbacks
- ETA estimation

---

## 10. History System

Each entry must include:
- preview, prompt, seed, model, settings, timestamp

Stored under `~/.pyforge/history/`.

---

## 11. Startup Behavior

On launch:
1. Initialize config (creates `~/.pyforge/` dirs).
2. Apply theme.
3. Build the main window with all tabs.
4. Enter Tkinter mainloop **immediately** — the UI is never blocked.

Models are **not** auto-downloaded at startup. If a modality has no installed model, its model selector displays "No models found" and the engine returns a clean error until the user installs one via the Downloader tab. Downloads always run on a background daemon thread so the UI stays responsive.

*(Historical note: an earlier version of this spec required a blocking first-run modal that downloaded the 4 defaults sequentially. That was removed — see app.py.)*

---

## 12. Packaging Rules

Generate packaging artifacts for:
- Windows `.exe`
- macOS `.app`
- Linux `.AppImage`

---

## 13. Bootstrapping

`main.py` must:
- Initialize config (import side-effect today)
- Initialize theme
- Create the main window
- Load tabs
- Start the Tkinter mainloop

---

## 14. Development Commands

| Command | Description |
|---------|-------------|
| `pip install -r requirements.txt` | Install dependencies |
| `python main.py` | Run application |
| `python scripts/benchmark.py` | Per-engine latency + memory benchmark (skips engines with no model) |
| `python scripts/cleanup.py [--dry-run] [--keep-history-days N]` | Remove orphan model dirs, temp outputs, aged history |
| `python scripts/package.py` | Build distributable |

---

## 15. Performance Improvements (Cross-Platform)

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
| **Parallel downloads** — queue multiple model downloads concurrently | `models/downloader.py` | 3x faster bulk fetch |
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

## 16. Functional Milestones

Legend: `[x]` code-complete, `[~]` partial / different approach, `[ ]` not done. Code-complete ≠ runtime-verified — see `status.md` for the truthful runtime story.

### M1 — Core Stable & Runnable
- [~] Install deps — `requirements.txt` lists the full set (incl. `faster-whisper`, `llama-cpp-python` as optional accelerators); install not verified in this environment.
- [x] Non-blocking startup (no first-run download modal)
- [x] Real cancellation tokens (`core/scheduler.CancellationToken`, plumbed through every engine)
- [x] History panel saves/loads from `~/.pyforge/history/` as JSON
- [x] `assets/icons/` and `assets/themes/` exist (with README placeholders so `package.py` doesn't crash)
- [ ] Verify app launches on Windows, macOS, Linux — needs human runtime testing

### M2 — Complete Image Engine
- [x] img2img (`StableDiffusionImg2ImgPipeline`)
- [x] inpainting (`StableDiffusionInpaintPipeline`)
- [~] Upscaling — uses `StableDiffusionUpscalePipeline` (latent-diffusion upscaler) rather than Real-ESRGAN. Same UX slot, different backend.
- [x] UI controls: mode toggle, seed (+random), steps slider, guidance slider, W/H, image+mask loaders, cancel button
- [x] Image metadata (resolution, seed, mode, model) returned in `EngineResponse.metadata` and stored in history entries

### M3 — Functional Video Engine
- [x] AnimateDiff via `diffusers` (`MotionAdapter` if present, else full-pipeline checkpoint)
- [x] Memory-efficient pipeline: VAE slicing/tiling + CPU offload
- [~] Video player — GIF previewed inline; non-GIF outputs open with the OS default app (no frame scrubber)
- [x] Video settings: frames, fps, steps

### M4 — Engine Acceleration
- [x] `torch.compile` (guarded by CUDA compute ≥ 8.0; opt-out via `PYFORGE_NO_COMPILE`)
- [x] SDPA inherited from PyTorch 2.x; FA2 attempted in `engine_llm.py` with graceful fallback
- [x] `enable_model_cpu_offload`, `enable_vae_tiling`, `enable_vae_slicing` applied to image engine (and `enable_xformers_memory_efficient_attention` when CUDA xformers is available)
- [x] `llama-cpp-python` chosen when a `.gguf` is present in the model dir, else transformers
- [x] `faster-whisper` chosen when importable, else transformers Whisper
- [x] Dynamic worker pool (`cpu_count()` capped at 4)
- [x] VRAM-aware dtype (`core/device.get_torch_dtype`)

### M5 — Full Speech Pipeline (ASR + TTS)
- [x] TTS via a new `engines/engine_tts.py` that delegates to the Bark audio engine
- [x] File picker dialog for ASR input
- [ ] Audio waveform display — intentionally deferred; out-of-scope without `matplotlib`
- [x] Playback button (OS-native via `open_with_default_app`)
- [ ] Mic recording — intentionally deferred (requires `sounddevice`/`pyaudio` dep)

### M6 — Model Management Complete
- [x] Interactive search results (clickable Download buttons per row)
- [x] Auto-catalog prefetch with 6h TTL → `~/.pyforge/hf_catalog.json`
- [x] Resume (`snapshot_download(resume_download=True)`), retry (3 attempts w/ backoff), cancel
- [x] Checksum verification (`compute_checksum=True` opt-in; stored on `ModelInfo.sha256`)
- [x] Model deletion (Delete button in Installed list; refuses defaults)
- [~] Model details panel — title shows id/downloads/tags; richer details popup not yet built
- [x] "Set as Default" toggle in `ModelSelector` (moves files between `default/` and the normal tree)

### M7 — History Gallery & Persistence
- [x] Disk-backed history (`~/.pyforge/history/<uuid>.json`, full metadata incl. preview path)
- [~] Gallery tab — text/metadata rows, not a thumbnail grid (deferred to keep PIL load cheap)
- [x] Click → restore prompt + settings (wired in image tab; other tabs persist entries too)
- [x] Search/filter by modality + prompt substring
- [x] Export to JSON or CSV via a save-as dialog

### M8 — Packaging & Polish
- [x] Windows `.exe` (PyInstaller `--onefile --windowed`)
- [x] macOS `.app` (PyInstaller `--windowed`)
- [x] Linux AppImage when `appimagetool` is on `PATH` (else plain binary)
- [x] First-run guidance UX — non-blocking banner in `main_window` pointing at the Downloader tab
- [x] Graceful error handling — engines never raise to the UI; missing deps surface as `EngineResponse.error`
- [x] Benchmark script — per-engine latency + RSS, skips uninstalled engines
- [x] Cleanup script — `--dry-run`, `--keep-history-days`, orphan model dirs + temp pyforge_* files

---

## 17. Key Architecture Decisions

- **Engines are singletons** — `engine_image = ImageEngine()` at module level. Avoids re-init but complicates cleanup. Consider making them methods on a shared `EngineManager`.
- **`app.after()` used for thread→UI communication** — standard tkinter pattern. All UI updates from engine callbacks must use `master.after(0, ...)`.
- **`config.py` has side effects** — creates `~/.pyforge/` dirs at import time. Move to explicit `init()` if this causes issues.
- **No database** — pure filesystem + JSON for history and model metadata. Sufficient for single-user, no-SQL complexity.
- **Non-blocking startup** — no auto-download modal; users opt in via the Downloader tab. Downloads run on a daemon thread.

---

## 18. Common Pitfalls

- `future.cancel()` only works if the task hasn't started running — need `threading.Event` for real cancellation
- `snapshot_download` blocks the calling thread — always run in task queue
- Model loading is blocking and slow — always show progress indicator
- `customtkinter` `CTkImage` requires PIL `Image` — ensure proper image format conversions
- On macOS MPS, `float16` is not supported for all operations — fall back to `float32` when op fails
