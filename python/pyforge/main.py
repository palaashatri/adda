"""PyForge entry point."""
from __future__ import annotations

from app import PyForgeApp
from core.logger import logger


def main() -> None:
    """Launch the PyForge app."""
    logger.info("Starting PyForge…")
    PyForgeApp().run()


if __name__ == "__main__":
    main()
