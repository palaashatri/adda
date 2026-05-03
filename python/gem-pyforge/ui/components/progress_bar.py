import customtkinter as ctk

class ProgressBar(ctk.CTkProgressBar):
    def __init__(self, master, **kwargs):
        super().__init__(master, **kwargs)
        self.set(0)

    def update_progress(self, value: float):
        """Value from 0.0 to 1.0"""
        self.set(value)
