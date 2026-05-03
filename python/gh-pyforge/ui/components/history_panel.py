from __future__ import annotations

import customtkinter as ctk
from datetime import datetime
from pathlib import Path
import json
from typing import Any


class HistoryPanel(ctk.CTkFrame):
    """A history sidebar that displays recent activity."""

    def __init__(self, master: ctk.CTk | ctk.CTkFrame) -> None:
        super().__init__(master)
        self.grid_rowconfigure(0, weight=0)
        self.grid_rowconfigure(1, weight=1)
        self.grid_columnconfigure(0, weight=1)
        self._history_path = Path.home() / ".pyforge" / "history" / "history.json"
        self._history_path.parent.mkdir(parents=True, exist_ok=True)
        self._build_panel()
        self.reload_history()

    def _build_panel(self) -> None:
        self.label = ctk.CTkLabel(self, text="History", anchor="w")
        self.label.grid(row=0, column=0, sticky="w", padx=8, pady=(8, 4))

        self.listbox = ctk.CTkTextbox(self, state="disabled")
        self.listbox.grid(row=1, column=0, sticky="nsew", padx=8, pady=(0, 8))

    def reload_history(self) -> None:
        entries = self._load_history()
        display_lines: list[str] = []
        for item in entries[-12:]:
            summary = f"{item.get('timestamp', '')} — {item.get('modality', '')} — {item.get('model', '')}"
            prompt = item.get("prompt", "")
            display_lines.append(f"{summary}\nPrompt: {prompt}")
        display = "\n\n".join(display_lines)
        self.listbox.configure(state="normal")
        self.listbox.delete("0.0", "end")
        self.listbox.insert("0.0", display or "No history yet.")
        self.listbox.configure(state="disabled")

    def _load_history(self) -> list[dict[str, Any]]:
        if not self._history_path.exists():
            return []
        try:
            return json.loads(self._history_path.read_text(encoding="utf-8"))
        except Exception:
            return []

    def add_entry(
        self,
        prompt: str,
        model: str,
        modality: str,
        preview: str | None = None,
        settings: dict[str, Any] | None = None,
    ) -> None:
        entries = self._load_history()
        entries.append(
            {
                "timestamp": datetime.utcnow().isoformat(timespec="seconds") + "Z",
                "prompt": prompt,
                "model": model,
                "modality": modality,
                "preview": preview or "",
                "settings": settings or {},
            }
        )
        self._history_path.write_text(json.dumps(entries, indent=2), encoding="utf-8")
        self.reload_history()
