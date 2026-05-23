"""Customtkinter progress bar wrapper."""
from __future__ import annotations

import customtkinter as ctk


class ProgressBar(ctk.CTkProgressBar):
    """Bar that starts at 0 and exposes an explicit `update_progress(value)`."""

    def __init__(self, master, **kwargs) -> None:
        super().__init__(master, **kwargs)
        self.set(0)

    def update_progress(self, value: float) -> None:
        """`value` is clamped to the [0.0, 1.0] range."""
        self.set(max(0.0, min(1.0, value)))
