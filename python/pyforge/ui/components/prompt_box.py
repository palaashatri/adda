"""Prompt entry box with a Generate button."""
from __future__ import annotations

from typing import Callable

import customtkinter as ctk


class PromptBox(ctk.CTkFrame):
    """Textbox + 'Generate' button. Calls `on_submit(prompt)` when submitted."""

    def __init__(self, master, on_submit: Callable[[str], None], **kwargs) -> None:
        super().__init__(master, **kwargs)
        self.on_submit = on_submit

        self.grid_columnconfigure(0, weight=1)

        self.textbox = ctk.CTkTextbox(self, height=80)
        self.textbox.grid(row=0, column=0, padx=5, pady=5, sticky="ew")

        self.submit_btn = ctk.CTkButton(self, text="Generate", command=self._submit)
        self.submit_btn.grid(row=0, column=1, padx=5, pady=5, sticky="e")

    def _submit(self) -> None:
        prompt = self.textbox.get("1.0", "end-1c")
        if prompt.strip():
            self.on_submit(prompt)

    def set_prompt(self, text: str) -> None:
        self.textbox.delete("1.0", "end")
        self.textbox.insert("1.0", text)
