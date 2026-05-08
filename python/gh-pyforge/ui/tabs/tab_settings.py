from __future__ import annotations

import customtkinter as ctk
from pathlib import Path
from typing import Any

from core.config import Config
from core.task_queue import CancellationToken
from models.registry import ModelInfo, ModelRegistry


class SettingsTab(ctk.CTkFrame):
    """Settings tab UI with model downloader and appearance controls."""

    def __init__(self, master: ctk.CTkTabview | ctk.CTkFrame, config: Config) -> None:
        super().__init__(master)
        self.config = config
        self.registry = ModelRegistry(config)
        self.current_modality = "image"
        self.search_results: list[ModelInfo] = []
        self.cancel_token: CancellationToken | None = None

        self.grid_columnconfigure(0, weight=1)
        self._build_tab()
        self._refresh_defaults()

    def _build_tab(self) -> None:
        self.label = ctk.CTkLabel(self, text="Settings", anchor="w")
        self.label.grid(row=0, column=0, sticky="w", padx=12, pady=(12, 8))

        self.dark_mode_toggle = ctk.CTkSwitch(
            self,
            text="Dark mode",
            command=self._toggle_dark_mode,
        )
        self.dark_mode_toggle.grid(row=1, column=0, sticky="w", padx=12, pady=(0, 8))
        self.dark_mode_toggle.select() if self.config.appearance_mode == "dark" else self.dark_mode_toggle.deselect()

        self.theme_selector = ctk.CTkOptionMenu(
            self,
            values=["blue", "green", "dark-blue", "blue"],
            command=self._update_color_theme,
        )
        self.theme_selector.grid(row=2, column=0, sticky="w", padx=12, pady=(0, 8))
        self.theme_selector.set(self.config.color_theme)

        ctk.CTkLabel(self, text="Model Downloader", anchor="w").grid(
            row=3, column=0, sticky="w", padx=12, pady=(16, 4)
        )
        self.modality_menu = ctk.CTkOptionMenu(
            self,
            values=["image", "video", "audio", "speech", "llm"],
            command=self._on_modality_changed,
        )
        self.modality_menu.grid(row=4, column=0, sticky="ew", padx=12, pady=(0, 8))
        self.modality_menu.set(self.current_modality)

        self.search_entry = ctk.CTkEntry(self, placeholder_text="Search HuggingFace models")
        self.search_entry.grid(row=5, column=0, sticky="ew", padx=12, pady=(0, 8))

        self.search_button = ctk.CTkButton(self, text="Search Models", command=self._search_models)
        self.search_button.grid(row=6, column=0, sticky="ew", padx=12, pady=(0, 8))

        self.results_box = ctk.CTkTextbox(self, height=160, state="disabled")
        self.results_box.grid(row=7, column=0, sticky="nsew", padx=12, pady=(0, 8))

        self.download_button = ctk.CTkButton(self, text="Download Selected Model", command=self._download_selected)
        self.download_button.grid(row=8, column=0, sticky="ew", padx=12, pady=(0, 8))

        self.progress_bar = ctk.CTkProgressBar(self)
        self.progress_bar.grid(row=9, column=0, sticky="ew", padx=12, pady=(0, 12))
        self.progress_bar.set(0.0)

    def _refresh_defaults(self) -> None:
        self.current_modality = self.modality_menu.get()
        self._render_results(self.registry.search_huggingface("", self.current_modality))

    def _toggle_dark_mode(self) -> None:
        self.config.appearance_mode = "dark" if self.dark_mode_toggle.get() else "light"
        self.config.save()

    def _update_color_theme(self, theme: str) -> None:
        self.config.color_theme = theme
        self.config.save()

    def _on_modality_changed(self, modality: str) -> None:
        self.current_modality = modality
        self._render_results(self.registry.search_huggingface("", modality))

    def _search_models(self) -> None:
        query = self.search_entry.get().strip()
        self._render_results(self.registry.search_huggingface(query, self.current_modality))

    def _download_selected(self) -> None:
        if not self.search_results:
            return
        model = self.search_results[0]
        self.cancel_token = CancellationToken()
        self.progress_bar.set(0.0)

        def progress(value: float) -> None:
            self.progress_bar.set(value)

        success = self.registry.download_model(model, progress_cb=progress, cancel_token=self.cancel_token)
        if success:
            self.progress_bar.set(1.0)
            self._render_results(self.search_results)
        else:
            self.progress_bar.set(0.0)

    def _render_results(self, models: list[ModelInfo]) -> None:
        self.search_results = models
        lines: list[str] = []
        for model in models[:8]:
            lines.append(
                f"{model.name}\n  type: {model.modality}\n  size: {model.size}MB\n  tags: {', '.join(model.tags)}\n  updated: {model.download_date}\n"
            )
        self.results_box.configure(state="normal")
        self.results_box.delete("0.0", "end")
        self.results_box.insert("0.0", "\n\n".join(lines) or "No models found.")
        self.results_box.configure(state="disabled")
