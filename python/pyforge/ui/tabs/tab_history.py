"""History tab: gallery of all persisted runs with filter + export."""
from __future__ import annotations

import csv
import json
from pathlib import Path
from tkinter import filedialog

import customtkinter as ctk

from core.config import HISTORY_DIR
from ui.components.history_panel import delete_entry, load_entries


class TabHistory(ctk.CTkFrame):
    """Cross-modality history browser with filter, export, delete."""

    MODALITIES = ("all", "image", "video", "audio", "speech", "llm")

    def __init__(self, master, **kwargs) -> None:
        super().__init__(master, **kwargs)
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(2, weight=1)

        ctk.CTkLabel(self, text="History", font=ctk.CTkFont(size=20, weight="bold")).grid(
            row=0, column=0, padx=10, pady=(10, 0), sticky="w"
        )

        controls = ctk.CTkFrame(self)
        controls.grid(row=1, column=0, padx=10, pady=5, sticky="ew")
        controls.grid_columnconfigure(2, weight=1)

        ctk.CTkLabel(controls, text="Filter:").grid(row=0, column=0, padx=5, pady=5)
        self.filter_combo = ctk.CTkComboBox(
            controls, values=list(self.MODALITIES), command=lambda _: self._refresh()
        )
        self.filter_combo.set("all")
        self.filter_combo.grid(row=0, column=1, padx=5, pady=5)

        self.search_entry = ctk.CTkEntry(controls, placeholder_text="Search prompts…")
        self.search_entry.grid(row=0, column=2, padx=5, pady=5, sticky="ew")
        self.search_entry.bind("<KeyRelease>", lambda _e: self._refresh())

        ctk.CTkButton(controls, text="Refresh", command=self._refresh).grid(
            row=0, column=3, padx=5, pady=5
        )
        ctk.CTkButton(controls, text="Export JSON", command=lambda: self._export("json")).grid(
            row=0, column=4, padx=5, pady=5
        )
        ctk.CTkButton(controls, text="Export CSV", command=lambda: self._export("csv")).grid(
            row=0, column=5, padx=5, pady=5
        )

        self.list_frame = ctk.CTkScrollableFrame(self)
        self.list_frame.grid(row=2, column=0, padx=10, pady=10, sticky="nsew")
        self.list_frame.grid_columnconfigure(0, weight=1)

        self._refresh()

    def _filtered(self) -> list[dict]:
        modality = self.filter_combo.get()
        entries = load_entries(modality=None if modality == "all" else modality)
        query = self.search_entry.get().strip().lower()
        if query:
            entries = [e for e in entries if query in e.get("prompt", "").lower()]
        return entries

    def _refresh(self) -> None:
        for child in self.list_frame.winfo_children():
            child.destroy()
        for i, entry in enumerate(self._filtered()):
            self._render_row(i, entry)

    def _render_row(self, i: int, entry: dict) -> None:
        row = ctk.CTkFrame(self.list_frame)
        row.grid(row=i, column=0, sticky="ew", padx=2, pady=2)
        row.grid_columnconfigure(0, weight=1)
        head = (
            f"{entry.get('timestamp', '')}  ·  "
            f"{entry.get('modality', '')}/{entry.get('model', '?')}"
        )
        ctk.CTkLabel(row, text=head, anchor="w", text_color="gray70").grid(
            row=0, column=0, sticky="ew", padx=5
        )
        body = entry.get("prompt") or entry.get("output_text") or entry.get("output_path") or ""
        ctk.CTkLabel(row, text=body[:200], anchor="w", justify="left", wraplength=600).grid(
            row=1, column=0, sticky="ew", padx=5, pady=2
        )
        ctk.CTkButton(
            row,
            text="Delete",
            width=70,
            fg_color="firebrick",
            command=lambda eid=entry["id"]: self._delete(eid),
        ).grid(row=0, column=1, rowspan=2, padx=5, pady=2)

    def _delete(self, entry_id: str) -> None:
        if delete_entry(entry_id):
            self._refresh()

    def _export(self, kind: str) -> None:
        entries = self._filtered()
        if not entries:
            return
        default_name = f"pyforge_history.{kind}"
        path = filedialog.asksaveasfilename(
            initialfile=default_name,
            defaultextension=f".{kind}",
            filetypes=[(kind.upper(), f"*.{kind}")],
        )
        if not path:
            return
        try:
            if kind == "json":
                with open(path, "w", encoding="utf-8") as f:
                    json.dump(entries, f, indent=2)
            else:
                fields = ["timestamp", "modality", "model", "prompt", "output_path", "output_text"]
                with open(path, "w", newline="", encoding="utf-8") as f:
                    writer = csv.DictWriter(f, fieldnames=fields, extrasaction="ignore")
                    writer.writeheader()
                    for e in entries:
                        writer.writerow(e)
        except Exception as exc:
            from core.logger import logger as _logger
            _logger.error(f"Export failed: {exc}")
