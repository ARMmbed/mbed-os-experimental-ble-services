#!/bin/bash
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# Repository root
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"/.. || exit

# Load symlink script
source "$ROOT"/scripts/symlink.sh

# Clone mbed-os
if [ -d "$ROOT/dependencies/mbed-os" ]
then
    echo "Using existing mbed-os"
else
    # git clone https://github.com/ARMmbed/mbed-os.git
    # Use feature branch until merged to master
    git clone --depth 1 https://github.com/ARMmbed/mbed-os.git -b feature_unittest_refactor "$ROOT"/dependencies/mbed-os
fi

# Add symlinks
symlink "$ROOT"/dependencies/mbed-os "$ROOT"/tests/UNITTESTS/mbed-os

symlink "$ROOT"/dependencies/mbed-os "$ROOT"/tests/TESTS/FOTA/device/mbed-os
symlink "$ROOT"/common               "$ROOT"/tests/TESTS/FOTA/device/common
symlink "$ROOT"/descriptors          "$ROOT"/tests/TESTS/FOTA/device/descriptors
symlink "$ROOT"/services/FOTA        "$ROOT"/tests/TESTS/FOTA/device/FOTA

symlink "$ROOT"/dependencies/mbed-os "$ROOT"/tests/TESTS/LinkLoss/device/mbed-os
symlink "$ROOT"/services/LinkLoss "$ROOT"/tests/TESTS/LinkLoss/device/LinkLoss

# Create virtual environment
if [ -d "$ROOT/tests/TESTS/venv" ]
then
  echo "Using existing virtual environment"
else
  mkdir "$ROOT"/tests/TESTS/venv
  # On Windows, the Python 3 executable is called 'python'
  if windows; then
    python  -m virtualenv "$ROOT"/tests/TESTS/venv
  else
    python3 -m virtualenv "$ROOT"/tests/TESTS/venv
  fi
fi

# Activate virtual environment
source "$ROOT"/scripts/activate.sh

# Install mbed-os requirements
pip install -r "$ROOT"/dependencies/mbed-os/requirements.txt

# Install testing requirements
pip install -r "$ROOT"/tests/TESTS/requirements.txt

# Install cli and tools
pip install --upgrade mbed-cli mbed-tools
