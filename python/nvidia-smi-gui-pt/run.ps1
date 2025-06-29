# NVIDIA-SMI GUI Monitor Runner Script for PowerShell
# This script runs the NVIDIA-SMI GUI application with proper error handling

param(
    [switch]$Verbose = $false
)

# Set error action preference
$ErrorActionPreference = "Stop"

# Color functions for output
function Write-Status {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Blue
}

function Write-Success {
    param([string]$Message)
    Write-Host "[SUCCESS] $Message" -ForegroundColor Green
}

function Write-Warning {
    param([string]$Message)
    Write-Host "[WARNING] $Message" -ForegroundColor Yellow
}

function Write-Error {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

# Function to check if command exists
function Test-Command {
    param([string]$Command)
    try {
        if (Get-Command $Command -ErrorAction SilentlyContinue) {
            return $true
        }
    }
    catch {
        return $false
    }
    return $false
}

# Function to check Python version
function Test-PythonVersion {
    param([string]$PythonCommand)
    try {
        $versionOutput = & $PythonCommand --version 2>&1
        if ($LASTEXITCODE -ne 0) {
            return $false
        }
        
        $version = ($versionOutput -split ' ')[1]
        $major, $minor = $version -split '\.'
        
        if ([int]$major -eq 3 -and [int]$minor -ge 6) {
            return $true
        }
    }
    catch {
        return $false
    }
    return $false
}

try {
    Write-Status "Starting NVIDIA-SMI GUI Monitor..."

    # Check for Python
    $pythonCmd = $null
    
    if (Test-Command "python") {
        if (Test-PythonVersion "python") {
            $pythonCmd = "python"
            $pythonPath = (Get-Command python).Source
            Write-Success "Found Python 3.6+ at: $pythonPath"
        }
        else {
            Write-Warning "Python found but version is too old (requires 3.6+)"
        }
    }
    
    if (-not $pythonCmd -and (Test-Command "python3")) {
        if (Test-PythonVersion "python3") {
            $pythonCmd = "python3"
            $python3Path = (Get-Command python3).Source
            Write-Success "Found Python 3.6+ at: $python3Path"
        }
        else {
            Write-Warning "Python 3 found but version is too old (requires 3.6+)"
        }
    }
    
    if (-not $pythonCmd -and (Test-Command "py")) {
        if (Test-PythonVersion "py") {
            $pythonCmd = "py"
            $pyPath = (Get-Command py).Source
            Write-Success "Found Python 3.6+ at: $pyPath"
        }
        else {
            Write-Warning "Python launcher found but version is too old (requires 3.6+)"
        }
    }

    if (-not $pythonCmd) {
        Write-Error "Python 3.6+ not found!"
        Write-Error "Please install Python 3.6 or higher from: https://python.org"
        Write-Error "Make sure to check 'Add Python to PATH' during installation"
        exit 1
    }

    # Check for nvidia-smi
    if (Test-Command "nvidia-smi") {
        $nvidiaSmiPath = (Get-Command nvidia-smi).Source
        Write-Success "nvidia-smi found at: $nvidiaSmiPath"
        
        # Test nvidia-smi
        Write-Status "Testing nvidia-smi..."
        try {
            $null = & nvidia-smi 2>&1
            if ($LASTEXITCODE -eq 0) {
                Write-Success "nvidia-smi is working correctly"
            }
            else {
                Write-Warning "nvidia-smi command failed - check your NVIDIA drivers"
            }
        }
        catch {
            Write-Warning "nvidia-smi test failed - check your NVIDIA drivers"
        }
    }
    else {
        Write-Warning "nvidia-smi command not found!"
        Write-Warning "The application will start but may show an error message."
        Write-Warning "To fix this, install NVIDIA drivers with nvidia-smi support from:"
        Write-Warning "https://www.nvidia.com/drivers"
    }

    # Check if App.py exists
    if (-not (Test-Path "App.py")) {
        Write-Error "App.py not found in current directory!"
        Write-Error "Please make sure you're running this script from the project directory."
        Write-Error "Current directory: $(Get-Location)"
        exit 1
    }

    # Check for tkinter
    Write-Status "Checking for tkinter..."
    try {
        $null = & $pythonCmd -c "import tkinter" 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Success "tkinter is available"
        }
        else {
            Write-Error "tkinter not found!"
            Write-Error "tkinter should be included with Python installation."
            Write-Error "If missing, reinstall Python with tkinter support."
            exit 1
        }
    }
    catch {
        Write-Error "Failed to check tkinter availability"
        Write-Error "This may indicate a Python installation issue"
        exit 1
    }

    # Check for display capability (basic check)
    if ($env:SSH_CLIENT -or $env:SSH_TTY) {
        Write-Warning "SSH session detected - GUI may not work without X11 forwarding"
    }

    # Run the application
    Write-Status "Launching NVIDIA-SMI GUI Monitor..."
    Write-Status "Close the GUI window to exit"
    
    if ($Verbose) {
        Write-Status "Running command: $pythonCmd App.py"
    }

    # Run with error handling
    try {
        & $pythonCmd App.py
        
        if ($LASTEXITCODE -eq 0) {
            Write-Success "Application closed successfully"
        }
        else {
            Write-Error "Application exited with error code: $LASTEXITCODE"
            
            # Provide helpful error messages based on common exit codes
            switch ($LASTEXITCODE) {
                1 { Write-Error "General error - check the application output above" }
                2 { Write-Error "Misuse of shell builtin or invalid argument" }
                126 { Write-Error "Command invoked cannot execute" }
                127 { Write-Error "Command not found" }
                130 { Write-Error "Script terminated by Ctrl+C" }
                default { Write-Error "Unknown error occurred" }
            }
            
            exit $LASTEXITCODE
        }
    }
    catch {
        Write-Error "Failed to start the application: $($_.Exception.Message)"
        exit 1
    }
}
catch {
    Write-Error "Script execution failed: $($_.Exception.Message)"
    if ($Verbose) {
        Write-Error "Full error details: $($_.Exception)"
    }
    exit 1
}

# Pause if running from double-click (not from command line)
if ($Host.Name -eq "ConsoleHost" -and [Environment]::UserInteractive) {
    $parentProcess = (Get-WmiObject Win32_Process -Filter "ProcessId=$PID").ParentProcessId
    $parentProcessName = (Get-Process -Id $parentProcess -ErrorAction SilentlyContinue).ProcessName
    
    if ($parentProcessName -eq "explorer") {
        Write-Host "`nPress any key to exit..." -ForegroundColor Cyan
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    }
}
