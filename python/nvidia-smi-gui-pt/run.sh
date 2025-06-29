#!/bin/bash

# NVIDIA-SMI GUI Monitor Runner Script
# This script runs the NVIDIA-SMI GUI application with proper error handling

set -e  # Exit on any error

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check Python version
check_python_version() {
    local python_cmd=$1
    local version=$($python_cmd --version 2>&1 | cut -d' ' -f2)
    local major=$(echo $version | cut -d'.' -f1)
    local minor=$(echo $version | cut -d'.' -f2)
    
    if [ "$major" -eq 3 ] && [ "$minor" -ge 6 ]; then
        return 0
    else
        return 1
    fi
}

print_status "Starting NVIDIA-SMI GUI Monitor..."

# Check for Python
PYTHON_CMD=""
if command_exists python3; then
    if check_python_version python3; then
        PYTHON_CMD="python3"
        print_success "Found Python 3.6+ at: $(which python3)"
    else
        print_warning "Python 3 found but version is too old (requires 3.6+)"
    fi
elif command_exists python; then
    if check_python_version python; then
        PYTHON_CMD="python"
        print_success "Found Python 3.6+ at: $(which python)"
    else
        print_warning "Python found but version is too old (requires 3.6+)"
    fi
fi

if [ -z "$PYTHON_CMD" ]; then
    print_error "Python 3.6+ not found!"
    print_error "Please install Python 3.6 or higher:"
    echo "  Ubuntu/Debian: sudo apt-get install python3"
    echo "  CentOS/RHEL:   sudo yum install python3"
    echo "  macOS:         brew install python3"
    exit 1
fi

# Check for nvidia-smi
if ! command_exists nvidia-smi; then
    print_warning "nvidia-smi command not found!"
    print_warning "The application will start but may show an error message."
    print_warning "To fix this, install NVIDIA drivers with nvidia-smi support."
else
    print_success "nvidia-smi found at: $(which nvidia-smi)"
    
    # Test nvidia-smi
    print_status "Testing nvidia-smi..."
    if nvidia-smi >/dev/null 2>&1; then
        print_success "nvidia-smi is working correctly"
    else
        print_warning "nvidia-smi command failed - check your NVIDIA drivers"
    fi
fi

# Check if App.py exists
if [ ! -f "App.py" ]; then
    print_error "App.py not found in current directory!"
    print_error "Please make sure you're running this script from the project directory."
    exit 1
fi

# Check for tkinter
print_status "Checking for tkinter..."
if $PYTHON_CMD -c "import tkinter" 2>/dev/null; then
    print_success "tkinter is available"
else
    print_error "tkinter not found!"
    print_error "Please install tkinter:"
    echo "  Ubuntu/Debian: sudo apt-get install python3-tk"
    echo "  CentOS/RHEL:   sudo yum install tkinter"
    echo "  macOS:         Usually included with Python"
    exit 1
fi

# Set display for headless systems (optional)
if [ -z "$DISPLAY" ] && [ -n "$SSH_CONNECTION" ]; then
    print_warning "No DISPLAY variable set (SSH session detected)"
    print_warning "GUI may not work without X11 forwarding"
    print_warning "Try: ssh -X or ssh -Y for X11 forwarding"
fi

# Run the application
print_status "Launching NVIDIA-SMI GUI Monitor..."
print_status "Close the GUI window to exit"

# Run with error handling
if $PYTHON_CMD App.py; then
    print_success "Application closed successfully"
else
    exit_code=$?
    print_error "Application exited with error code: $exit_code"
    
    # Provide helpful error messages based on common exit codes
    case $exit_code in
        1)
            print_error "General error - check the application output above"
            ;;
        2)
            print_error "Misuse of shell builtin"
            ;;
        126)
            print_error "Command invoked cannot execute"
            ;;
        127)
            print_error "Command not found"
            ;;
        130)
            print_error "Script terminated by Control-C"
            ;;
        *)
            print_error "Unknown error occurred"
            ;;
    esac
    
    exit $exit_code
fi
