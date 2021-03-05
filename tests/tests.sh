#!/bin/bash
set -e

# set pwd to script location
cd "$( dirname "$0" )"

# Bootstrap of the environment
../bootstrap.sh

# -- Link Loss Service --
cd TESTS/LinkLoss/device
# Build for NRF52840_DK (old tools)
mbed compile -t GCC_ARM -m NRF52840_DK
# Build for NRF52840_DK (new tools)
mbedtools compile -t GCC_ARM -m NRF52840_DK
# Build for DISCO_L496AG (old tools)
mbed compile -t GCC_ARM -m DISCO_L496AG
# Build for DISCO_L496AG (new tools)
mbedtools compile -t GCC_ARM -m DISCO_L496AG
