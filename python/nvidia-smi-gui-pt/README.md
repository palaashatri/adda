# NVIDIA-SMI GUI Monitor

A simple, real-time graphical interface for monitoring NVIDIA GPU status using `nvidia-smi` command output.

## Overview

This application provides a user-friendly GUI that displays NVIDIA GPU information in real-time, updating every 2 seconds. It's built using Python's Tkinter library and executes the `nvidia-smi` command to fetch GPU status information.

## Features

- Real-time GPU monitoring with automatic updates every 2 seconds
- Clean, scrollable text interface displaying full `nvidia-smi` output
- Error handling for missing drivers or command failures
- Cross-platform compatibility (Windows, Linux, macOS)
- Lightweight and minimal dependencies

## Prerequisites

### Hardware Requirements
- NVIDIA GPU with compatible drivers installed
- NVIDIA driver version that supports `nvidia-smi` command

### Software Requirements
- Python 3.6 or higher
- Tkinter (usually included with Python installation)
- NVIDIA drivers with `nvidia-smi` utility

## Installation

1. **Clone or download the project:**
   ```bash
   git clone <repository-url>
   cd nvidia-smi-gui-pt
   ```

2. **Verify Python installation:**
   ```bash
   python --version
   # or
   python3 --version
   ```

3. **Verify NVIDIA drivers:**
   ```bash
   nvidia-smi
   ```
   This command should display your GPU information. If it fails, install appropriate NVIDIA drivers.

## Usage

### Quick Start

#### Windows (PowerShell)
```powershell
.\run.ps1
```

#### Linux/macOS (Bash)
```bash
./run.sh
```

#### Manual Execution
```bash
python App.py
# or
python3 App.py
```

### Application Interface

- The application opens a window titled "NVIDIA-SMI Monitor"
- GPU information is displayed in a scrollable text area
- Information updates automatically every 2 seconds
- Close the window to exit the application

## Configuration

### Update Interval
To change the refresh rate, modify the `update_gui()` function in `App.py`:

```python
root.after(2000, update_gui)  # Change 2000 (2 seconds) to desired milliseconds
```

### Window Size
Adjust the text area dimensions in `App.py`:

```python
text_area = tk.Text(root, height=30, width=100)  # Modify height and width
```

## Troubleshooting

### Common Issues

1. **"nvidia-smi not found" error:**
   - Install NVIDIA drivers
   - Ensure `nvidia-smi` is in your system PATH
   - Verify GPU is NVIDIA brand

2. **Python not found:**
   - Install Python from [python.org](https://python.org)
   - Ensure Python is added to system PATH

3. **Tkinter import error:**
   - On Ubuntu/Debian: `sudo apt-get install python3-tk`
   - On CentOS/RHEL: `sudo yum install tkinter`
   - On macOS: Usually included with Python

4. **Permission denied (Linux/macOS):**
   ```bash
   chmod +x run.sh
   ```

### Error Messages

- **"nvidia-smi not found"**: NVIDIA drivers not installed or not in PATH
- **"Error: [details]"**: General execution error with specific details

## Development

### Project Structure
```
nvidia-smi-gui-pt/
├── App.py          # Main application file
├── README.md       # This documentation
├── run.sh          # Linux/macOS runner script
├── run.ps1         # Windows PowerShell runner script
└── requirements.txt # Python dependencies (minimal)
```

### Code Structure

- `get_nvidia_smi_output()`: Executes nvidia-smi command and handles errors
- `update_gui()`: Updates the GUI with fresh data and schedules next update
- Main execution: Sets up Tkinter window and starts the update loop

### Extending the Application

To add features:
1. Modify `get_nvidia_smi_output()` for different nvidia-smi parameters
2. Enhance `update_gui()` for custom formatting or filtering
3. Add menu bars, buttons, or configuration options to the main window

## System Requirements

### Minimum Requirements
- Python 3.6+
- NVIDIA GPU with driver support
- 50MB RAM
- Minimal CPU usage

### Tested Platforms
- Windows 10/11
- Ubuntu 20.04+
- macOS 10.14+
- Various NVIDIA GPU models (GTX, RTX, Tesla, Quadro series)

## License

This project is open source. Please check the repository for specific license terms.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on your platform
5. Submit a pull request

## Support

For issues and questions:
1. Check the troubleshooting section
2. Verify system requirements
3. Create an issue in the repository

## Changelog

### Version 1.0
- Initial release
- Real-time nvidia-smi monitoring
- Cross-platform support
- Error handling for missing drivers
