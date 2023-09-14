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

# LinuxDeployQt.cmake
# This script implements functionality similar to macdeployqt
# and windeployqt for Linux executable targets.
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
#     -DQt5_VERSION_MAJOR="${Qt5_VERSION_MAJOR}"
#     -DQt5_VERSION="${Qt5_VERSION}"
#     -DQTLIBS="Core;Network;Xml;XmlPatterns"
#     -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/LinuxDeployQt.cmake"
# )

cmake_minimum_required(VERSION 3.21.1)

separate_arguments(QTLIBS)
#message(STATUS "LinuxDeployQt: TARGET_EXE=${TARGET_EXE} QTLIBS=${QTLIBS}")
get_filename_component(__bin_dir ${TARGET_EXE} DIRECTORY)
set(__lib_dir "${__bin_dir}/../lib")

if("Gui" IN_LIST QTLIBS)
  # libQt5DBus.so, libQt5XcbQpa.so and libQt5WaylandClient.so
  # are indirect dependencies, required by GUI platform plugins
  # (bin/platforms/libqxcb.so and bin/platforms/libqwayland-*.so)
  # libQt5Network.so is required by Squish test framework
  list(APPEND QTLIBS DBus XcbQpa Network)
  list(REMOVE_DUPLICATES QTLIBS)

  file(COPY
    "${QTDIR}/plugins/imageformats"
    "${QTDIR}/plugins/printsupport"
    DESTINATION "${__bin_dir}"
    FILES_MATCHING PATTERN "*.so"
  )
  # Only bundle XCB plugin (Wayland doesn't work well in Qt5)
  file(COPY "${QTDIR}/plugins/platforms/libqxcb.so" DESTINATION "${__bin_dir}/platforms")
endif()

foreach(__qt_lib ${QTLIBS})
  file(COPY
    "${QTDIR}/lib/libQt5${__qt_lib}.so.${Qt5_VERSION_MAJOR}"
    "${QTDIR}/lib/libQt5${__qt_lib}.so.${Qt5_VERSION}"
    DESTINATION "${__lib_dir}"
  )
endforeach()

file(COPY
  "${QTDIR}/lib/libicui18n.so.56"
  "${QTDIR}/lib/libicui18n.so.56.1"
  "${QTDIR}/lib/libicuuc.so.56"
  "${QTDIR}/lib/libicuuc.so.56.1"
  "${QTDIR}/lib/libicudata.so.56"
  "${QTDIR}/lib/libicudata.so.56.1"
  DESTINATION "${__lib_dir}"
)
