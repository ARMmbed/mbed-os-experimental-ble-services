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

usage() {
  cat <<HELP_USAGE
  $0 -s <service> -t <toolchain> -m <target>
  -s Service test suite
  -t Compile toolchain
  -m Compile target
HELP_USAGE
}

# Set wd to script location
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Activate virtual environment
source ../../scripts/activate.sh

# Parse options
while getopts "s:t:m:h" opt
do
  case "$opt" in
   s)
     service="$OPTARG";;
   t)
     toolchain="$OPTARG";;
   m)
     target="$OPTARG";;
   h)
     usage; exit 1;;
  \?)
    echo "Invalid option: -$OPTARG" 1>&2; exit 1;;
  : )
    echo "Invalid option: -$OPTARG requires an argument" 1>&2; exit 1;;
  esac
done

# Verify that all arguments exist
if [[ (-z "$toolchain") ||  (-z "$target") || (-z "$service") ]]
then
  echo "No arguments supplied." 1>&2
  exit 1
fi

# Enter device folder for specified service
cd "$service"/device

# Build Mbed BLE app (new tools)
mbed-tools compile -t "$toolchain" -m "$target"

# Build Mbed BLE app (old tools)
mbed compile -t "$toolchain" -m "$target"

# Deactivate virtual environment
deactivate

exit 0
