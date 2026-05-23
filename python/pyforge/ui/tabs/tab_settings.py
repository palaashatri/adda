"""Downloader tab: HF search + auto catalog + interactive list + delete."""
from __future__ import annotations

import threading
from typing import Optional

import customtkinter as ctk

from core.logger import logger
from models.downloader import MODALITY_TAGS, downloader
from models.registry import registry
from ui.components.progress_bar import ProgressBar


class _ResultRow(ctk.CTkFrame):
    """Single result line: id, downloads, tags, Download button."""

    def __init__(self, master, result: dict, modality: str, on_download, **kwargs) -> None:
        super().__init__(master, **kwargs)
        self.result = result
        self.modality = modality
        self.grid_columnconfigure(0, weight=1)

        title = f"{result['id']}  ·  ↓{result.get('downloads', 0):,}"
        ctk.CTkLabel(self, text=title, anchor="w", justify="left").grid(
            row=0, column=0, sticky="ew", padx=5, pady=2
        )
        tags = ", ".join((result.get("tags") or [])[:4])
        if tags:
            ctk.CTkLabel(self, text=tags, anchor="w", text_color="gray70").grid(
                row=1, column=0, sticky="ew", padx=5
            )
        ctk.CTkButton(
            self, text="Download", width=90, command=lambda: on_download(result, modality)
        ).grid(row=0, column=1, rowspan=2, padx=5, pady=2)


