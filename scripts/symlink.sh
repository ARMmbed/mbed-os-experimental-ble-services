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

FIRST_ARG=$1

# Test if script is running on Windows
windows() {
    [[ -n "$WINDIR" ]];
}

# Create symbolic link
symlink() {
    local TARGET=$1
    local LINKNAME=$2
    local FORCE_FLAG=$3

    # Find depth of target by counting slashes in path
    local SLASHES="${LINKNAME//[^\/]}"
    local DEPTH=${#SLASHES}

    # Make relative path
    local RELATIVE_TARGET="${TARGET}"
    for (( ; DEPTH>0 ; DEPTH-- ))
    do
        RELATIVE_TARGET="../${RELATIVE_TARGET}"
    done

    # Replace slashes for Windows paths
    local WIN_LINKNAME="\"${LINKNAME//\//\\}\""
    local WIN_TARGET="${TARGET//\//\\}"
    local WIN_RELATIVE_TARGET="\"${RELATIVE_TARGET//\//\\}\""

    # Make parent directory
    if [[ "$FIRST_ARG" != "clean" ]]; then
        mkdir -p "$(dirname "$LINKNAME")"
    fi

    # Link creation/deletion mode
    if [[ "$FIRST_ARG" == "clean" ]]; then
        echo "rm $LINKNAME"
        rm "$LINKNAME" 2> /dev/null || true
    else
        # Remove target if already exists
        if [[ -a "$LINKNAME" ]]; then
            # Allow forced deletion
            if [[ "$FORCE_FLAG" != "force" ]]; then
                # if $LINKNAME is a directory and not a symbolic link, ask for confirmation.
                while [ -d "$LINKNAME" ] && [ ! -h "$LINKNAME" ]; do
                    read -rp "You are about to delete $LINKNAME. Are you sure you want to proceed ?" yn
                    case $yn in
                        [Yy]* ) break;;
                        [Nn]* ) exit;;
                        * ) echo "Please answer yes or no.";;
                    esac
                done
            fi
            rm -rf "$LINKNAME"
        fi
        if windows; then
            if [ -d "$WIN_TARGET" ]; then
                cmd <<< "mklink /D $WIN_LINKNAME $WIN_RELATIVE_TARGET" > /dev/null
            else
                cmd <<< "mklink $WIN_LINKNAME $WIN_RELATIVE_TARGET" > /dev/null
            fi
        else
            ln -s "$RELATIVE_TARGET" "$LINKNAME"
        fi
        echo "$LINKNAME" >> .gitignore
    fi
}
