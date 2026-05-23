# PyForge

A local, offline, monolithic AI studio built with Python + Tkinter (`customtkinter`).

## Modalities

- Text → Image (txt2img, img2img, inpainting, upscaling)
- Text → Video (AnimateDiff / Mochi via `diffusers`)
- Text → Audio (Bark)
- Speech → Text (faster-whisper or transformers Whisper)
- Text → Speech (Bark)
- LLM inference (llama-cpp-python for GGUF, transformers otherwise)
- HuggingFace model search + manager
- Disk-backed history gallery

## Setup

```bash
pip install -r requirements.txt
python main.py
```

`faster-whisper` and `llama-cpp-python` are optional accelerated backends — the engines fall back to `transformers` if they aren't installed.

## First run

The app starts immediately and shows a non-blocking banner listing any missing default models. Open the **Downloader** tab to fetch what you need; downloads run on a background thread and can be cancelled.

## Packaging

```bash
python scripts/package.py
```

The script detects the OS and produces:

- Windows: `.exe` (PyInstaller `--onefile`)
- macOS: `.app` (PyInstaller `--windowed`)
- Linux: a `--onefile` binary, then an AppImage if `appimagetool` is on `PATH`

## Scripts

- `python scripts/benchmark.py` — per-engine latency + RSS (skips engines with no installed model).
- `python scripts/cleanup.py [--dry-run] [--keep-history-days N]` — purge orphan model dirs, temp outputs, and aged history.

## Project rules

See [AGENTS.md](AGENTS.md) for the canonical project conventions, milestones, and architecture decisions.
