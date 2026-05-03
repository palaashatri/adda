from __future__ import annotations

import customtkinter as ctk


def initialize_theme(config) -> None:
    """Initialize theming for the Tkinter application."""

    ctk.set_appearance_mode(config.appearance_mode)
    ctk.set_default_color_theme(config.color_theme)
