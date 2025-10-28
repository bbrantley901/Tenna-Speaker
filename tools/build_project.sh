#!/usr/bin/env bash
set -euo pipefail

ERROR_LEVEL=0
export PICO_EXTRAS_PATH="$HOME/pico-extras"
export PICO_SDK_PATH="$HOME/.pico-sdk/pico-sdk/2.1.0"

echo "PICO_EXTRAS_PATH is set to: $PICO_EXTRAS_PATH"
echo "PICO_SDK_PATH is set to: $PICO_SDK_PATH"
function cmake_configure() {
  echo "Configuring CMake project"
  mkdir -p build
  cd build
  cmake ../
  ERROR_LEVEL=$?
  if [ $ERROR_LEVEL -ne 0 ]; then
    echo "Failed to configure CMake, exit status: $ERROR_LEVEL"
    exit $ERROR_LEVEL
  fi
}

function build_project() {
  echo "Building project"
  cmake --build .
  ERROR_LEVEL=$?
  if [$ERROR_LEVEL -ne 0]; then
    echo "Failed to build project, exit status: $ERROR_LEVEL"
    exit
  fi
}

cd ..

if [ -d "build" ]; then
  echo "build directory exists"
else
  echo "Creating build directory and configuring CMake"
fi

cmake_configure
build_project
echo "Build completed successfully"


