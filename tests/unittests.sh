#!/bin/bash
set -e

# set pwd to script location
cd "$( dirname "$0" )"

# we need stubs from mbed-os
if [ -d "mbed-os" ]
then
    echo "Using existing mbed-os"
else
    #git clone https://github.com/ARMmbed/mbed-os.git
    # until it's not merged we use my branch
    git clone --depth 1 https://github.com/paul-szczepanek-arm/mbed-os.git -b cmake-tests
fi

cmake -S . -B cmake_build -GNinja -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE:STRING=xml
cmake --build cmake_build

# normal test
(cd cmake_build; ctest -V)
# valgrind
(cd cmake_build; ctest -D ExperimentalMemCheck)
# gcov (only show coverage of services)
gcovr --html=coverage.html  -f ".*cmake_build/services.*"


