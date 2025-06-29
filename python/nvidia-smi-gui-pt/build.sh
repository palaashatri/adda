#!/bin/bash
# Build script for nvidia-smi-gui-pt (Linux)
# Requires: python3, pip, pyinstaller

set -e

APP_NAME="nvidia-smi-gui-pt"
PY_FILE="App.py"
DIST_DIR="dist"

# Check for python3
if ! command -v python3 &>/dev/null; then
  echo "[ERROR] python3 not found. Please install Python 3.6+." >&2
  exit 1
fi

# Check for pip
if ! command -v pip3 &>/dev/null; then
  echo "[ERROR] pip3 not found. Please install pip for Python 3." >&2
  exit 1
fi

# Install PyInstaller if not present
if ! python3 -m pyinstaller --version &>/dev/null; then
  echo "[INFO] Installing PyInstaller..."
  pip3 install --user pyinstaller
fi

# Clean previous builds
rm -rf build $DIST_DIR __pycache__ *.spec

# Build executable
python3 -m pyinstaller --onefile --name "$APP_NAME" "$PY_FILE"

# Move executable to project root for convenience
if [ -f "$DIST_DIR/$APP_NAME" ]; then
  mv "$DIST_DIR/$APP_NAME" .
  echo "[SUCCESS] Built ./$APP_NAME (Linux executable)"
else
  echo "[ERROR] Build failed. Check PyInstaller output above." >&2
  exit 1
fi
