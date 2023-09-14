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
include(MtbHelperFunctions)

# Check if QTDIR environment variable was set by the build script
if(DEFINED ENV{QTDIR})
  get_filename_component(QTDIR "$ENV{QTDIR}" ABSOLUTE)
endif()
if(NOT EXISTS "${QTDIR}")
  message(FATAL_ERROR "Qt5 directory not found, check QTDIR in build.sh")
endif()
set(QTDIR "${QTDIR}" CACHE PATH "Qt5 base directory." FORCE)
list(APPEND CMAKE_PREFIX_PATH "${QTDIR}/lib/cmake")

find_package(Qt5 COMPONENTS Core Gui Widgets LinguistTools PrintSupport SerialPort Svg Xml XmlPatterns REQUIRED)

if(WIN32)
  if(NOT WINDEPLOYQT_EXECUTABLE)
    find_program(WINDEPLOYQT_EXECUTABLE
      NAMES windeployqt
      PATHS "${QTDIR}/bin"
      NO_DEFAULT_PATH
      REQUIRED
    )
  endif()
  # http://doc.qt.io/qt-5/windows-deployment.html
  set(WINDEPLOYQT_ARGS --no-translations --no-system-d3d-compiler --no-compiler-runtime --no-webkit2 --no-angle --no-opengl-sw --verbose 1)
elseif(APPLE)
  if(NOT MACDEPLOYQT_EXECUTABLE)
    find_program(MACDEPLOYQT_EXECUTABLE
      NAMES macdeployqt
      PATHS "${QTDIR}/bin"
      NO_DEFAULT_PATH
      REQUIRED
    )
  endif()
  # http://doc.qt.io/qt-5/osx-deployment.html
  set(MACDEPLOYQT_ARGS -always-overwrite)
endif()

# Query the list of Qt library names the target depends on.
# Returns the result to qt_deps list in the following format:
# Core;Gui;Widgets;XmlPatterns;SerialPort;Xml;PrintSupport
function(mtb_list_qt_deps target_name qt_deps)
  mtb_list_dependencies(${target_name} __libs)
  set(__qt_deps "")
  foreach(__lib ${__libs})
    if ("${__lib}" MATCHES "^Qt5::(.*)")
      list(APPEND __qt_deps ${CMAKE_MATCH_1})
    endif()
  endforeach()

  list(REMOVE_DUPLICATES __qt_deps)
  #message(STATUS "mtb_list_qt_deps for ${target_name}: ${__qt_deps}")
  set(${qt_deps} ${__qt_deps} PARENT_SCOPE)
endfunction()

# Adds commands to copy Qt libraries and plugins to binary directory.
function(mtb_deploy_qt target_name)
  if(WIN32)
    add_custom_command(TARGET ${target_name} POST_BUILD VERBATIM
      COMMENT "Running windeployqt for ${target_name}"
      COMMAND "${WINDEPLOYQT_EXECUTABLE}" ${WINDEPLOYQT_ARGS} "$<TARGET_FILE:${target_name}>"
      USES_TERMINAL
    )
  elseif(APPLE)
    # Check if the target is an application bundle
    get_target_property(_is_macos_bundle ${target_name} MACOSX_BUNDLE)
    get_target_property(_main_app ${target_name} MAIN_APP)
    if(${_is_macos_bundle})
      add_custom_command(TARGET ${target_name} POST_BUILD VERBATIM
        COMMENT "Running macdeployqt for ${target_name}"
        COMMAND "${MACDEPLOYQT_EXECUTABLE}" "$<TARGET_BUNDLE_DIR:${target_name}>" ${MACDEPLOYQT_ARGS}
        USES_TERMINAL
      )
    elseif(NOT TARGET ${_main_app})
      # Note: skip Qt deployment for CLI applications with active
      # MAIN_APP GUI target: Qt Frameworks are reused by CLI and GUI
      mtb_list_qt_deps(${target_name} __qt_libs)
      add_custom_command(TARGET ${target_name} POST_BUILD
        COMMENT "Deploying Qt libraries for ${target_name}"
        COMMAND "${CMAKE_COMMAND}"
          -DTARGET_EXE="$<TARGET_FILE:${target_name}>"
          -DQTDIR="${QTDIR}"
          -DQTLIBS="${__qt_libs}"
          -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/MacDeployQt.cmake"
      )
    endif()
    unset(_is_macos_bundle)
    unset(_main_app)
  else() # Linux
    mtb_list_qt_deps(${target_name} __qt_libs)
    add_custom_command(TARGET ${target_name} POST_BUILD
      COMMENT "Deploying Qt libraries for ${target_name}"
      COMMAND "${CMAKE_COMMAND}"
        -DTARGET_EXE="$<TARGET_FILE:${target_name}>"
        -DQTDIR="${QTDIR}"
        -DQt5_VERSION_MAJOR="${Qt5_VERSION_MAJOR}"
        -DQt5_VERSION="${Qt5_VERSION}"
        -DQTLIBS="${__qt_libs}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/LinuxDeployQt.cmake"
    )
  endif()
endfunction()
