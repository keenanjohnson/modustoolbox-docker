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

# Add CyBridge Library to CMake target
#
# Inputs (defined as environment variables in ci_config.sh or dev_config.sh):
#   CYBRIDGE_DIR            CyBridge base directory

include_guard()
include(MtbAddQt5)

# Set CyBridge base directory
set(CyBridge_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CyBridge")

set(_release_config "Release")
set(_debug_config "Debug")

unset(CyBridge_INCLUDE_DIR CACHE)
find_path(CyBridge_INCLUDE_DIR
  NAMES "cybridge.h"
  PATHS "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  DOC "CyBridge include directory"
  NO_DEFAULT_PATH
)
mark_as_advanced(CyBridge_INCLUDE_DIR)

# CyBridge_LIBRARY is used for Release Configuration
unset(CyBridge_LIBRARY CACHE)
find_library(CyBridge_LIBRARY
  NAMES "cybridge"
  PATHS "${CyBridge_DIR}/${_release_config}/lib"
  DOC "CyBridge library (release)"
  NO_DEFAULT_PATH
  REQUIRED
)

# CyBridge_LIBRARY_DEBUG is used for Debug Configuration
unset(CyBridge_LIBRARY_DEBUG CACHE)
find_library(CyBridge_LIBRARY_DEBUG
  NAMES "cybridge"
  PATHS "${CyBridge_DIR}/${_debug_config}/lib"
  DOC "CyBridge library (debug)"
  NO_DEFAULT_PATH
  REQUIRED
)

add_library(CyBridge::CyBridge SHARED IMPORTED GLOBAL)
set_target_properties(CyBridge::CyBridge PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CyBridge_INCLUDE_DIR}"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_CONFIGURATIONS "Debug,Release"
  MAP_IMPORTED_CONFIG_MINSIZEREL "Release"
  MAP_IMPORTED_CONFIG_RELWITHDEBINFO "Release"
  IMPORTED_LOCATION_RELEASE "${CyBridge_LIBRARY}"
  IMPORTED_IMPLIB_RELEASE "${CyBridge_LIBRARY}"
  IMPORTED_LOCATION_DEBUG "${CyBridge_LIBRARY_DEBUG}"
  IMPORTED_IMPLIB_DEBUG "${CyBridge_LIBRARY_DEBUG}"
  IMPORTED_NO_SONAME ON
)

unset(libusb_LIBRARY CACHE)
# Note: find_library cannot find *.dll without *.lib
find_file(libusb_LIBRARY
  NAMES "libusb-1.0.dll" "libusb-1.0.0.dylib" "libusb-1.0.so.0"
  PATHS "${CyBridge_DIR}/${_release_config}/lib"
  DOC "libusb-1.0 library"
  NO_DEFAULT_PATH
  REQUIRED
)
if(NOT TARGET libusb::libusb)
  add_library(libusb::libusb SHARED IMPORTED GLOBAL)
  set_target_properties(libusb::libusb PROPERTIES
    IMPORTED_LOCATION "${libusb_LIBRARY}"
  )
endif()

function(mtb_deploy_libusb target_name)
  if(APPLE)
    # libusb-1.0.0.dylib has wrong LC_ID_DYLIB: fix it on-the-fly
    add_custom_command(TARGET "${target_name}" PRE_BUILD VERBATIM
      COMMAND install_name_tool -id libusb-1.0.0.dylib $<TARGET_FILE:libusb::libusb>
    )
  endif()

  # Copy libusb to the target directory
  add_custom_command(TARGET "${target_name}" POST_BUILD VERBATIM
    COMMENT "Deploying libusb for target ${target_name}"
    # Copy libusb library
    COMMAND "${CMAKE_COMMAND}"
      -DSOURCE_LIB=$<TARGET_FILE:libusb::libusb>
      -DTARGET_EXE=$<TARGET_FILE:${target_name}>
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeployLibrary.cmake"
  )
endfunction()

function(mtb_deploy_cybridge target_name)
  mtb_deploy_libusb(${target_name})

  # Copy libcybridge to the target directory
  add_custom_command(TARGET "${target_name}" POST_BUILD VERBATIM
    COMMENT "Deploying CyBridge for target ${target_name}"
    # Copy CyBridge library
    COMMAND "${CMAKE_COMMAND}"
      -DSOURCE_LIB=$<TARGET_FILE:CyBridge::CyBridge>
      -DTARGET_EXE=$<TARGET_FILE:${target_name}>
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeployLibrary.cmake"
    # Copy devdb.xml
    COMMAND "${CMAKE_COMMAND}" -E copy "${CyBridge_DIR}/${_release_config}/lib/devdb.xml"
      "$<TARGET_FILE_DIR:${target_name}>"
    USES_TERMINAL
  )
  
  if(WIN32)
    add_custom_command(TARGET ${target_name} POST_BUILD VERBATIM
      COMMENT "Running windeployqt for Cybridge.dll"
      COMMAND "${WINDEPLOYQT_EXECUTABLE}" ${WINDEPLOYQT_ARGS} "$<TARGET_FILE_DIR:${target_name}>/cybridge.dll"
      USES_TERMINAL
    )
  endif()
endfunction()
