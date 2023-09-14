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

include_guard()

# Store "cp" executable path in CMake cache.
# This ensures custom commands like mtb_copy_data_files can execute
# in Visual Studio IDE without cygwin in PATH.
find_program(CP cp REQUIRED)

# macOS "cp" doesn't support "-u" option
if(APPLE)
  set(CP_OPTS "-av")
else()
  set(CP_OPTS "-auv")
endif()

# Read the build number from the environment:
# - BUILD_NUMBER variable is defined for Jenkins jobs
# - Fallback to 0 (for developer build)
if(DEFINED ENV{BUILD_NUMBER})
  set(BUILD_NUMBER $ENV{BUILD_NUMBER})
else()
  set(BUILD_NUMBER 0)
endif()

# Use the CMake project version ("major.minor.patch") to determine 3-digit "short" tool version
function(mtb_get_project_version_short output_var)
  set(${output_var} "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" PARENT_SCOPE)
endfunction()

# Use the CMake project version + dynamic BUILD_NUMBER to determine 4-digit "long" tool version
function(mtb_get_project_version output_var)
  mtb_get_project_version_short(_version_short)
  set(${output_var} "${_version_short}.${BUILD_NUMBER}" PARENT_SCOPE)
endfunction()

# Recursively list all targets the given target depends on.
# See https://cmake.org/cmake/help/latest/prop_tgt/LINK_LIBRARIES.html
function(mtb_list_dependencies target_name deps)
  get_target_property(_target_type ${target_name} TYPE)
  if ("${_target_type}" STREQUAL "EXECUTABLE")
    get_target_property(__libs ${target_name} LINK_LIBRARIES)
  else()
    get_target_property(__libs ${target_name} INTERFACE_LINK_LIBRARIES)
  endif()
  set(__deps "")
  if(NOT __libs MATCHES ".*NOTFOUND")
    foreach(__lib ${__libs})
      list(APPEND __deps ${__lib})
      if(TARGET ${__lib})
        mtb_list_dependencies(${__lib} __lib_deps)
        list(APPEND __deps ${__lib_deps})
      endif()
    endforeach()
  endif()

  list(REMOVE_DUPLICATES __deps)
  #message("mtb_list_dependencies for ${target_name}: ${__deps}")
  set(${deps} ${__deps} PARENT_SCOPE)
endfunction()

