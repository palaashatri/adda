from __future__ import annotations

import customtkinter as ctk

from core.config import Config
from models.registry import ModelRegistry


class ModelSelector(ctk.CTkFrame):
    """A reusable model selector component that shows active models."""

    def __init__(self, master: ctk.CTk | ctk.CTkFrame, config: Config) -> None:
        super().__init__(master)
        self.config = config
        self.registry = ModelRegistry(config)
        self.current_modality = "image"
        self.grid_columnconfigure(0, weight=1)
        self._build_selector()
        self._refresh_models()

    def _build_selector(self) -> None:
        self.label = ctk.CTkLabel(self, text="Model Selector", anchor="w")
        self.label.grid(row=0, column=0, sticky="w", padx=8, pady=(8, 4))

        self.modality_menu = ctk.CTkOptionMenu(
            self,
            values=["image", "video", "audio", "speech", "llm"],
            command=self._on_modality_changed,
        )
        self.modality_menu.grid(row=1, column=0, sticky="ew", padx=8, pady=(0, 8))
        self.modality_menu.set(self.current_modality)

        self.active_model_label = ctk.CTkLabel(self, text="Active model:")
        self.active_model_label.grid(row=2, column=0, sticky="w", padx=8, pady=(0, 4))

        self.model_name = ctk.CTkLabel(self, text="", anchor="w")
        self.model_name.grid(row=3, column=0, sticky="w", padx=8, pady=(0, 12))

        self.model_menu = ctk.CTkOptionMenu(self, values=[], command=self._on_model_selected)
        self.model_menu.grid(row=4, column=0, sticky="ew", padx=8, pady=(0, 8))

        self.set_default_button = ctk.CTkButton(self, text="Set as Default", command=self._on_set_default)
        self.set_default_button.grid(row=5, column=0, sticky="ew", padx=8, pady=(0, 8))

    def _on_modality_changed(self, modality: str) -> None:
        self.current_modality = modality
        self._refresh_models()

    def _refresh_models(self) -> None:
        active = self.registry.get_active_model(self.current_modality)
        self.model_name.configure(text=active.name)
        installed = self.registry.get_installed_models(self.current_modality)
        values = [model.name for model in installed]
        if not values:
            values = [active.name]
        self.model_menu.configure(values=values)
        self.model_menu.set(values[0])

    def _on_model_selected(self, selected: str) -> None:
        self.model_name.configure(text=selected)

    def _on_set_default(self) -> None:
        selected = self.model_menu.get()
        models = self.registry.get_installed_models(self.current_modality)
        for model in models:
            if model.name == selected:
                self.registry.set_active_model(self.current_modality, model)
                self._refresh_models()
                break
