"""PyInstaller wrapper for Windows .exe, macOS .app, and Linux AppImage."""
from __future__ import annotations

import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASSETS_ICONS = ROOT / "assets" / "icons"


def _icon_for(system: str) -> str | None:
    name = {"Windows": "app.ico", "Darwin": "app.icns", "Linux": "app.png"}[system]
    path = ASSETS_ICONS / name
    return str(path) if path.exists() else None


def _pyinstaller_base(name: str) -> list[str]:
    cmd = [sys.executable, "-m", "PyInstaller", "--name", name, "--windowed", "--noconfirm"]
    return cmd


def build_windows() -> None:
    print("Building Windows .exe …")
    cmd = _pyinstaller_base("PyForge") + ["--onefile"]
    icon = _icon_for("Windows")
    if icon:
        cmd += ["--icon", icon]
    cmd.append(str(ROOT / "main.py"))
    subprocess.run(cmd, check=True, cwd=ROOT)


def build_macos() -> None:
    print("Building macOS .app …")
    cmd = _pyinstaller_base("PyForge")
    icon = _icon_for("Darwin")
    if icon:
        cmd += ["--icon", icon]
    cmd.append(str(ROOT / "main.py"))
    subprocess.run(cmd, check=True, cwd=ROOT)


def build_linux() -> None:
    print("Building Linux binary (+ optional AppImage) …")
    cmd = _pyinstaller_base("PyForge") + ["--onefile"]
    icon = _icon_for("Linux")
    if icon:
        cmd += ["--icon", icon]
    cmd.append(str(ROOT / "main.py"))
    subprocess.run(cmd, check=True, cwd=ROOT)

    appimagetool = shutil.which("appimagetool")
    if not appimagetool:
        print("appimagetool not on PATH; skipping AppImage step. Install it to enable.")
        return

    appdir = ROOT / "dist" / "PyForge.AppDir"
    appdir.mkdir(parents=True, exist_ok=True)
    (appdir / "usr" / "bin").mkdir(parents=True, exist_ok=True)
    shutil.copy(ROOT / "dist" / "PyForge", appdir / "usr" / "bin" / "PyForge")
    (appdir / "AppRun").write_text(
        '#!/bin/sh\nHERE="$(dirname "$(readlink -f "$0")")"\nexec "$HERE/usr/bin/PyForge" "$@"\n',
        encoding="utf-8",
    )
    os.chmod(appdir / "AppRun", 0o755)
    (appdir / "PyForge.desktop").write_text(
        "[Desktop Entry]\nType=Application\nName=PyForge\nExec=PyForge\nIcon=app\nCategories=Utility;\n",
        encoding="utf-8",
    )
    if icon:
        shutil.copy(icon, appdir / "app.png")
    subprocess.run([appimagetool, str(appdir)], check=True, cwd=ROOT / "dist")


def main() -> None:
    system = platform.system()
    if system == "Windows":
        build_windows()
    elif system == "Darwin":
        build_macos()
    elif system == "Linux":
        build_linux()
    else:
        print(f"Unsupported system: {system}")
        sys.exit(1)


if __name__ == "__main__":
    main()
