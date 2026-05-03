from __future__ import annotations

import customtkinter as ctk


class OutputViewer(ctk.CTkFrame):
    """A reusable output preview component."""

    def __init__(self, master: ctk.CTk | ctk.CTkFrame) -> None:
        super().__init__(master)
        self.grid_rowconfigure(0, weight=0)
        self.grid_rowconfigure(1, weight=1)
        self.grid_columnconfigure(0, weight=1)
        self._build_viewer()

    def _build_viewer(self) -> None:
        self.label = ctk.CTkLabel(self, text="Output Preview", anchor="w")
        self.label.grid(row=0, column=0, sticky="w", padx=8, pady=(8, 4))

        self.output = ctk.CTkTextbox(self, state="disabled")
        self.output.grid(row=1, column=0, sticky="nsew", padx=8, pady=(0, 8))

    def set_output(self, text: str) -> None:
        self.output.configure(state="normal")
        self.output.delete("0.0", "end")
        self.output.insert("0.0", text)
        self.output.configure(state="disabled")
