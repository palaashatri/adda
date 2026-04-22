# PyForge: Local AI Creative Studio

PyForge is a sleek, minimalist, cross-platform local AI studio built in Python. It provides a unified interface for various AI tasks, all running locally on your hardware.

## Features

- **Chat**: Local LLM interaction using `llama-cpp-python`.
- **Image**: Text-to-image generation using `diffusers` (Stable Diffusion).
- **Video**: Text-to-video generation using `diffusers`.
- **Audio**: Text-to-music generation using `audiocraft` (MusicGen).
- **TTS**: Text-to-speech synthesis using `XTTS v2`.
- **STT**: Speech-to-text transcription using `Whisper`.
- **Downloads**: Integrated HuggingFace model manager.
- **Unified API**: OpenAI-compatible local API server for all engines.
- **Settings**: Custom themes, device selection (CUDA/MPS/CPU), and cache management.

## Installation

1. Clone the repository.
2. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```
3. Run the application:
   ```bash
   python main.py
   ```

## Architecture

PyForge is built with a modular architecture:
- `pyforge/ui`: Tkinter/ttkbootstrap based user interface.
- `pyforge/engines`: Backend wrappers for AI models.
- `pyforge/core`: Core logic for settings, HF integration, and the API server.

## License

MIT
