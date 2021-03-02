#!/bin/bash
set -e

# Load symlink script, it sets the current directory to the root of
# the repository and export its path in a ROOT variable
source $(dirname $0)/symlink.sh

cd "$ROOT"

# enter the test folder
cd ./tests

# we need stubs from mbed-os
if [ -d "mbed-os" ]
then
    echo "Using existing mbed-os"
else
    # git clone https://github.com/ARMmbed/mbed-os.git
    # until it's not merged we use my branch
    git clone --depth 1 https://github.com/ARMmbed/mbed-os.git -b feature-bluetooth-unit-test
fi

# Add symbolic links for Link Loss Service integration tests
cd "$ROOT"
symlink "tests/mbed-os" "tests/TESTS/LinkLoss/device/mbed-os"
symlink "services/LinkLoss" "tests/TESTS/LinkLoss/device/LinkLoss"
