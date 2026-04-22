import ttkbootstrap as ttk
from ttkbootstrap.constants import *

class ThemeEngine:
    def __init__(self, theme_name="darkly"):
        self.theme_name = theme_name
        self.style = ttk.Style(theme=self.theme_name)

    def set_theme(self, theme_name):
        self.theme_name = theme_name
        self.style.theme_use(self.theme_name)

    def toggle_theme(self):
        new_theme = "flatly" if self.theme_name == "darkly" else "darkly"
        self.set_theme(new_theme)
        return new_theme

theme_engine = ThemeEngine()
