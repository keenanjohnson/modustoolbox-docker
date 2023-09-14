#!/bin/bash
#
# Copyright 2018-2023 Cypress Semiconductor Corporation (an Infineon company)
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
#
# Line below must be the first non-commented line in this script. It ensures bash doesn't choke on \r on Windows
(set -o igncr) 2>/dev/null && set -o igncr; # this comment is needed

# Include shell script with environment variables. Required for build system 
script_dir="$(dirname "${BASH_SOURCE[0]}")"
source "$script_dir/setenv.sh"

set -$-ue${DEBUG+xv}

# Check if QTDIR is set
if [[ $QTDIR == "" ]]; then
  echo "QTDIR must be specified in setenv.sh"
  exit 1
fi

# Check if BOOST_ROOT is set
if [[ $BOOST_ROOT == "" ]]; then
  echo "BOOST_ROOT must be specified in setenv.sh"
  exit 1
fi

# Path to the build output directory (relative to $PWD)
# Ninja generators change this to "b-n"
[[ -n ${BUILD_DIR:-} ]] || BUILD_DIR="build"

# Path to the tools packaging directory (relative to $PWD)
[[ -n ${INSTALL_DIR:-} ]] || INSTALL_DIR="tools_3.1"

# Check if this script is executed with custom arguments
if [[ $# -eq 0 ]]; then
  # No custom args: build the default target - "dfuh-tool_ALL"
  CMAKE_BUILD_ARGS="--target dfuh-tool_ALL"
else
  # Treat all script arguments as CMake targets (passed as-is to CMake)
  CMAKE_BUILD_ARGS="--target $*"
fi

# Build RelWithDebInfo by default
CONFIG=${CONFIG:-RelWithDebInfo}

KERNEL="$(uname -s)"
case "$KERNEL" in

  Darwin)
    # Select Qt target platform
    qt_platform="clang_64"

    # Define macOS SDK path and version
    # https://cmake.org/cmake/help/latest/variable/CMAKE_OSX_SYSROOT.html
    export SDKROOT="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
    # Ensure compatibility with macOS High Sierra (-mmacosx-version-min)
    # https://cmake.org/cmake/help/latest/envvar/MACOSX_DEPLOYMENT_TARGET.html
    export MACOSX_DEPLOYMENT_TARGET=10.13

    # By default, use Ninja Multi-Config generator
    # To use Xcode generator:
    # export GENERATOR_NAME="Xcode"
    # ./build.sh
    if [[ -z "${GENERATOR_NAME:-}" ]]; then
        GENERATOR_NAME="Ninja Multi-Config"
    fi
    ;;

  Linux)
    # Select Qt target platform
    qt_platform="gcc_64"

    if [[ -z "${XDG_RUNTIME_DIR:-}" ]]; then
        # Qt expects QStandardPaths: XDG_RUNTIME_DIR to be defined by the environment.
        # When undefined, set it to $BUILD_DIR/tmp to avoid fatal warning during unit test execution.
        export XDG_RUNTIME_DIR="$BUILD_DIR/tmp"
        mkdir -p "$XDG_RUNTIME_DIR"
    fi

    # For Linux, always use Ninja Multi-Config generator
    # https://cmake.org/cmake/help/latest/generator/Ninja%20Multi-Config.html
    if [[ -z "${GENERATOR_NAME:-}" ]]; then
        GENERATOR_NAME="Ninja Multi-Config"
    fi
    ;;

  CYGWIN*|MINGW*|MSYS*)
    # Select Qt target platform
    qt_platform="msvc2019_64"

    # Check whether VCToolsInstallDir directory path was defined by vsdevcmd.bat
    if [[ -z "${VCToolsInstallDir:-}" ]]; then
        echo "vsdevcmd.bat was not called: run build.bat"
        exit 1
    fi
    echo "VCToolsInstallDir=$VCToolsInstallDir"

    # By default, use Visual Studio project generator
    # To use Ninja Multi-Config generator:
    # export GENERATOR_NAME="Ninja Multi-Config"
    # ./build.bat
    if [[ -z "${GENERATOR_NAME:-}" ]]; then
        GENERATOR_NAME="Visual Studio 16 2019"
    fi

    # For Visual Studio generators, specify x64 as architecture target
    # https://cmake.org/cmake/help/latest/generator/Visual%20Studio%2016%202019.html
    if [[ $GENERATOR_NAME = "Visual Studio"* ]]; then
        CMAKE_CONFIGURE_ARGS+=" -A x64"
    fi
    ;;

  *)
    echo "FATAL ERROR: Unsupported OS: $KERNEL"
    exit 1
    ;;
esac

if [[ $GENERATOR_NAME = Ninja* ]]; then
  # Legacy Ninja generator is not supported, switch to Multi-Config:
  # https://cmake.org/cmake/help/latest/generator/Ninja%20Multi-Config.html
  GENERATOR_NAME="Ninja Multi-Config"

  # Use different build directory to enable easy switching b/w MSVC and Ninja builds
  BUILD_DIR="b-n"

  # Generate compile_commands.json for VSCode C++ IntelliSense
  # https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html
  CMAKE_CONFIGURE_ARGS+=" -DCMAKE_MAKE_PROGRAM=${NINJA_COMMAND} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
fi

# Skip CMake configure/build stages when sourced from another script
[[ "$0" == "${BASH_SOURCE[0]}" ]] || return 0

# Increase verbosity
set -x

# Configure CMake project
${CMAKE_COMMAND} -S . -B $BUILD_DIR -G "$GENERATOR_NAME" -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" ${CMAKE_CONFIGURE_ARGS:-}

# Build CMake project
${CMAKE_COMMAND} --build $BUILD_DIR --config $CONFIG --verbose $CMAKE_BUILD_ARGS

set +x

echo "SUCCESS"
