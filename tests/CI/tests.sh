#!/bin/bash
set -e

# set pwd to script location
cd "$( dirname "$0" )"

# Bootstrap of the environment
../../bootstrap.sh

# Link Loss Service builds
cd ../TESTS/LinkLoss/device
mbed compile -t GCC_ARM -m NRF52840_DK
mbed-tools compile -t GCC_ARM -m NRF52840_DK
mbed compile -t GCC_ARM -m DISCO_L496AG
mbed-tools compile -t GCC_ARM -m DISCO_L496AG
