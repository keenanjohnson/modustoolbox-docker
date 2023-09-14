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

# Absolute path to Qt installation. For example: "C:/Qt/5.15.2/msvc2019_64"
export QTDIR=""

# Absolute path to Boost installation. For example: "C:/boost-1.76.0-msvc141-x86_64-mswin"
export BOOST_ROOT=""

# Absolute path to CMake executable, or bare "cmake" command if cmake is in PATH. For example: "C:/Qt/Tools/CMake_64/bin/cmake"
export CMAKE_COMMAND="cmake"

# Absolute path to Ninja executable, or bare "ninja" command if ninja is in PATH. For example: "C:/Qt/Tools/Ninja/ninja"
# Note: this is required only if GENERATOR_NAME="Ninja Multi-Config".
export NINJA_COMMAND="ninja"
