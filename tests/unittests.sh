#!/bin/bash
set -e

# we need stubs from mbed-os
if [ -d "mbed-os" ]
then
    echo "Using existing mbed-os"
else
    git clone https://github.com/ARMmbed/mbed-os.git
fi

cmake -S . -B cmake_build -GNinja
cmake --build cmake_build
cmake --build cmake_build --target test
