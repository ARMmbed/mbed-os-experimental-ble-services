#!/usr/bin/env bash
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

ROOT="$(cd "$(dirname $0)" && pwd)"
FIRST_ARG=$1

# Test if the script is running on windows
windows() {
    [[ -n "$WINDIR" ]];
}

# link $target $linkname
symlink() {
    local TARGET=$1
    local LINKNAME=$2
    local FORCE_FLAG=$3

    # find depth of the target bycounting slashes in path
    local SLASHES="${LINKNAME//[^\/]}"
    local DEPTH=${#SLASHES}

    # make relative path by prepending ../
    local RELATIVE_TARGET="${TARGET}"
    for (( ; DEPTH>0 ; DEPTH-- ))
    do
        RELATIVE_TARGET="../${RELATIVE_TARGET}"
    done

    # replace slashes for windows paths
    local WIN_LINKNAME="\"${LINKNAME//\//\\}\""
    local WIN_TARGET="${TARGET//\//\\}"
    local WIN_RELATIVE_TARGET="\"${RELATIVE_TARGET//\//\\}\""

    # Make parent directory
    if [[ "$FIRST_ARG" != "clean" ]]; then
        mkdir -p $(dirname "$LINKNAME")
    fi

    # Link-creation/deletion mode.
    if [[ "$FIRST_ARG" == "clean" ]]; then
        echo "rm $ROOT/$LINKNAME"
        rm "$ROOT/$LINKNAME" 2> /dev/null || true
    else
        # Remove the target if already exists
        if [[ -a "$ROOT/$LINKNAME" ]]; then
            # Allow silently forced deletion, which is slightly dangerous unless
            # used for specific directories only.
            if [[ "$FORCE_FLAG" != "force" ]]; then
                # if $LINKNAME is a directory AND NOT a symlink, ask for confirmation.
                while [ -d "$ROOT/$LINKNAME" ] && [ ! -h "$ROOT/$LINKNAME" ]; do
                    read -p "You are about to delete $LINKNAME. Are you sure you want to proceed ?" yn
                    case $yn in
                        [Yy]* ) break;;
                        [Nn]* ) exit;;
                        * ) echo "Please answer yes or no.";;
                    esac
                done
            fi
            rm -rf "$ROOT/$LINKNAME"
        fi
        if windows; then
            if [ -d "$ROOT/$WIN_TARGET" ]; then
                cmd <<< "mklink /D $WIN_LINKNAME $WIN_RELATIVE_TARGET" > /dev/null
            else
                cmd <<< "mklink $WIN_LINKNAME $WIN_RELATIVE_TARGET" > /dev/null
            fi
        else
            ln -s "$RELATIVE_TARGET" "$ROOT/$LINKNAME"
        fi
        echo $LINKNAME >> .gitignore
    fi
}