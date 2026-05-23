"""PyForge application bootstrap."""
from __future__ import annotations

from core.logger import logger
from ui.main_window import MainWindow
from ui.theme import apply_theme


class PyForgeApp:
    """Thin wrapper around the main window + theme bootstrap."""

    def __init__(self) -> None:
        apply_theme()
        self.window = MainWindow()

    def run(self) -> None:
        logger.info("PyForge ready. Use the Downloader tab to fetch models.")
        self.window.mainloop()
