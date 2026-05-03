import os
import requests
from pathlib import Path
from datetime import datetime
from huggingface_hub import HfApi, snapshot_download
from core.logger import logger
from models.metadata import ModelInfo
from core.config import MODELS_DIR, DEFAULT_MODELS_DIR

class ModelDownloader:
    def __init__(self):
        self.api = HfApi()

    def search_huggingface(self, query: str, modality: str) -> list[dict]:
        """Searches HuggingFace for models."""
        try:
            # Map our modalities to HF pipeline tags where applicable
            tags = []
            if modality == "image":
                tags.append("text-to-image")
            elif modality == "video":
                tags.append("text-to-video")
            elif modality == "audio":
                tags.append("text-to-audio")
            elif modality == "speech":
                tags.append("automatic-speech-recognition")
            elif modality == "llm":
                tags.append("text-generation")

            models = self.api.list_models(search=query, tags=tags, limit=20, sort="downloads", direction=-1)
            
            results = []
            for m in models:
                results.append({
                    "id": m.id,
                    "downloads": getattr(m, "downloads", 0),
                    "tags": getattr(m, "tags", []),
                })
            return results
        except Exception as e:
            logger.error(f"Error searching HuggingFace: {e}")
            return []

    def download_model(self, hf_id: str, modality: str, is_default: bool = False, progress_cb=None) -> bool:
        """Downloads a model from HuggingFace."""
        try:
            model_name = hf_id.split("/")[-1]
            if is_default:
                target_dir = DEFAULT_MODELS_DIR / modality / model_name
            else:
                target_dir = MODELS_DIR / modality / model_name

            if target_dir.exists() and (target_dir / "model.json").exists():
                logger.info(f"Model {hf_id} already exists at {target_dir}")
                if progress_cb:
                    progress_cb(100)
                return True

            logger.info(f"Downloading model {hf_id} to {target_dir}")
            
            # Use snapshot_download for robust downloading
            # Note: snapshot_download doesn't have a simple progress callback per file that works easily in a UI thread without complex setup, 
            # so we'll just log and rely on its internal tqdm or wait for completion. 
            # A real implementation might use a custom transfer agent.
            local_path = snapshot_download(repo_id=hf_id, local_dir=str(target_dir), local_dir_use_symlinks=False)
            
            # Create metadata
            meta = ModelInfo(
                name=model_name,
                type=modality,
                hf_id=hf_id,
                version="1.0",
                size=0.0, # Could calculate dir size here
                tags=[modality],
                local_path=str(local_path),
                download_date=datetime.now().isoformat()
            )
            meta.save(target_dir / "model.json")
            
            if progress_cb:
                progress_cb(100)
                
            return True
        except Exception as e:
            logger.error(f"Error downloading model {hf_id}: {e}")
            return False

downloader = ModelDownloader()
