#!/bin/bash
set -e

# Test if the script is running on windows
windows() {
    [[ -n "$WINDIR" ]];
}

# Load symlink script, it sets the current directory to the root of
# the repository and export its path in a ROOT variable
source $(dirname $0)/symlink.sh

cd "$ROOT"

# Enter the test folder
cd ./tests

# We need stubs from mbed-os
if [ -d "mbed-os" ]
then
    echo "Using existing mbed-os"
else
    # git clone https://github.com/ARMmbed/mbed-os.git
    # until it's not merged we use my branch
    git clone --depth 1 https://github.com/ARMmbed/mbed-os.git -b feature-bluetooth-unit-test
fi

# Add symbolic links
cd "$ROOT"
symlink "tests/mbed-os" "tests/TESTS/LinkLoss/device/mbed-os"
symlink "services/LinkLoss" "tests/TESTS/LinkLoss/device/LinkLoss"

# Enter the integration testing folder
cd ./tests/TESTS

# Create virtual environment
if [ -d "venv" ]
then
  echo "Using existing virtual environment"
else
  mkdir venv
  # On Windows, the Python 3 executable is called 'python'
  if windows; then
    python  -m virtualenv venv
  else
    python3 -m virtualenv venv
  fi
fi

# Enter the virtual environment folder
cd venv

# Activate virtual environment
if windows; then
  source Scripts/activate
else
  source bin/activate
fi

# Install mbed-os requirements
pip install -r ../../mbed-os/requirements.txt

# Install testing requirements
pip install -r ../requirements.txt
