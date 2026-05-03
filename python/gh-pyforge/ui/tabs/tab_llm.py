from __future__ import annotations

import customtkinter as ctk
from core.config import Config


class LLMTab(ctk.CTkFrame):
    """LLM inference tab UI."""

    def __init__(self, master: ctk.CTkTabview, config: Config) -> None:
        super().__init__(master)
        self.config = config
        self.grid_rowconfigure(0, weight=0)
        self.grid_rowconfigure(1, weight=1)
        self.grid_columnconfigure(0, weight=1)
        self._build_tab()

    def _build_tab(self) -> None:
        ctk.CTkLabel(self, text="LLM settings", anchor="w").grid(
            row=0, column=0, sticky="w", padx=12, pady=(12, 8)
        )
        ctk.CTkLabel(
            self,
            text="Configure text generation and prompt engineering controls for large language models.",
            wraplength=520,
            justify="left",
        ).grid(row=1, column=0, sticky="nw", padx=12, pady=(0, 12))
