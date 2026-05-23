"""Model selector: dropdown of installed models + 'Set as Default' toggle."""
from __future__ import annotations

from typing import Callable, Optional

import customtkinter as ctk

from core.config import DEFAULT_MODELS_DIR
from core.logger import logger
from models.metadata import ModelInfo
from models.registry import registry


class ModelSelector(ctk.CTkFrame):
    """Dropdown + refresh + 'set as default' for a single modality."""

    def __init__(
        self,
        master,
        modality: str,
        on_change: Optional[Callable[[ModelInfo], None]] = None,
        **kwargs,
    ) -> None:
        super().__init__(master, **kwargs)
        self.modality = modality
        self.on_change = on_change

        self.grid_columnconfigure(1, weight=1)

        ctk.CTkLabel(self, text="Model:").grid(row=0, column=0, padx=5, pady=5)

        self.combo = ctk.CTkComboBox(self, command=self._on_select)
        self.combo.grid(row=0, column=1, padx=5, pady=5, sticky="ew")

        self.default_var = ctk.BooleanVar(value=False)
        self.default_chk = ctk.CTkCheckBox(
            self, text="Default", variable=self.default_var, command=self._toggle_default
        )
        self.default_chk.grid(row=0, column=2, padx=5, pady=5)

        ctk.CTkButton(self, text="Refresh", width=70, command=self.refresh).grid(
            row=0, column=3, padx=5, pady=5
        )

        self.refresh()

    def _models(self) -> list[ModelInfo]:
        return registry.get_installed_models(self.modality)

    def refresh(self) -> None:
        """Reload installed-model list and keep the current selection if still present."""
        models = self._models()
        if not models:
            self.combo.configure(values=["No models found"])
            self.combo.set("No models found")
            self.default_chk.configure(state="disabled")
            self.default_var.set(False)
            return

        names = [m.name for m in models]
        self.combo.configure(values=names)
        self.default_chk.configure(state="normal")

        active = registry.get_active_model(self.modality)
        chosen = active if active and active.name in names else models[0]
        self.combo.set(chosen.name)
        registry.set_active_model(self.modality, chosen)
        self.default_var.set(chosen.is_default)

    def _current_model(self) -> Optional[ModelInfo]:
        name = self.combo.get()
        for m in self._models():
            if m.name == name:
                return m
        return None

    def _on_select(self, _choice: str) -> None:
        model = self._current_model()
        if model is None:
            return
        registry.set_active_model(self.modality, model)
        self.default_var.set(model.is_default)
        if self.on_change:
            self.on_change(model)

    def _toggle_default(self) -> None:
        """Move the chosen model's directory under defaults/ (or back)."""
        model = self._current_model()
        if model is None:
            return
        try:
            import shutil
            src = self._dir_for(model)
            if self.default_var.get():
                target = DEFAULT_MODELS_DIR / model.type / model.name
            else:
                from core.config import MODELS_DIR
                target = MODELS_DIR / model.type / model.name
            if src == target:
                return
            target.parent.mkdir(parents=True, exist_ok=True)
            if target.exists():
                shutil.rmtree(target)
            shutil.move(str(src), str(target))
            model.local_path = str(target)
            model.is_default = self.default_var.get()
            model.save(target / "model.json")
            logger.info(f"Toggled default={model.is_default} for {model.name}")
            self.refresh()
        except Exception as exc:
            logger.error(f"Failed to toggle default for {model.name}: {exc}")

    @staticmethod
    def _dir_for(model: ModelInfo):
        from pathlib import Path
        return Path(model.local_path)
