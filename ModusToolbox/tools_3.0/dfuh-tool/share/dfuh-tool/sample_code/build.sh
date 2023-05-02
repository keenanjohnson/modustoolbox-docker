# Copyright 2018-2022 Cypress Semiconductor Corporation (an Infineon company)
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#!/bin/bash
# 
# this must be the first non-commented line in this script. It ensures
# bash doesn't choke on \r on Windows
(set -o igncr) 2>/dev/null && set -o igncr; # this comment is needed

set -e

if [ "$1" == "" ]; then TARGET="dfuh-tool_ALL"; else TARGET="$1"; fi

CONFIG=${CONFIG:-RelWithDebInfo}
if [ "x$CY_DEP_DIR" = "x" ]; then
  if [ "x$XDG_CACHE_HOME" != "x" ]; then
    CY_DEP_DIR="$XDG_CACHE_HOME/cydep"
  else
    CY_DEP_DIR="$HOME/.cache/cydep"
  fi
fi
export CY_DEP_DIR

case "$(uname -s)" in

   Darwin)
    # Mac OS X
    export SDKROOT="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
    export MACOSX_DEPLOYMENT_TARGET=10.13
    
    mkdir -p build/reports
    cd build

    if [ "x$GENERATOR_NAME" = "x" ]; then
        GENERATOR_NAME="Xcode"
    fi
    echo GENERATOR_NAME=$GENERATOR_NAME
    if [ "x$GENERATOR_NAME" = "xNinja" ]; then
        export CMAKE_CMD_EXTRA="${CMAKE_CMD_EXTRA} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    fi

    cmake -G "$GENERATOR_NAME" ${CMAKE_CMD_EXTRA} -DCMAKE_BUILD_TYPE=$CONFIG ..
    cmake --build . --config $CONFIG --target $TARGET
    ;;

   Linux)
    # Linux
    if [ "x$XDG_RUNTIME_DIR" = "x" ]; then
        # Qt expects QStandardPaths: XDG_RUNTIME_DIR to be defined by the environment.
        # When undefined, set it to $TMP/$UID to avoid fatal warning during unit test execution.
        # Note: the directory must be owned by the user
        export XDG_RUNTIME_DIR="/tmp/$(id -u)"
        mkdir -p "$XDG_RUNTIME_DIR"
    fi

    mkdir -p build/reports
    cd build

    if [ "x$GENERATOR_NAME" = "x" ]; then
        GENERATOR_NAME="Unix Makefiles"
    fi

    cmake -G "$GENERATOR_NAME" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=$CONFIG ..
    cmake --build . --config $CONFIG --target $TARGET
    ;;

   CYGWIN*|MINGW32*|MSYS*)
    # Windows
    echo 'Unsupported OS. Run build.bat instead.'
    exit 1
    ;;

   *)
    echo 'Unsupported OS'
    exit 1
    ;;
esac

echo "SUCCESS"
