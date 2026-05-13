import customtkinter as ctk
from models.registry import registry
from typing import Callable

class ModelSelector(ctk.CTkFrame):
    def __init__(self, master, modality: str, on_change: Callable = None, **kwargs):
        super().__init__(master, **kwargs)
        self.modality = modality
        self.on_change = on_change
        
        self.grid_columnconfigure(1, weight=1)

        self.lbl = ctk.CTkLabel(self, text="Model:")
        self.lbl.grid(row=0, column=0, padx=5, pady=5)

        self.combo = ctk.CTkComboBox(self, command=self._on_select)
        self.combo.grid(row=0, column=1, padx=5, pady=5, sticky="ew")

        self.refresh_btn = ctk.CTkButton(self, text="Refresh", width=60, command=self.refresh)
        self.refresh_btn.grid(row=0, column=2, padx=5, pady=5)
        
        self.refresh()

    def refresh(self):
        models = registry.get_installed_models(self.modality)
        if not models:
            self.combo.configure(values=["No models found"])
            self.combo.set("No models found")
            return
            
        names = [m.name for m in models]
        self.combo.configure(values=names)
        
        active = registry.get_active_model(self.modality)
        if active and active.name in names:
            self.combo.set(active.name)
        else:
            self.combo.set(names[0])
            registry.set_active_model(self.modality, models[0])

    def _on_select(self, choice):
        models = registry.get_installed_models(self.modality)
        for m in models:
            if m.name == choice:
                registry.set_active_model(self.modality, m)
                if self.on_change:
                    self.on_change(m)
                break
