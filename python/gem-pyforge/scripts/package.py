import subprocess
import platform
import os
import sys
from pathlib import Path

def build_windows():
    print("Building for Windows...")
    subprocess.run([
        sys.executable, "-m", "PyInstaller",
        "--name", "PyForge",
        "--windowed",
        "--onefile",
        "--icon", "../assets/icons/app.ico", # Placeholder
        "../main.py"
    ], check=True)

def build_macos():
    print("Building for macOS...")
    subprocess.run([
        sys.executable, "-m", "PyInstaller",
        "--name", "PyForge",
        "--windowed",
        "--icon", "../assets/icons/app.icns", # Placeholder
        "../main.py"
    ], check=True)
    # PyInstaller automatically creates the .app bundle on macOS when --windowed is used.

def build_linux():
    print("Building for Linux...")
    subprocess.run([
        sys.executable, "-m", "PyInstaller",
        "--name", "PyForge",
        "--windowed",
        "--onefile",
        "../main.py"
    ], check=True)
    print("To create an AppImage, you need to use appimagetool on the generated binary.")
    
if __name__ == "__main__":
    os.chdir(Path(__file__).parent)
    system = platform.system()
    if system == "Windows":
        build_windows()
    elif system == "Darwin":
        build_macos()
    elif system == "Linux":
        build_linux()
    else:
        print(f"Unsupported system: {system}")
