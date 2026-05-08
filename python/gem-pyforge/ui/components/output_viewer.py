import customtkinter as ctk
from PIL import Image
import os

class OutputViewer(ctk.CTkFrame):
    def __init__(self, master, **kwargs):
        super().__init__(master, **kwargs)
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(0, weight=1)

        self.label = ctk.CTkLabel(self, text="Output Preview", fg_color="gray20", corner_radius=8)
        self.label.grid(row=0, column=0, sticky="nsew", padx=10, pady=10)

    def show_image(self, image: Image.Image):
        # Resize image for display if needed
        ctk_image = ctk.CTkImage(light_image=image, dark_image=image, size=(512, 512))
        self.label.configure(image=ctk_image, text="")

    def show_text(self, text: str):
        self.label.configure(image="", text=text, justify="left", anchor="nw", wraplength=500)

    def clear(self):
        self.label.configure(image="", text="Output Preview")
