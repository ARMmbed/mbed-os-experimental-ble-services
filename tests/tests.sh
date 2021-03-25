#!/bin/bash
set -e

# set pwd to script location
cd "$( dirname "$0" )"

# Bootstrap of the environment
../bootstrap.sh

# Link Loss Service builds
cd TESTS/LinkLoss/device
mbed compile -t GCC_ARM -m NRF52840_DK
mbedtools compile -t GCC_ARM -m NRF52840_DK
mbed compile -t GCC_ARM -m DISCO_L496AG
mbedtools compile -t GCC_ARM -m DISCO_L496AG

# FOTA Service builds
cd TESTS/FOTA/device
mbed compile -t GCC_ARM -m NRF52840_DK
mbedtools compile -t GCC_ARM -m NRF52840_DK
mbed compile -t GCC_ARM -m DISCO_L496AG
mbedtools compile -t GCC_ARM -m DISCO_L496AG
