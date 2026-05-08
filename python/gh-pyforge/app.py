from __future__ import annotations

import logging
import sys

from core.config import Config
from core.logger import setup_logger
from ui.main_window import MainWindow
from ui.theme import initialize_theme


def run() -> int:
    """Initialize the application and start the UI."""

    config = Config.load_default()
    logger = setup_logger(config)
    logger.info("Starting PyForge application")

    initialize_theme(config)
    app = MainWindow(config)
    app.geometry("1320x860")

    try:
        app.mainloop()
        return 0
    except Exception as error:
        logger.exception("Unhandled exception in main loop")
        print(f"PyForge failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(run())
