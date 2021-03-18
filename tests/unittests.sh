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

set -e

# Enter repository root
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"/..

# Bootstrap of the environment
./scripts/bootstrap.sh

# Activate virtual environment
source scripts/activate.sh

cd tests

cmake -S . -B cmake_build -GNinja -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE:STRING=xml
cmake --build cmake_build

# Normal test
(cd cmake_build; ctest -V)
# valgrind
(cd cmake_build; ctest -D ExperimentalMemCheck)
# gcov (only show coverage of services)
gcovr --html=coverage.html  -f ".*cmake_build/services.*"
