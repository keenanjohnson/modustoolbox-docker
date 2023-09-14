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

cmake_minimum_required(VERSION 3.21.1)

project(dfuh-tool VERSION 2.0.0)
set(CMAKE_FOLDER ${PROJECT_NAME})

###################################
# Discover cmake directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

###################################
# Load CMake recipe definitions
include(MtbSetTargetProperties)
include(MtbAddBoost)
include(MtbPackageTool)

find_package(CyBridge REQUIRED)

###################################
# Define target names
set(GUI_TARGET ${PROJECT_NAME})
set(CLI_TARGET dfuh-cli)

###################################
# Define source groups
file(GLOB_RECURSE BACKEND_SOURCES
  "src/backend/*.cpp"
  "include/backend/*.h"
)
file(GLOB_RECURSE BOOTLOADER_SOURCES
  "src/backend/cybootloaderutils/*.c"
  "src/backend/cybootloaderutils/*.h"
)
file(GLOB_RECURSE CLI_SOURCES
  "src/cli/*.cpp"
  "include/cli/*.h"
)
file(GLOB_RECURSE GUI_SOURCES
  "src/gui/*.cpp"
  "include/gui/*.h"
)
file(GLOB_RECURSE GUI_RESOURCES
  "resources/*.qrc"
  "include/gui/*.ui"
)

###################################
# DFU Host Tool GUI
add_executable(${GUI_TARGET} WIN32 MACOSX_BUNDLE
  ${BACKEND_SOURCES}
  ${BOOTLOADER_SOURCES}
  ${GUI_SOURCES}
  ${GUI_RESOURCES}
)
target_include_directories(${GUI_TARGET} PRIVATE
  "include/backend"
  "include/backend/cybootloaderutils"
  "include/gui"
)
target_link_libraries(${GUI_TARGET} PRIVATE
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::Network
  Qt5::XmlPatterns
  Qt5::Xml
  Qt5::SerialPort
  Boost::headers
  CyBridge::CyBridge
)
mtb_set_target_properties(${GUI_TARGET}
  DISPLAY_NAME  "Device Firmware Update Tool"
  ICON_RESOURCE "resources/images/dfu"
)

mtb_deploy_cybridge(${GUI_TARGET})
mtb_deploy_qt(${GUI_TARGET})

###################################
# DFU Host Tool CLI
add_executable(${CLI_TARGET}
  ${BACKEND_SOURCES}
  ${BOOTLOADER_SOURCES}
  ${CLI_SOURCES}
  ${GUI_RESOURCES}
)
target_include_directories(${CLI_TARGET} PRIVATE
  "include/backend"
  "include/backend/cybootloaderutils"
  "include/cli"
)
target_link_libraries(${CLI_TARGET} PRIVATE
  Qt5::Core
  Qt5::Widgets
  Qt5::Network
  Qt5::SerialPort
  Boost::headers
  CyBridge::CyBridge
) # Widgets for QApplication so we can prove we're not a GUI
mtb_set_target_properties(${CLI_TARGET}
  MAIN_APP ${GUI_TARGET}
)
mtb_deploy_cybridge(${CLI_TARGET})

mtb_package_tool(${GUI_TARGET} ${CLI_TARGET})

###################################
# Custom targets
add_custom_target(${PROJECT_NAME}_ALL DEPENDS
  ${GUI_TARGET}
  ${CLI_TARGET}
  ${PROJECT_NAME}_PACKAGE
)
add_custom_target(${PROJECT_NAME}_BUILD_ONLY DEPENDS
  ${GUI_TARGET}
  ${CLI_TARGET}
)
