from __future__ import annotations

import subprocess
import shutil
from pathlib import Path


def create_appimage() -> None:
    """Build a Linux AppImage from the PyInstaller bundle."""

    subprocess.run(
        ["pyinstaller", "--onefile", "main.py", "--name", "PyForge"],
        check=True,
    )
    appdir = Path("dist") / "PyForge.AppDir"
    if appdir.exists():
        print("AppDir created; package with appimagetool manually.")
    else:
        print("Build completed; use a native AppImage packager to wrap the output.")


if __name__ == "__main__":
    create_appimage()
