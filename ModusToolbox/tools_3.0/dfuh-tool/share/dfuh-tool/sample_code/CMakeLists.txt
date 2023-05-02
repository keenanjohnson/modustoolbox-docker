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

# To use this file:
# 1. Set the BOOST_ROOT variable to the directory containing Boost (Cypress uses Boost 1.76.0).
# 2. Set the QTDIR variable to the directory containing the compiler-specific subdirectory for Qt.
#    For example, Qt-5.15.2-x86_64-linux/5.15.2/gcc_64 (which also works for Clang, despite the directory name).
# 3. Set CyBridge_LIBRARY to point to the CyBridge shared library file in the ../lib directory (for Linux and Mac)
#    or the cybridge.lib file in this directory (for Windows).
# 4. Either run the build.bat script (on Windows) or the build.sh script (on Linux or Mac).
# 5. Copy the devdb.xml file, as well as the CyBridge.dll and libusb-1.0.dll files (Windows)
#    or the libcybridge.so and libusb-1.0.so.0 (Linux/Mac) files
#    from the dfuh-tool directory into the directory containing the executables
#    (usually build/RelWithDebInfo, unless the CONFIG environment variable is set to Debug, which makes it build/Debug).
# The dfuh-tool and dfuh-cli executables should be ready to run.

cmake_minimum_required(VERSION 3.12)

project(dfuh-tool)

set(BOOST_ROOT "")
set(QTDIR "")
set(CyBridge_LIBRARY "")

set(CMAKE_CXX_STANDARD 14)
# For Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

list(APPEND CMAKE_PREFIX_PATH "${QTDIR}/lib/cmake")
set(QT5_DIR "${QTDIR}")
find_package(Qt5 COMPONENTS Core Gui Widgets SerialPort)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS atomic program_options thread REQUIRED)
set(CyBridge_INCLUDE_DIR "include/")

# Common compiler flags
add_definitions(-DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE -DBOOST_ALL_NO_LIB)
if(MSVC)
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
  add_compile_options(/MP /W3 /w34189 /WX /utf-8 /FS)
  add_compile_options(/w34062)  # C4062: enumerator 'identifier' in switch of enum 'enumeration' is not handled
  add_compile_options(/w34189)  # C4189: 'identifier': local variable is initialized but not referenced
  add_compile_options(/w34266)  # C4266: 'virtual_function': no override available for virtual member function from base 'classname'; function is hidden
  add_compile_options(/w34355)  # C4344: 'this': used in base member initializer list
  add_compile_options(/w34701)  # C4701: Potentially uninitialized local variable 'name' used
  add_compile_options(/w35038)  # C5038: data member 'member1' will be initialized after data member 'member2'
else()
  add_compile_options(-Wall -Werror)
endif()
if(WIN32)
  add_definitions(-DNOMINMAX -DNOGDI)   # Disable macros that conflict with other names
endif()

set(GUI_TARGET ${PROJECT_NAME})
set(CLI_TARGET "dfuh-cli")

file(GLOB_RECURSE BACKEND_SOURCES "src/backend/*.cpp" "include/backend/*.h")
file(GLOB_RECURSE BOOTLOADER_SOURCES "src/backend/cybootloaderutils/*.c" "src/backend/cybootloaderutils/*.h")
file(GLOB_RECURSE CLI_SOURCES "src/cli/*.cpp" "include/cli/*.h")
file(GLOB_RECURSE GUI_SOURCES "src/gui/*.cpp" "include/gui/*.h")

file(GLOB_RECURSE GUI_RESOURCES "resources/*.qrc" "include/gui/*.ui")

make_directory("${CMAKE_CURRENT_BINARY_DIR}/lib")
make_directory("${CMAKE_CURRENT_BINARY_DIR}/bin")

if(WIN32)
  if(NOT WINDEPLOYQT_EXECUTABLE)
    find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt PATHS "${QTDIR}/bin")
  endif()
  # http://doc.qt.io/qt-5/windows-deployment.html
  set(WINDEPLOYQT_ARGS --no-translations --no-system-d3d-compiler --no-compiler-runtime --no-webkit2 --no-angle --no-opengl-sw)
elseif(APPLE)
  if(NOT MACDEPLOYQT_EXECUTABLE)
    find_program(MACDEPLOYQT_EXECUTABLE NAMES macdeployqt PATHS "${QTDIR}/bin")
  endif()
  # http://doc.qt.io/qt-5/osx-deployment.html
  set(MACDEPLOYQT_ARGS -always-overwrite)
endif()

# Query the list of Qt libraries the target depends on.
# See https://cmake.org/cmake/help/latest/prop_tgt/LINK_LIBRARIES.html
# Returns the result to qt_deps list in the following format:
# Core;Gui;Widgets;XmlPatterns;SerialPort;Xml;PrintSupport
function(cy_query_qt_deps target_name qt_deps)
  get_target_property(__libs ${target_name} LINK_LIBRARIES)
  set(__deps "")
  foreach(__lib ${__libs})
    if ("${__lib}" MATCHES "^Qt5::(.*)")
      list(APPEND __deps ${CMAKE_MATCH_1})
    elseif (TARGET ${__lib})
      cy_query_qt_deps(${__lib} __lib_deps)
      list(APPEND __deps ${__lib_deps})
    endif()
  endforeach()
  list(REMOVE_DUPLICATES __deps)
  set(${qt_deps} ${__deps} PARENT_SCOPE)
endfunction()

