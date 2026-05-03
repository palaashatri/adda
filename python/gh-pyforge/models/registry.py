from __future__ import annotations

import json
from pathlib import Path
from typing import Callable, Iterable

from core.config import Config
from core.task_queue import CancellationToken
from .downloader import download_model, search_huggingface
from .metadata import ModelInfo


ProgressCallback = Callable[[float], None]


class ModelRegistry:
    """A model registry that manages default models and installed models."""

    def __init__(self, config: Config) -> None:
        self.config = config
        self.base_dir = config.models_dir

    def get_default_model(self, modality: str) -> ModelInfo:
        default_name = self.config.default_models.get(modality, "")
        return ModelInfo(
            name=default_name,
            modality=modality,
            size=0,
            local_path=self.base_dir / "default" / modality / default_name,
            hf_id=default_name,
            version="1.0.0",
            tags=[modality],
            download_date="",
        )

    def get_installed_models(self, modality: str) -> list[ModelInfo]:
        root = self.base_dir / modality
        if not root.exists():
            return []
        models: list[ModelInfo] = []
        for model_dir in root.iterdir():
            metadata_file = model_dir / "model.json"
            if not metadata_file.exists():
                continue
            try:
                raw = json.loads(metadata_file.read_text(encoding="utf-8"))
                models.append(ModelInfo.from_dict(raw))
            except Exception:
                continue
        return models

    def search_huggingface(self, query: str, modality: str) -> list[ModelInfo]:
        return search_huggingface(query, modality)

    def download_model(
        self,
        model: ModelInfo,
        progress_cb: ProgressCallback | None = None,
        cancel_token: CancellationToken | None = None,
    ) -> bool:
        target = self.base_dir / modality_path(model.modality) / model.name
        return download_model(model, target, progress_callback=progress_cb, cancel_token=cancel_token)

    def set_active_model(self, modality: str, model: ModelInfo) -> None:
        self.config.active_models[modality] = model.name
        self.config.save()

    def get_active_model(self, modality: str) -> ModelInfo:
        active_name = self.config.active_models.get(modality)
        installed = self.get_installed_models(modality)
        if active_name:
            for candidate in installed:
                if candidate.name == active_name:
                    return candidate
        return self.get_default_model(modality)


def modality_path(modality: str) -> str:
    return modality.lower()
