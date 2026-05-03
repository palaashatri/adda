import os
from pathlib import Path

# Paths
HOME_DIR = Path.home()
PYFORGE_DIR = HOME_DIR / ".pyforge"
MODELS_DIR = PYFORGE_DIR / "models"
HISTORY_DIR = PYFORGE_DIR / "history"

DEFAULT_MODELS_DIR = MODELS_DIR / "default"

# Ensure directories exist
for path in [MODELS_DIR, HISTORY_DIR, DEFAULT_MODELS_DIR]:
    path.mkdir(parents=True, exist_ok=True)

for modality in ["image", "video", "audio", "speech", "llm"]:
    (DEFAULT_MODELS_DIR / modality).mkdir(parents=True, exist_ok=True)

DEFAULT_MODELS = {
    "image": "runwayml/stable-diffusion-v1-5",
    "video": "genmo/mochi-1-preview",
    "audio": "suno/bark",
    "speech": "openai/whisper-small" # Using openai/whisper-small as default as it is supported well in transformers
}

APP_TITLE = "PyForge AI Studio"
APP_VERSION = "1.0.0"
