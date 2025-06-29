# Build script for nvidia-smi-gui-pt (Windows)
# Requires: Python 3, pip, pyinstaller

$ErrorActionPreference = 'Stop'

$APP_NAME = "nvidia-smi-gui-pt.exe"
$PY_FILE = "App.py"
$DIST_DIR = "dist"

# Check for python
if (-not (Get-Command python -ErrorAction SilentlyContinue)) {
    Write-Host "[ERROR] python not found. Please install Python 3.6+." -ForegroundColor Red
    exit 1
}

# Check for pip
if (-not (Get-Command pip -ErrorAction SilentlyContinue)) {
    Write-Host "[ERROR] pip not found. Please install pip for Python 3." -ForegroundColor Red
    exit 1
}

# Install PyInstaller if not present
try {
    python -m pyinstaller --version | Out-Null
} catch {
    Write-Host "[INFO] Installing PyInstaller..." -ForegroundColor Yellow
    pip install --user pyinstaller
}

# Clean previous builds
Remove-Item -Recurse -Force build,$DIST_DIR,*.spec -ErrorAction SilentlyContinue

# Build executable
python -m pyinstaller --onefile --name "nvidia-smi-gui-pt" $PY_FILE

# Move executable to project root for convenience
if (Test-Path "$DIST_DIR/nvidia-smi-gui-pt.exe") {
    Move-Item "$DIST_DIR/nvidia-smi-gui-pt.exe" . -Force
    Write-Host "[SUCCESS] Built .\nvidia-smi-gui-pt.exe (Windows executable)" -ForegroundColor Green
} else {
    Write-Host "[ERROR] Build failed. Check PyInstaller output above." -ForegroundColor Red
    exit 1
}
