from __future__ import annotations

import customtkinter as ctk


class PromptBox(ctk.CTkFrame):
    """A reusable prompt input component."""

    def __init__(self, master: ctk.CTk | ctk.CTkFrame) -> None:
        super().__init__(master)
        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(0, weight=1)
        self._build_prompt()

    def _build_prompt(self) -> None:
        self.label = ctk.CTkLabel(self, text="Prompt", anchor="w")
        self.label.grid(row=0, column=0, sticky="w", padx=8, pady=(8, 4))

        self.textbox = ctk.CTkTextbox(self, height=160)
        self.textbox.grid(row=1, column=0, sticky="nsew", padx=8, pady=(0, 8))

    def get_prompt(self) -> str:
        return self.textbox.get("0.0", "end").strip()
