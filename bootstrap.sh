#!/bin/bash
set -e

# set pwd to script location
cd "$( dirname "$0" )"

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

# Add symlinks for Link Loss Service integration tests
cd TESTS/LinkLoss/device
ln -s ../../../mbed-os mbed-os
cd services
ln -s ../../../../../services/LinkLoss LinkLoss
