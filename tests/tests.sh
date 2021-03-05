#!/bin/bash
set -e

# set pwd to script location
cd "$( dirname "$0" )"

# Bootstrap of the environment
../bootstrap.sh

# -- Link Loss Service --
cd TESTS/LinkLoss/device
# Build for NRF52840_DK
mbed compile -t GCC_ARM -m NRF52840_DK
# Build for DISCO_L496AG
mbed compile -t GCC_ARM -m DISCO_L496AG
