from __future__ import annotations

import subprocess


def create_executable() -> None:
    """Build a Windows executable with PyInstaller."""

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
    create_executable()