class TabSettings(ctk.CTkFrame):
    """Model Downloader tab — search, auto-catalog, install, delete."""

    def __init__(self, master, **kwargs) -> None:
        super().__init__(master, **kwargs)
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(2, weight=1)
        self.grid_rowconfigure(4, weight=1)

        ctk.CTkLabel(self, text="Model Downloader", font=ctk.CTkFont(size=20, weight="bold")).grid(
            row=0, column=0, padx=10, pady=(10, 0), sticky="w"
        )

        # Search bar -------------------------------------------------------
        search_bar = ctk.CTkFrame(self)
        search_bar.grid(row=1, column=0, padx=10, pady=10, sticky="ew")
        search_bar.grid_columnconfigure(2, weight=1)

        ctk.CTkLabel(search_bar, text="Modality:").grid(row=0, column=0, padx=5, pady=5)
        self.modality_combo = ctk.CTkComboBox(
            search_bar, values=list(MODALITY_TAGS.keys()), command=lambda _: self._render_catalog()
        )
        self.modality_combo.set("image")
        self.modality_combo.grid(row=0, column=1, padx=5, pady=5)

        self.search_entry = ctk.CTkEntry(search_bar, placeholder_text="Search HuggingFace…")
        self.search_entry.grid(row=0, column=2, padx=5, pady=5, sticky="ew")

        ctk.CTkButton(search_bar, text="Search", command=self._search).grid(
            row=0, column=3, padx=5, pady=5
        )
        ctk.CTkButton(search_bar, text="Refresh catalog", command=self._refresh_catalog).grid(
            row=0, column=4, padx=5, pady=5
        )

        self.status_lbl = ctk.CTkLabel(self, text="", anchor="w")
        self.status_lbl.grid(row=2, column=0, padx=10, sticky="ew")

        # Results list -----------------------------------------------------
        self.results_frame = ctk.CTkScrollableFrame(self, label_text="Results")
        self.results_frame.grid(row=3, column=0, padx=10, pady=5, sticky="nsew")
        self.results_frame.grid_columnconfigure(0, weight=1)

        # Installed list ---------------------------------------------------
        self.installed_frame = ctk.CTkScrollableFrame(self, label_text="Installed")
        self.installed_frame.grid(row=4, column=0, padx=10, pady=5, sticky="nsew")
        self.installed_frame.grid_columnconfigure(0, weight=1)

        # Download progress ------------------------------------------------
        dl_frame = ctk.CTkFrame(self)
        dl_frame.grid(row=5, column=0, padx=10, pady=10, sticky="ew")
        dl_frame.grid_columnconfigure(0, weight=1)
        self.dl_label = ctk.CTkLabel(dl_frame, text="Idle.", anchor="w")
        self.dl_label.grid(row=0, column=0, sticky="ew", padx=5)
        self.progress = ProgressBar(dl_frame)
        self.progress.grid(row=1, column=0, columnspan=2, sticky="ew", padx=5, pady=5)
        self.cancel_btn = ctk.CTkButton(
            dl_frame, text="Cancel", fg_color="firebrick", state="disabled", command=self._cancel
        )
        self.cancel_btn.grid(row=0, column=1, padx=5, pady=5, sticky="e")

        self._current_dl_id: Optional[str] = None
        self._catalog: dict = {}
        self.after(100, self._initial_catalog)
        self._refresh_installed()

    # --- catalog -----------------------------------------------------------

    def _initial_catalog(self) -> None:
        """Load cached catalog (or trigger first fetch) without blocking the UI."""
        cached = downloader.load_catalog_cache()
        if cached is not None:
            self._catalog = cached.get("modalities", {})
            self._render_catalog()
            self.status_lbl.configure(text="Catalog loaded from cache.")
        else:
            self._refresh_catalog()

    def _refresh_catalog(self) -> None:
        self.status_lbl.configure(text="Fetching HuggingFace catalog…")

        def worker() -> None:
            catalog = downloader.refresh_catalog()
            self._catalog = catalog.get("modalities", {})
            self.after(0, self._render_catalog)
            self.after(0, lambda: self.status_lbl.configure(text="Catalog refreshed."))

        threading.Thread(target=worker, daemon=True).start()

    def _render_catalog(self) -> None:
        modality = self.modality_combo.get()
        self._render_results(self._catalog.get(modality, []))

    # --- search ------------------------------------------------------------

    def _search(self) -> None:
        modality = self.modality_combo.get()
        query = self.search_entry.get().strip()
        if not query:
            self._render_catalog()
            return
        self.status_lbl.configure(text=f"Searching '{query}' in {modality}…")

        def worker() -> None:
            results = downloader.search_huggingface(query, modality)
            self.after(0, self._render_results, results)
            self.after(0, lambda: self.status_lbl.configure(text=f"{len(results)} results."))

        threading.Thread(target=worker, daemon=True).start()

    def _render_results(self, results: list[dict]) -> None:
        for child in self.results_frame.winfo_children():
            child.destroy()
        if not results:
            ctk.CTkLabel(self.results_frame, text="No results.").grid(row=0, column=0, padx=5, pady=5)
            return
        modality = self.modality_combo.get()
        for i, r in enumerate(results):
            row = _ResultRow(self.results_frame, r, modality, on_download=self._download_clicked)
            row.grid(row=i, column=0, sticky="ew", padx=2, pady=2)

    # --- installed list ---------------------------------------------------

    def _refresh_installed(self) -> None:
        for child in self.installed_frame.winfo_children():
            child.destroy()
        row_idx = 0
        for modality in MODALITY_TAGS:
            for m in registry.get_installed_models(modality):
                tag = "[default] " if m.is_default else ""
                title = f"{tag}{modality}/{m.name}  ·  {m.size} GB"
                ctk.CTkLabel(self.installed_frame, text=title, anchor="w").grid(
                    row=row_idx, column=0, sticky="ew", padx=5, pady=2
                )
                btn = ctk.CTkButton(
                    self.installed_frame,
                    text="Delete",
                    width=80,
                    fg_color="firebrick",
                    state="disabled" if m.is_default else "normal",
                    command=lambda mm=m: self._delete_model(mm),
                )
                btn.grid(row=row_idx, column=1, padx=5, pady=2, sticky="e")
                row_idx += 1
        if row_idx == 0:
            ctk.CTkLabel(self.installed_frame, text="No models installed.").grid(
                row=0, column=0, padx=5, pady=5
            )

    def _delete_model(self, model) -> None:
        if registry.delete_model(model):
            self._refresh_installed()

    # --- download flow ----------------------------------------------------

    def _download_clicked(self, result: dict, modality: str) -> None:
        hf_id = result["id"]
        if self._current_dl_id is not None:
            logger.warning("A download is already running.")
            return
        self._current_dl_id = hf_id
        self.dl_label.configure(text=f"Downloading {hf_id}…")
        self.progress.update_progress(0.05)
        self.cancel_btn.configure(state="normal")

        def progress_cb(pct: float) -> None:
            self.after(0, self.progress.update_progress, pct / 100.0)

        def worker() -> None:
            ok = downloader.download_model(hf_id, modality, progress_cb=progress_cb)
            self.after(0, self._download_done, hf_id, ok)

        threading.Thread(target=worker, daemon=True).start()

    def _cancel(self) -> None:
        if self._current_dl_id:
            downloader.cancel(self._current_dl_id)

    def _download_done(self, hf_id: str, ok: bool) -> None:
        self.dl_label.configure(text=f"{'✅' if ok else '❌'} {hf_id}")
        self.progress.update_progress(1.0 if ok else 0.0)
        self.cancel_btn.configure(state="disabled")
        self._current_dl_id = None
        self._refresh_installed()
