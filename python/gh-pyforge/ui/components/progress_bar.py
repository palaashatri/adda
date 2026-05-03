from __future__ import annotations

import customtkinter as ctk


class ProgressPanel(ctk.CTkFrame):
    """A shared progress bar component."""

    def __init__(self, master: ctk.CTk | ctk.CTkFrame) -> None:
        super().__init__(master)
        self.grid_columnconfigure(0, weight=1)
        self._build_progress()

    def _build_progress(self) -> None:
        self.label = ctk.CTkLabel(self, text="Task Progress", anchor="w")
        self.label.grid(row=0, column=0, sticky="w", padx=8, pady=(8, 4))

        self.bar = ctk.CTkProgressBar(self)
        self.bar.grid(row=1, column=0, sticky="ew", padx=8, pady=(0, 8))
        self.bar.set(0.0)

    def set_progress(self, value: float) -> None:
        self.bar.set(max(0.0, min(1.0, value)))
