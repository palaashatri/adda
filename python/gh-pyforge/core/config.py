from __future__ import annotations

import json
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Any


@dataclass
class Config:
    """Application configuration and persisted runtime state."""

    base_dir: Path = field(default_factory=lambda: Path.home() / ".pyforge")
    models_dir: Path = field(init=False)
    history_dir: Path = field(init=False)
    config_path: Path = field(init=False)
    appearance_mode: str = "dark"
    color_theme: str = "blue"
    default_models: dict[str, str] = field(default_factory=lambda: {
        "image": "runwayml/stable-diffusion-v1-5",
        "video": "genmo/mochi-1-preview",
        "audio": "suno/bark",
        "speech": "openai/whisper-small",
        "llm": "gpt2",
    })
    active_models: dict[str, str] = field(default_factory=lambda: {
        "image": "runwayml/stable-diffusion-v1-5",
        "video": "genmo/mochi-1-preview",
        "audio": "suno/bark",
        "speech": "openai/whisper-small",
        "llm": "gpt2",
    })
    download_on_first_launch: bool = True

    def __post_init__(self) -> None:
        self.models_dir = self.base_dir / "models"
        self.history_dir = self.base_dir / "history"
        self.config_path = self.base_dir / "config.json"
        self.base_dir.mkdir(parents=True, exist_ok=True)
        self.models_dir.mkdir(parents=True, exist_ok=True)
        self.history_dir.mkdir(parents=True, exist_ok=True)

    @classmethod
    def load_default(cls) -> Config:
        config = cls()
        if config.config_path.exists():
            try:
                raw = json.loads(config.config_path.read_text(encoding="utf-8"))
                config = cls(**cls._normalize(raw))
            except Exception:
                pass
        return config

    def save(self) -> None:
        self.base_dir.mkdir(parents=True, exist_ok=True)
        self.config_path.write_text(json.dumps(asdict(self), indent=2), encoding="utf-8")

    @staticmethod
    def _normalize(raw: dict[str, Any]) -> dict[str, Any]:
        normalized = {
            "base_dir": Path(raw.get("base_dir", Path.home() / ".pyforge")),
            "appearance_mode": str(raw.get("appearance_mode", "dark")),
            "color_theme": str(raw.get("color_theme", "blue")),
            "default_models": raw.get("default_models", {}),
            "active_models": raw.get("active_models", {}),
            "download_on_first_launch": bool(raw.get("download_on_first_launch", True)),
        }
        return normalized
