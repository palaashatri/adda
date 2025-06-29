# Quick Setup Guide

## Windows Setup

1. **Install Python 3.6+** from [python.org](https://python.org)
   - ✅ Check "Add Python to PATH" during installation
   
2. **Install NVIDIA Drivers** from [nvidia.com](https://nvidia.com/drivers)
   - Make sure `nvidia-smi` command works in Command Prompt/PowerShell
   
3. **Run the application:**
   ```powershell
   # Open PowerShell in project directory
   .\run.ps1
   
   # Alternative - if execution policy blocks the script:
   powershell -ExecutionPolicy Bypass -File .\run.ps1
   
   # Manual execution:
   python App.py
   ```

## Linux Setup (Ubuntu/Debian)

1. **Install Python and tkinter:**
   ```bash
   sudo apt update
   sudo apt install python3 python3-tk
   ```

2. **Install NVIDIA Drivers:**
   ```bash
   # Option 1: Using package manager
   sudo apt install nvidia-driver-470  # or latest version
   
   # Option 2: Download from NVIDIA website
   # Follow NVIDIA's installation guide
   ```

3. **Run the application:**
   ```bash
   ./run.sh
   
   # Manual execution:
   python3 App.py
   ```

## Linux Setup (CentOS/RHEL/Fedora)

1. **Install Python and tkinter:**
   ```bash
   # CentOS/RHEL
   sudo yum install python3 tkinter
   
   # Fedora
   sudo dnf install python3 tkinter
   ```

2. **Install NVIDIA Drivers:**
   ```bash
   # Enable RPM Fusion repository first
   sudo dnf install nvidia-driver nvidia-settings
   ```

3. **Run the application:**
   ```bash
   ./run.sh
   ```

## macOS Setup

1. **Install Python 3.6+:**
   ```bash
   # Using Homebrew (recommended)
   brew install python3
   
   # Or download from python.org
   ```

2. **Install NVIDIA Drivers:**
   - Download from [NVIDIA website](https://nvidia.com/drivers)
   - Note: Newer Macs use Apple Silicon and don't support NVIDIA GPUs

3. **Run the application:**
   ```bash
   ./run.sh
   
   # Manual execution:
   python3 App.py
   ```

## Troubleshooting

### Common Issues

**"nvidia-smi not found"**
- Install/reinstall NVIDIA drivers
- Reboot after driver installation
- Check if GPU is NVIDIA (not AMD/Intel)

**"Permission denied" (Linux/macOS)**
```bash
chmod +x run.sh
```

**Python not found (Windows)**
- Reinstall Python with "Add to PATH" checked
- Or add Python to PATH manually

**tkinter import error (Linux)**
```bash
# Ubuntu/Debian
sudo apt install python3-tk

# CentOS/RHEL
sudo yum install tkinter
```

**GUI doesn't appear (SSH)**
```bash
# Use X11 forwarding
ssh -X username@hostname
# or
ssh -Y username@hostname
```

### Testing Your Setup

1. **Test Python:**
   ```bash
   python --version
   # Should show Python 3.6+
   ```

2. **Test NVIDIA drivers:**
   ```bash
   nvidia-smi
   # Should show GPU information
   ```

3. **Test tkinter:**
   ```bash
   python -c "import tkinter; print('tkinter OK')"
   ```

## Quick Start Commands

### Windows PowerShell
```powershell
cd path\to\nvidia-smi-gui-pt
.\run.ps1
```

### Linux/macOS Terminal
```bash
cd /path/to/nvidia-smi-gui-pt
./run.sh
```

### Direct Python Execution
```bash
python App.py
# or
python3 App.py
```
