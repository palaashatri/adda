"""Model registry: track default + installed, set active, delete."""
from __future__ import annotations

import shutil
from collections import OrderedDict
from pathlib import Path
from typing import List, Optional

from core.config import DEFAULT_MODELS, DEFAULT_MODELS_DIR, MODELS_DIR
from core.logger import logger
from models.metadata import ModelInfo


class ModelRegistry:
    """In-memory active-model selection over the filesystem-backed model tree."""

    def __init__(self) -> None:
        self.active_models: dict[str, ModelInfo] = {}
        # LRU cache for loaded engine pipelines, keyed by local_path.
        self._loaded_pipelines: "OrderedDict[str, object]" = OrderedDict()
        self._max_cached_pipelines = 2

    # --- discovery ---------------------------------------------------------

    def get_default_model(self, modality: str) -> Optional[ModelInfo]:
        """Return default model metadata if downloaded, else None."""
        hf_id = DEFAULT_MODELS.get(modality)
        if not hf_id:
            return None
        model_name = hf_id.split("/")[-1]
        meta_path = DEFAULT_MODELS_DIR / modality / model_name / "model.json"
        if meta_path.exists():
            info = ModelInfo.load(meta_path)
            info.is_default = True
            return info
        return None

    def get_installed_models(self, modality: str) -> List[ModelInfo]:
        """Return every installed model for a modality (defaults first)."""
        models: list[ModelInfo] = []
        default = self.get_default_model(modality)
        if default:
            models.append(default)

        modality_dir = MODELS_DIR / modality
        if modality_dir.exists():
            for model_dir in sorted(modality_dir.iterdir()):
                if not model_dir.is_dir():
                    continue
                meta_path = model_dir / "model.json"
                if not meta_path.exists():
                    continue
                try:
                    info = ModelInfo.load(meta_path)
                    if not any(m.local_path == info.local_path for m in models):
                        models.append(info)
                except Exception as exc:
                    logger.error(f"Failed to load metadata for {model_dir.name}: {exc}")
        return models

    # --- active selection --------------------------------------------------

    def set_active_model(self, modality: str, model: ModelInfo) -> None:
        self.active_models[modality] = model
        logger.info(f"Set active {modality} model to {model.name}")

    def get_active_model(self, modality: str) -> Optional[ModelInfo]:
        return self.active_models.get(modality) or self.get_default_model(modality)

    # --- mutation ----------------------------------------------------------

    def delete_model(self, model: ModelInfo) -> bool:
        """Remove a model directory from disk. Refuses to delete defaults."""
        if model.is_default:
            logger.warning(f"Refusing to delete default model {model.name}")
            return False
        path = Path(model.local_path)
        if not path.exists():
            logger.warning(f"Model path {path} does not exist; nothing to delete.")
            return False
        try:
            shutil.rmtree(path)
            if self.active_models.get(model.type) and (
                self.active_models[model.type].local_path == model.local_path
            ):
                self.active_models.pop(model.type, None)
            logger.info(f"Deleted model {model.name} at {path}")
            return True
        except Exception as exc:
            logger.error(f"Failed to delete {path}: {exc}")
            return False

    # --- pipeline LRU cache ------------------------------------------------

    def cache_pipeline(self, local_path: str, pipeline: object) -> None:
        """Insert a loaded pipeline, evicting the least-recently-used over cap."""
        if local_path in self._loaded_pipelines:
            self._loaded_pipelines.move_to_end(local_path)
            return
        self._loaded_pipelines[local_path] = pipeline
        while len(self._loaded_pipelines) > self._max_cached_pipelines:
            evict_path, _ = self._loaded_pipelines.popitem(last=False)
            logger.info(f"Evicted cached pipeline {evict_path}")

    def get_cached_pipeline(self, local_path: str) -> Optional[object]:
        pipe = self._loaded_pipelines.get(local_path)
        if pipe is not None:
            self._loaded_pipelines.move_to_end(local_path)
        return pipe


registry = ModelRegistry()
