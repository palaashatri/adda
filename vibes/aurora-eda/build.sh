#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
CONFIG="RelWithDebInfo"
GENERATOR=""
BUILD_UI="ON"
BUILD_PYTHON="ON"
INSTALL_DEPS=0
RUN_APP=0
RUN_TESTS=1
CLEAN=0

usage() {
  cat <<'EOF'
Usage: ./build.sh [options]

Build aurora-eda and optionally install dependencies, run tests, and launch the app.

Options:
  --install-deps       Install common build dependencies for this platform.
  --run                Run the aurora-eda application after building.
  --no-test            Skip ctest after building.
  --clean              Remove the build directory before configuring.
  --debug              Build Debug configuration.
  --release            Build Release configuration.
  --config <name>      Build configuration (default: RelWithDebInfo).
  --build-dir <path>   Build directory (default: ./build).
  --generator <name>   CMake generator, such as Ninja.
  --no-ui              Configure without Qt UI support.
  --no-python          Configure without pybind11 Python bindings.
  -h, --help           Show this help.

Examples:
  ./build.sh --install-deps
  ./build.sh --run
  ./build.sh --config Debug --run
EOF
}

require_command() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "error: required command not found: $1" >&2
    return 1
  fi
}

run_as_root() {
  if [ "$(id -u)" -eq 0 ]; then
    "$@"
  elif command -v sudo >/dev/null 2>&1; then
    sudo "$@"
  else
    echo "error: sudo is required to install dependencies" >&2
    return 1
  fi
}

install_macos_deps() {
  require_command brew || {
    echo "Install Homebrew first: https://brew.sh" >&2
    return 1
  }

  brew install cmake qt pybind11 fmt spdlog nlohmann-json ngspice
}

install_linux_deps() {
  if command -v apt-get >/dev/null 2>&1; then
    run_as_root apt-get update
    run_as_root apt-get install -y \
      build-essential cmake ninja-build pkg-config \
      qt6-base-dev qt6-tools-dev libgl1-mesa-dev \
      python3-dev python3-pip pybind11-dev \
      nlohmann-json3-dev libfmt-dev libspdlog-dev ngspice
  elif command -v dnf >/dev/null 2>&1; then
    run_as_root dnf install -y \
      gcc-c++ cmake ninja-build pkgconf-pkg-config \
      qt6-qtbase-devel python3-devel pybind11-devel \
      nlohmann-json-devel fmt-devel spdlog-devel ngspice
  elif command -v pacman >/dev/null 2>&1; then
    run_as_root pacman -Sy --needed \
      base-devel cmake ninja pkgconf qt6-base python pybind11 \
      nlohmann-json fmt spdlog ngspice
  else
    echo "error: unsupported Linux package manager. Install CMake, a C++20 compiler, Qt 6, and pybind11 manually." >&2
    return 1
  fi
}

install_dependencies() {
  case "$(uname -s)" in
    Darwin)
      install_macos_deps
      ;;
    Linux)
      install_linux_deps
      ;;
    *)
      echo "error: unsupported platform for automatic dependency installation: $(uname -s)" >&2
      return 1
      ;;
  esac
}

add_qt_prefix_path() {
  if [ "$(uname -s)" = "Darwin" ] && command -v brew >/dev/null 2>&1; then
    local qt_prefix
    qt_prefix="$(brew --prefix qt 2>/dev/null || true)"
    if [ -n "$qt_prefix" ]; then
      export CMAKE_PREFIX_PATH="$qt_prefix${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
    fi
  fi
}

find_app_binary() {
  local candidate
  for candidate in \
    "$BUILD_DIR/src/ui/aurora-eda" \
    "$BUILD_DIR/src/ui/$CONFIG/aurora-eda" \
    "$BUILD_DIR/src/ui/aurora-eda.app/Contents/MacOS/aurora-eda"; do
    if [ -x "$candidate" ]; then
      printf '%s\n' "$candidate"
      return 0
    fi
  done

  return 1
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --install-deps)
      INSTALL_DEPS=1
      ;;
    --run)
      RUN_APP=1
      ;;
    --no-test)
      RUN_TESTS=0
      ;;
    --clean)
      CLEAN=1
      ;;
    --debug)
      CONFIG="Debug"
      ;;
    --release)
      CONFIG="Release"
      ;;
    --config)
      CONFIG="${2:?--config requires a value}"
      shift
      ;;
    --build-dir)
      BUILD_DIR="${2:?--build-dir requires a value}"
      shift
      ;;
    --generator)
      GENERATOR="${2:?--generator requires a value}"
      shift
      ;;
    --no-ui)
      BUILD_UI="OFF"
      ;;
    --no-python)
      BUILD_PYTHON="OFF"
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "error: unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
  shift
done

if [ "$INSTALL_DEPS" -eq 1 ]; then
  install_dependencies
fi

require_command cmake
add_qt_prefix_path

case "$BUILD_DIR" in
  /*) ;;
  *) BUILD_DIR="$ROOT_DIR/$BUILD_DIR" ;;
esac

if [ "$CLEAN" -eq 1 ]; then
  rm -rf "$BUILD_DIR"
fi

cmake_args=(
  -S "$ROOT_DIR"
  -B "$BUILD_DIR"
  -DCMAKE_BUILD_TYPE="$CONFIG"
  -DAURORA_BUILD_UI="$BUILD_UI"
  -DAURORA_BUILD_PYTHON="$BUILD_PYTHON"
  -DAURORA_BUILD_TESTS=ON
)

if [ -n "$GENERATOR" ]; then
  cmake_args+=(-G "$GENERATOR")
fi

cmake "${cmake_args[@]}"
cmake --build "$BUILD_DIR" --config "$CONFIG"

if [ "$RUN_TESTS" -eq 1 ]; then
  ctest --test-dir "$BUILD_DIR" --build-config "$CONFIG" --output-on-failure
fi

if [ "$RUN_APP" -eq 1 ]; then
  app_binary="$(find_app_binary)" || {
    echo "error: aurora-eda executable was not found under $BUILD_DIR" >&2
    exit 1
  }
  "$app_binary"
fi
