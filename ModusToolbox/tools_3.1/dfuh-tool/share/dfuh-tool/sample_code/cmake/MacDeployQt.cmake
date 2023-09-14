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

# MacDeployQt.cmake
# This script implements functionality similar to macdeployqt
# for CMake executable targets that are not application bundles.
#
# Note: this is standalone CMake script, it doesn't has access
# to variables and target properties configured for the current
# CMake project. All variables are passed as script arguments.
#
# Expected usage: call this script as post-build hook:
#
# add_custom_command(TARGET ${target_name} POST_BUILD
#   COMMAND "${CMAKE_COMMAND}"
#     -DTARGET_EXE="$<TARGET_FILE:${target_name}>"
#     -DQTDIR="${QTDIR}"
#     -DQTLIBS="Core;Network;Xml;XmlPatterns"
#     -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/MacDeployQt.cmake"
# )

cmake_minimum_required(VERSION 3.21.1)

# Convert the QTLIBS variable to a semi-colon separated list
# https://cmake.org/cmake/help/latest/command/separate_arguments.html
separate_arguments(QTLIBS)

# Uncomment the below line for script debugging
#message(STATUS "MacDeployQt: TARGET_EXE=${TARGET_EXE} QTLIBS=${QTLIBS}")

# Determine target directory name
get_filename_component(__bin_dir ${TARGET_EXE} DIRECTORY)

# Determine paths to Qt Frameworks and Plugins
set(__lib_dir "${__bin_dir}/../Frameworks")
set(__plugin_dir "${__bin_dir}/../PlugIns")

# Create Qt Frameworks directory
execute_process(
  COMMAND_ERROR_IS_FATAL ANY
  COMMAND mkdir -p "${__lib_dir}"
)

if("Gui" IN_LIST QTLIBS)
  # QtDBus.framework and QtPrintSupport.framework are indirect
  # dependencies, required by GUI platform plugins (libqcocoa.dylib)
  list(APPEND QTLIBS DBus PrintSupport)
  list(REMOVE_DUPLICATES QTLIBS)

  # Create Qt Plugins directory
  execute_process(
    COMMAND_ERROR_IS_FATAL ANY
    COMMAND mkdir -p "${__plugin_dir}"
  )

  file(COPY
    "${QTDIR}/plugins/bearer"
    "${QTDIR}/plugins/iconengines"
    "${QTDIR}/plugins/imageformats"
    "${QTDIR}/plugins/platforms"
    "${QTDIR}/plugins/printsupport"
    "${QTDIR}/plugins/styles"
    DESTINATION "${__plugin_dir}"
    FILES_MATCHING PATTERN "*.dylib"
  )
endif()

# Iterate over the library list (Core;Network;Xml;XmlPatterns)
foreach(__qt_lib ${QTLIBS})
  set(__qt_fwk "Qt${__qt_lib}.framework")
  set(__qt_lib_rpath "@executable_path/../Frameworks/${__qt_fwk}/Qt${__qt_lib}")
  # Copy the Qt framework directory, exclude Headers, and patch the
  # executable rpath with a relative path to the Qt framework library
  execute_process(
    COMMAND_ERROR_IS_FATAL ANY
    COMMAND rsync -a --exclude=Headers "${QTDIR}/lib/${__qt_fwk}/" "${__lib_dir}/${__qt_fwk}/"
    COMMAND install_name_tool -change "${__qt_lib}" "${__qt_lib_rpath}" "${TARGET_EXE}"
  )
endforeach()
