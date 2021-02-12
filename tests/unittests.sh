#!/bin/bash
set -e

# set pwd to script location
cd "$( dirname "$0" )"

# Bootstrap of the environment 
../bootstrap.sh

cmake -S . -B cmake_build -GNinja -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE:STRING=xml
cmake --build cmake_build

# normal test
(cd cmake_build; ctest -V)
# valgrind
(cd cmake_build; ctest -D ExperimentalMemCheck)
# gcov (only show coverage of services)
gcovr --html=coverage.html  -f ".*cmake_build/services.*"
