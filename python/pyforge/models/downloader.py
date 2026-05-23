"""HuggingFace search + snapshot_download with resume/retry/checksum/catalog cache."""
from __future__ import annotations

import hashlib
import json
import threading
import time
from datetime import datetime
from pathlib import Path
from typing import Callable, Optional

from huggingface_hub import HfApi, snapshot_download

from core.config import DEFAULT_MODELS_DIR, MODELS_DIR, PYFORGE_DIR
from core.logger import logger
from models.metadata import ModelInfo

CATALOG_CACHE_PATH = PYFORGE_DIR / "hf_catalog.json"
CATALOG_TTL_SECONDS = 6 * 60 * 60  # 6 hours
MAX_RETRIES = 3
MODALITY_TAGS = {
    "image": "text-to-image",
    "video": "text-to-video",
    "audio": "text-to-audio",
    "speech": "automatic-speech-recognition",
    "llm": "text-generation",
}


def _dir_size_bytes(path: Path) -> int:
    total = 0
    for p in path.rglob("*"):
        if p.is_file():
            try:
                total += p.stat().st_size
            except OSError:
                pass
    return total


def _sha256_of_dir(path: Path) -> str:
    """Compose a deterministic SHA256 over (relative-path, content) pairs."""
    h = hashlib.sha256()
    for file in sorted(p for p in path.rglob("*") if p.is_file()):
        rel = str(file.relative_to(path)).replace("\\", "/")
        h.update(rel.encode("utf-8"))
        try:
            with open(file, "rb") as f:
                for chunk in iter(lambda: f.read(1024 * 1024), b""):
                    h.update(chunk)
        except OSError as exc:
            logger.warning(f"Skipping {file} in checksum: {exc}")
    return h.hexdigest()


class ModelDownloader:
    """HF API wrapper with retry, resume, checksum, and a JSON catalog cache."""

    def __init__(self) -> None:
        self.api = HfApi()
        self._cancellations: set[str] = set()
        self._lock = threading.Lock()

    # --- search ------------------------------------------------------------

    def search_huggingface(self, query: str, modality: str, limit: int = 25) -> list[dict]:
        """Live search HF for models filtered by modality."""
        try:
            tags = [MODALITY_TAGS[modality]] if modality in MODALITY_TAGS else []
            models = self.api.list_models(
                search=query or None,
                tags=tags,
                limit=limit,
                sort="downloads",
                direction=-1,
            )
            results = []
            for m in models:
                results.append({
                    "id": m.id,
                    "downloads": getattr(m, "downloads", 0) or 0,
                    "tags": list(getattr(m, "tags", []) or []),
                    "last_modified": str(getattr(m, "last_modified", "") or ""),
                })
            return results
        except Exception as exc:
            logger.error(f"Error searching HuggingFace ({modality}/{query!r}): {exc}")
            return []

    # --- catalog cache -----------------------------------------------------

    def load_catalog_cache(self) -> Optional[dict]:
        """Return cached catalog if present and fresh, else None."""
        if not CATALOG_CACHE_PATH.exists():
            return None
        try:
            with open(CATALOG_CACHE_PATH, "r", encoding="utf-8") as f:
                cache = json.load(f)
            if time.time() - cache.get("generated_at", 0) > CATALOG_TTL_SECONDS:
                return None
            return cache
        except Exception as exc:
            logger.warning(f"Catalog cache unreadable, ignoring: {exc}")
            return None

    def save_catalog_cache(self, data: dict) -> None:
        try:
            CATALOG_CACHE_PATH.parent.mkdir(parents=True, exist_ok=True)
            with open(CATALOG_CACHE_PATH, "w", encoding="utf-8") as f:
                json.dump(data, f, indent=2)
        except Exception as exc:
            logger.warning(f"Failed to write catalog cache: {exc}")

    def refresh_catalog(self, per_modality_limit: int = 15) -> dict:
        """Query HF once per modality and persist the result."""
        catalog = {"generated_at": time.time(), "modalities": {}}
        for modality in MODALITY_TAGS:
            catalog["modalities"][modality] = self.search_huggingface(
                query="", modality=modality, limit=per_modality_limit
            )
        self.save_catalog_cache(catalog)
        return catalog

    # --- download lifecycle ------------------------------------------------

    def cancel(self, hf_id: str) -> None:
        with self._lock:
            self._cancellations.add(hf_id)
        logger.info(f"Cancellation marked for {hf_id}")

    def _was_cancelled(self, hf_id: str) -> bool:
        with self._lock:
            return hf_id in self._cancellations

    def _clear_cancel(self, hf_id: str) -> None:
        with self._lock:
            self._cancellations.discard(hf_id)

    def download_model(
        self,
        hf_id: str,
        modality: str,
        is_default: bool = False,
        progress_cb: Optional[Callable[[float], None]] = None,
        compute_checksum: bool = False,
    ) -> bool:
        """Download an HF repo with retry+resume. Idempotent if model.json exists."""
        self._clear_cancel(hf_id)
        model_name = hf_id.split("/")[-1]
        target_dir = (
            DEFAULT_MODELS_DIR / modality / model_name
            if is_default
            else MODELS_DIR / modality / model_name
        )
        target_dir.parent.mkdir(parents=True, exist_ok=True)

        if (target_dir / "model.json").exists():
            logger.info(f"Model {hf_id} already installed at {target_dir}")
            if progress_cb:
                progress_cb(100.0)
            return True

        last_error: Optional[Exception] = None
        for attempt in range(1, MAX_RETRIES + 1):
            if self._was_cancelled(hf_id):
                logger.info(f"Aborted download of {hf_id} (cancelled)")
                return False
            try:
                logger.info(
                    f"Downloading {hf_id} -> {target_dir} "
                    f"(attempt {attempt}/{MAX_RETRIES})"
                )
                snapshot_download(
                    repo_id=hf_id,
                    local_dir=str(target_dir),
                    local_dir_use_symlinks=False,
                    resume_download=True,
                )
                break
            except Exception as exc:
                last_error = exc
                logger.warning(f"Download attempt {attempt} for {hf_id} failed: {exc}")
                time.sleep(min(2 ** attempt, 8))
        else:
            logger.error(f"Giving up on {hf_id} after {MAX_RETRIES} attempts: {last_error}")
            return False

        try:
            size_bytes = _dir_size_bytes(target_dir)
            sha = _sha256_of_dir(target_dir) if compute_checksum else ""
            meta = ModelInfo(
                name=model_name,
                type=modality,
                hf_id=hf_id,
                version="1.0",
                size=round(size_bytes / (1024 ** 3), 3),
                tags=[modality],
                local_path=str(target_dir),
                download_date=datetime.now().isoformat(),
                sha256=sha,
                is_default=is_default,
            )
            meta.save(target_dir / "model.json")
        except Exception as exc:
            logger.error(f"Failed to persist metadata for {hf_id}: {exc}")
            return False

        if progress_cb:
            progress_cb(100.0)
        return True


downloader = ModelDownloader()
