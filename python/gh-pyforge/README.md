# PyForge

PyForge is a local, offline AI studio built on Python and Tkinter with a modular UI and model registry.

## Features

- Text → Image
- Image → Image
- Image Upscaling
- Text → Video
- Text → Audio
- Speech → Text
- Text → Speech
- LLM inference
- Model management and HuggingFace search
- History gallery with metadata

## Structure

The project uses the following package layout:

- `app.py`, `main.py`
- `core/` for configuration, logging, device discovery, scheduling, and task queue
- `ui/` for theme, main window, tabs, and reusable components
- `engines/` for modality-specific execution logic
- `models/` for registry, metadata, and downloader support
- `assets/` for icons and theme assets
- `scripts/` for benchmark, cleanup, and packaging helpers

## Run

Use the helper script to bootstrap the virtual environment and launch PyForge:

```bash
./run.sh
```

## Development

Install dependencies in a virtual environment:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
```

Then launch the application:

```bash
python main.py
```

## Packaging

- `scripts/package_windows.py` builds a Windows `.exe`
- `scripts/package_macos.py` builds a macOS `.app`
- `scripts/package_linux.py` supports Linux AppImage packaging
