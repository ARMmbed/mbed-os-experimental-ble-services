#!/bin/bash
set -e

# set pwd to script location
cd "$( dirname "$0" )"

# Bootstrap of the environment
../bootstrap.sh

# Build Mbed apps
cd TESTS/LinkLoss/device
mbed compile -t GCC_ARM -m K64F
