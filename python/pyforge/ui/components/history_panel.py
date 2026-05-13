import customtkinter as ctk

class HistoryPanel(ctk.CTkScrollableFrame):
    def __init__(self, master, **kwargs):
        super().__init__(master, **kwargs)
        self.items = []

    def add_history(self, summary: str):
        lbl = ctk.CTkLabel(self, text=summary, anchor="w", justify="left", fg_color="gray30", corner_radius=5)
        lbl.grid(row=len(self.items), column=0, sticky="ew", padx=5, pady=2)
        self.items.append(lbl)
