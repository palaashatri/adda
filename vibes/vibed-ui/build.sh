#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

BUILD_DIR="build"
BUILD_TYPE="Release"
RUN_DEMO=""
INSTALL_DEPS=0
CLEAN=0

print_help() {
    cat <<'EOF'
Usage: ./build.sh [options]

Options:
  --build-type <type>    Build type (Debug, Release). Default: Release
    --run <demo>           Run a demo target after build (demo_* target name)
  --install-deps         Attempt to install missing dependencies
  --clean                Remove build directory before configuring
  --help                 Show this help

Examples:
  ./build.sh
  ./build.sh --build-type Debug
    ./build.sh --run demo_text_editor
  ./build.sh --install-deps --run demo_basic
EOF
}

command_exists() {
    command -v "$1" >/dev/null 2>&1
}

run_with_sudo() {
    if [[ "$(id -u)" -eq 0 ]]; then
        "$@"
    else
        sudo "$@"
    fi
}

install_linux_deps() {
    if command_exists apt-get; then
        run_with_sudo apt-get update
        run_with_sudo apt-get install -y build-essential cmake pkg-config libx11-dev
        return
    fi

    if command_exists dnf; then
        run_with_sudo dnf install -y gcc-c++ make cmake pkgconf-pkg-config libX11-devel
        return
    fi

    if command_exists pacman; then
        run_with_sudo pacman -Sy --noconfirm base-devel cmake pkgconf libx11
        return
    fi

    if command_exists zypper; then
        run_with_sudo zypper install -y gcc-c++ make cmake pkg-config libX11-devel
        return
    fi

    echo "Unsupported Linux package manager. Install dependencies manually: cmake, C++ compiler, pkg-config, X11 headers."
    exit 1
}

ensure_macos_deps() {
    if ! xcode-select -p >/dev/null 2>&1; then
        if [[ "$INSTALL_DEPS" -eq 1 ]]; then
            echo "Installing Xcode command line tools..."
            xcode-select --install || true
            echo "Finish the Xcode tools installation, then rerun this script."
            exit 1
        fi
        echo "Missing Xcode command line tools. Run: xcode-select --install"
        exit 1
    fi

    if ! command_exists cmake; then
        if [[ "$INSTALL_DEPS" -eq 1 ]]; then
            if command_exists brew; then
                brew install cmake
            else
                echo "Homebrew is not installed. Install Homebrew first: https://brew.sh/"
                exit 1
            fi
        else
            echo "Missing cmake. Install it with Homebrew (brew install cmake) or rerun with --install-deps."
            exit 1
        fi
    fi
}

ensure_linux_deps() {
    local missing=0

    if ! command_exists cmake; then
        missing=1
    fi
    if ! command_exists c++; then
        missing=1
    fi
    if ! command_exists pkg-config; then
        missing=1
    fi
    if command_exists pkg-config && ! pkg-config --exists x11; then
        missing=1
    fi

    if [[ "$missing" -eq 1 ]]; then
        if [[ "$INSTALL_DEPS" -eq 1 ]]; then
            install_linux_deps
        else
            echo "Missing Linux build dependencies. Rerun with --install-deps or install: cmake, C++ compiler, pkg-config, libx11-dev."
            exit 1
        fi
    fi

    if ! pkg-config --exists x11; then
        echo "X11 development package is still missing after dependency install attempt."
        exit 1
    fi
}

ensure_common_tools() {
    if ! command_exists cmake; then
        echo "cmake is required but not found."
        exit 1
    fi
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-type)
            if [[ $# -lt 2 ]]; then
                echo "--build-type requires a value"
                exit 1
            fi
            BUILD_TYPE="$2"
            shift 2
            ;;
        --run)
            if [[ $# -lt 2 ]]; then
                echo "--run requires a demo name"
                exit 1
            fi
            RUN_DEMO="$2"
            shift 2
            ;;
        --install-deps)
            INSTALL_DEPS=1
            shift
            ;;
        --clean)
            CLEAN=1
            shift
            ;;
        --help)
            print_help
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            print_help
            exit 1
            ;;
    esac
done

if [[ -n "$RUN_DEMO" && "$RUN_DEMO" != demo_* ]]; then
    echo "Unsupported demo target name: $RUN_DEMO"
    echo "Demo target must start with demo_"
    exit 1
fi

OS_NAME="$(uname -s)"
case "$OS_NAME" in
    Linux)
        ensure_linux_deps
        ;;
    Darwin)
        ensure_macos_deps
        ;;
    *)
        echo "Unsupported OS for build.sh: $OS_NAME"
        echo "Use build.bat on Windows."
        exit 1
        ;;
esac

ensure_common_tools

if [[ "$CLEAN" -eq 1 ]]; then
    rm -rf "$BUILD_DIR"
fi

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$BUILD_DIR" -j

if [[ -n "$RUN_DEMO" ]]; then
    DEMO_PATH="$BUILD_DIR/examples/$RUN_DEMO"
    if [[ ! -x "$DEMO_PATH" ]]; then
        ALT_PATH="$BUILD_DIR/examples/$BUILD_TYPE/$RUN_DEMO"
        if [[ -x "$ALT_PATH" ]]; then
            DEMO_PATH="$ALT_PATH"
        fi
    fi

    if [[ ! -x "$DEMO_PATH" ]]; then
        echo "Unable to locate runnable demo binary for: $RUN_DEMO"
        exit 1
    fi

    echo "Running $RUN_DEMO..."
    "$DEMO_PATH"
fi

echo "Build completed successfully."
