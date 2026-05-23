"""History panel: in-tab strip backed by disk-persisted JSON entries."""
from __future__ import annotations

import json
import uuid
from datetime import datetime
from pathlib import Path
from typing import Callable, Optional

import customtkinter as ctk

from core.config import HISTORY_DIR
from core.logger import logger


def _entry_path(entry_id: str) -> Path:
    return HISTORY_DIR / f"{entry_id}.json"


def save_entry(
    modality: str,
    prompt: str,
    model_name: str,
    settings: dict,
    output_path: str = "",
    output_text: str = "",
    seed: Optional[int] = None,
    preview_path: str = "",
) -> dict:
    """Write a history entry to disk and return the serialized dict."""
    HISTORY_DIR.mkdir(parents=True, exist_ok=True)
    entry = {
        "id": uuid.uuid4().hex,
        "modality": modality,
        "prompt": prompt,
        "model": model_name,
        "settings": settings,
        "seed": seed,
        "output_path": output_path,
        "output_text": output_text,
        "preview_path": preview_path,
        "timestamp": datetime.now().isoformat(),
    }
    try:
        with open(_entry_path(entry["id"]), "w", encoding="utf-8") as f:
            json.dump(entry, f, indent=2)
    except Exception as exc:
        logger.error(f"Failed to write history entry: {exc}")
    return entry


def load_entries(modality: Optional[str] = None) -> list[dict]:
    """Return every persisted entry, newest first, optionally filtered by modality."""
    if not HISTORY_DIR.exists():
        return []
    entries: list[dict] = []
    for f in HISTORY_DIR.glob("*.json"):
        try:
            with open(f, "r", encoding="utf-8") as fh:
                e = json.load(fh)
            if modality is None or e.get("modality") == modality:
                entries.append(e)
        except Exception as exc:
            logger.warning(f"Skipping unreadable history entry {f.name}: {exc}")
    entries.sort(key=lambda e: e.get("timestamp", ""), reverse=True)
    return entries


def delete_entry(entry_id: str) -> bool:
    p = _entry_path(entry_id)
    if not p.exists():
        return False
    try:
        p.unlink()
        return True
    except Exception as exc:
        logger.error(f"Failed to delete history entry {entry_id}: {exc}")
        return False


class HistoryPanel(ctk.CTkScrollableFrame):
    """Per-tab scrollable list of recent runs (disk-backed)."""

    def __init__(
        self,
        master,
        modality: str,
        on_select: Optional[Callable[[dict], None]] = None,
        **kwargs,
    ) -> None:
        super().__init__(master, **kwargs)
        self.modality = modality
        self.on_select = on_select
        self.items: list[ctk.CTkButton] = []
        self.refresh()

    def add_history(self, entry: dict) -> None:
        """Add a single entry to the top of the panel."""
        summary = f"{entry.get('prompt', '')[:32]}…"
        btn = ctk.CTkButton(
            self,
            text=summary,
            anchor="w",
            fg_color="gray30",
            command=lambda e=entry: self._select(e),
        )
        btn.grid(row=0, column=0, sticky="ew", padx=5, pady=2)
        for i, existing in enumerate(self.items, start=1):
            existing.grid_configure(row=i)
        self.items.insert(0, btn)

    def refresh(self) -> None:
        for w in self.items:
            w.destroy()
        self.items.clear()
        for entry in load_entries(self.modality)[:50]:
            self.add_history(entry)

    def _select(self, entry: dict) -> None:
        if self.on_select:
            self.on_select(entry)
