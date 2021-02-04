#!/bin/bash
set -e

# we need stubs from mbed-os
if [ -d "mbed-os" ]
then
    echo "Using existing mbed-os"
else
    #git clone https://github.com/ARMmbed/mbed-os.git
    # until it's not merged we use my branch
    git clone --depth 1 https://github.com/paul-szczepanek-arm/mbed-os.git -b cmake-tests
fi

cmake -S . -B cmake_build -GNinja
cmake --build cmake_build
CTEST_OUTPUT_ON_FAILURE=TRUE cmake --build cmake_build --target test
