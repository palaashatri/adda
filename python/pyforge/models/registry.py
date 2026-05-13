import os
import shutil
from pathlib import Path
from typing import List, Optional
from core.config import MODELS_DIR, DEFAULT_MODELS_DIR, DEFAULT_MODELS
from models.metadata import ModelInfo
from core.logger import logger

class ModelRegistry:
    def __init__(self):
        self.active_models = {}

    def get_default_model(self, modality: str) -> Optional[ModelInfo]:
        """Gets default model metadata if downloaded."""
        hf_id = DEFAULT_MODELS.get(modality)
        if not hf_id:
            return None
            
        model_name = hf_id.split("/")[-1]
        path = DEFAULT_MODELS_DIR / modality / model_name / "model.json"
        
        if path.exists():
            return ModelInfo.load(path)
        return None

    def get_installed_models(self, modality: str) -> List[ModelInfo]:
        """Gets all installed models for a modality."""
        models = []
        modality_dir = MODELS_DIR / modality
        
        if not modality_dir.exists():
            return models

        for model_dir in modality_dir.iterdir():
            if model_dir.is_dir():
                meta_path = model_dir / "model.json"
                if meta_path.exists():
                    try:
                        models.append(ModelInfo.load(meta_path))
                    except Exception as e:
                        logger.error(f"Failed to load metadata for {model_dir.name}: {e}")
        
        # Add default model if installed and not in the main list
        default_model = self.get_default_model(modality)
        if default_model:
            models.append(default_model)
            
        return models

    def set_active_model(self, modality: str, model: ModelInfo):
        self.active_models[modality] = model
        logger.info(f"Set active {modality} model to {model.name}")

    def get_active_model(self, modality: str) -> Optional[ModelInfo]:
        return self.active_models.get(modality) or self.get_default_model(modality)

registry = ModelRegistry()
