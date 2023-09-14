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

# Deploy shared library (*.dll, *.so or *.dylib)
# to the target runtime directory
#
# Usage: cmake -DSOURCE_LIB=<path-to-library>
#              -DTARGET_EXE=<path-to-executable-target>
#              -P<path-to-this-script>

cmake_minimum_required(VERSION 3.21.1)

# Uncomment below lines to debug this script
#message(STATUS SOURCE_LIB=${SOURCE_LIB})
#message(STATUS TARGET_EXE=${TARGET_EXE})

get_filename_component(__source_lib_ext "${SOURCE_LIB}" EXT)
get_filename_component(__source_lib_dir "${SOURCE_LIB}" DIRECTORY)
get_filename_component(__source_lib_name_we "${SOURCE_LIB}" NAME_WE)

# Windows: deploy the *.dll instead of the import *.lib
if(WIN32)
  if("${__source_lib_ext}" STREQUAL ".lib")
    set(SOURCE_LIB "${__source_lib_dir}/${__source_lib_name_we}.dll")
  endif()
endif()

# Determine the target runtime library directory
get_filename_component(__target_dir "${TARGET_EXE}" DIRECTORY)
if(WIN32)
  set(__target_lib_dir "${__target_dir}")
elseif(APPLE)
  set(__target_lib_dir "${__target_dir}/../Frameworks")
else() # Linux
  set(__target_lib_dir "${__target_dir}/../lib")
endif()

# Create the target directory
file(MAKE_DIRECTORY "${__target_lib_dir}")

# Copy the library to the target directory, set executable bit
# Skip DLLs built from the same directory as target executable
if(NOT "${__source_lib_dir}" STREQUAL "${__target_lib_dir}")
  file(COPY "${SOURCE_LIB}" DESTINATION "${__target_lib_dir}" FILE_PERMISSIONS
    OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
  )
endif()

# macOS: patch the executable to define the correct library rpath
if(APPLE)
  get_filename_component(__target_lib_name "${SOURCE_LIB}" NAME)
  execute_process(COMMAND install_name_tool -change "${__target_lib_name}"
    "@loader_path/../Frameworks/${__target_lib_name}" "${TARGET_EXE}"
  )
endif()