# Adds commands to copy Qt libraries and translations to binary directory.
function(cy_deploy_qt target_name)
  # Single-config generators (Ninja, Unix Makefiles) handle RUNTIME_OUTPUT_DIRECTORY
  # differently from multi-config generators (Visual Studio, XCode): there is no
  # config-specific sub-directory with build artifacts. Hack Ninja/Makefiles generators
  # to behave similarly to IDE generators - this enables consistent packaging logic.
  # Use undocumented CMake behavior: CMAKE_CONFIGURATION_TYPES variable is empty
  # for single-configuration generators
  if (CMAKE_CONFIGURATION_TYPES)
    set(_output_dir "${CMAKE_CURRENT_BINARY_DIR}")
  else()
    set(_output_dir "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
  endif()
  set_property(TARGET "${target_name}" PROPERTY RUNTIME_OUTPUT_DIRECTORY "${_output_dir}")
  if (WIN32)
    add_custom_command(TARGET "${target_name}" POST_BUILD VERBATIM
      COMMENT "Running windeployqt for ${target_name}"
      COMMAND "${WINDEPLOYQT_EXECUTABLE}" ${WINDEPLOYQT_ARGS} "$<TARGET_FILE:${target_name}>")
  elseif(APPLE)
    add_custom_command(TARGET "${target_name}" POST_BUILD
      COMMENT "Running macdeployqt for ${target_name}"
      COMMAND "${MACDEPLOYQT_EXECUTABLE}" "$<TARGET_BUNDLE_DIR:${target_name}>" ${MACDEPLOYQT_ARGS})
  else()
    # Linux
    set(__BIN_DIR "${_output_dir}/bin")
    set(__LIB_DIR "${_output_dir}/lib")
    set_property(TARGET "${target_name}" PROPERTY RUNTIME_OUTPUT_DIRECTORY "${__BIN_DIR}")
    set_property(TARGET "${target_name}" APPEND PROPERTY BUILD_RPATH "\$ORIGIN/../lib")
    file(MAKE_DIRECTORY "${__BIN_DIR}")
    file(MAKE_DIRECTORY "${__LIB_DIR}")
    cy_query_qt_deps("${target_name}" __qt_deps)
    # libQt5Network.so is pulled in by libQt5XmlPatterns.so
    # libQt5DBus.so and libQt5XcbQpa.so are indirect dependencies,
    # pulled in by platform plugin (bin/platforms/libqxcb.so)
    # libQt5WaylandClient.so is also an indirect dependency,
    # pulled in by bin/platforms/libqwayland-*.so
    list(APPEND __qt_deps Network DBus XcbQpa WaylandClient)
    #message(STATUS "${target_name} Qt dependencies: ${__qt_deps}")
    foreach(__qt_lib ${__qt_deps})
      file(COPY
        "${QTDIR}/lib/libQt5${__qt_lib}.so.${Qt5_VERSION_MAJOR}"
        "${QTDIR}/lib/libQt5${__qt_lib}.so.${Qt5_VERSION}"
        DESTINATION "${__LIB_DIR}")
    endforeach()
    file(COPY
      "${QTDIR}/lib/libicui18n.so.56"
      "${QTDIR}/lib/libicui18n.so.56.1"
      "${QTDIR}/lib/libicuuc.so.56"
      "${QTDIR}/lib/libicuuc.so.56.1"
      "${QTDIR}/lib/libicudata.so.56"
      "${QTDIR}/lib/libicudata.so.56.1"
      DESTINATION "${__LIB_DIR}")
    file(COPY
      "${QTDIR}/plugins/imageformats"
      "${QTDIR}/plugins/platforms"
      "${QTDIR}/plugins/printsupport"
      DESTINATION "${__BIN_DIR}")
  endif()
endfunction()

###################################
# Bootloader Host Tool CLI
add_executable(${CLI_TARGET} MACOSX_BUNDLE ${BACKEND_SOURCES} ${BOOTLOADER_SOURCES} ${CLI_SOURCES} ${GUI_RESOURCES})
target_include_directories(${CLI_TARGET} PRIVATE
    ${Boost_INCLUDE_DIRS} # macro provided by CMake for Boost dependencies
    "${CyBridge_INCLUDE_DIR}"
    "include/backend"
    "include/backend/cybootloaderutils"
    "include/cli"
)
target_link_libraries(${CLI_TARGET} ${CyBridge_LIBRARY} Qt5::Widgets Qt5::SerialPort) # Widgets for QApplication so we can prove we're not a GUI
set_target_properties("${CLI_TARGET}" PROPERTIES FOLDER "${PROJECT_NAME}")
cy_deploy_qt(${CLI_TARGET})

###################################
# Bootloader Host Tool GUI
add_executable(${GUI_TARGET} WIN32 MACOSX_BUNDLE ${BACKEND_SOURCES} ${BOOTLOADER_SOURCES} ${GUI_SOURCES} ${GUI_RESOURCES})
target_include_directories(${GUI_TARGET} PRIVATE
    ${Boost_INCLUDE_DIRS} # macro provided by CMake for Boost dependencies
    "${CyBridge_INCLUDE_DIR}"
    "include/backend"
    "include/backend/cybootloaderutils"
    "include/gui"
)
target_link_libraries(${GUI_TARGET} ${CyBridge_LIBRARY} Qt5::SerialPort Qt5::Gui Qt5::Widgets)
cy_deploy_qt(${GUI_TARGET})

add_custom_target("${GUI_TARGET}_ALL" DEPENDS "${GUI_TARGET}" "${CLI_TARGET}")
set_target_properties("${GUI_TARGET}_ALL" PROPERTIES FOLDER "${PROJECT_NAME}")
