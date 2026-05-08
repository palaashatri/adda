from __future__ import annotations

import subprocess


def create_app_bundle() -> None:
    """Build a macOS application bundle with PyInstaller."""

    subprocess.run(
        [
            "pyinstaller",
            "--onefile",
            "--windowed",
            "main.py",
            "--name",
            "PyForge",
        ],
        check=True,
    )


if __name__ == "__main__":
    create_app_bundle()
