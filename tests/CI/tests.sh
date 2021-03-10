#!/bin/bash
set -e

# Test if the script is running on windows
windows() {
    [[ -n "$WINDIR" ]];
}

# set pwd to script location
cd "$( dirname "$0" )"

# Bootstrap of the environment
../../bootstrap.sh

# Activate virtual environment
source ../../activate.sh

# Link Loss Service builds
cd ../TESTS/LinkLoss/device
mbed compile -t GCC_ARM -m NRF52840_DK
# TODO: support new tools
# mbed-tools compile -t GCC_ARM -m NRF52840_DK
mbed compile -t GCC_ARM -m DISCO_L496AG
# TODO: support new tools
# mbed-tools compile -t GCC_ARM -m DISCO_L496AG
