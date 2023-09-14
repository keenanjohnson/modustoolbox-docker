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

# Do not print "Up-to-date" messages when installing
set(CMAKE_INSTALL_MESSAGE LAZY)

# Check if MTB_DEPLOY_DIR environment variable was set by the build script
if(DEFINED ENV{MTB_DEPLOY_DIR})
  get_filename_component(MTB_DEPLOY_DIR "$ENV{MTB_DEPLOY_DIR}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
elseif(NOT "${MTB_DEPLOY_DIR}")
  set(MTB_DEPLOY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deploy")
endif()
set(MTB_DEPLOY_DIR "${MTB_DEPLOY_DIR}" CACHE PATH "Target directory for ZIP package deployment." FORCE)

# Install executable and strip debug symbols
function(mtb_install_binary tool_name target_name binary_dir)
  if(UNIX)
    install(TARGETS ${target_name}
      DESTINATION ${tool_name}/${binary_dir}
      COMPONENT ${tool_name}
    )
  endif()
endfunction()

# Create CMake target for tool packaging, include in "install" target
function(mtb_package_tool tool_name)
  # Expand PROJECT_VERSION_* variables in version.xml.in template
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/templates/version.xml.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.xml"
  )

  # Install version.xml with read-only permissions
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/version.xml"
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    DESTINATION ${tool_name}
    COMPONENT ${tool_name}
  )

  # Install props.json
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/props.json.in")
    # Expand PROJECT_VERSION_* variables in props.json.in template
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/props.json.in"
      "${CMAKE_CURRENT_BINARY_DIR}/props.json"
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/props.json"
      DESTINATION ${tool_name}
      COMPONENT ${tool_name}
    )
  elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/props.json")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/props.json"
      DESTINATION ${tool_name}
      COMPONENT ${tool_name}
    )
  endif()

  # Copy configurator.xml to the tool directory
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/templates/configurator.xml.in")
    # Expand PROJECT_VERSION_* variables in configurator.xml.in template
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/templates/configurator.xml.in"
      "${CMAKE_CURRENT_BINARY_DIR}/configurator.xml"
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/configurator.xml"
      DESTINATION ${tool_name}
      COMPONENT ${tool_name}
    )
  endif()

  # Iterate over all target names passed to this function
  # This is necessary to support packaging of multiple binaries with a single tool
  # For example: dfuh-tool GUI + CLI
  foreach(target_name ${ARGV})
    if(WIN32)
      install(DIRECTORY "$<TARGET_FILE_DIR:${target_name}>/"
        USE_SOURCE_PERMISSIONS
        DESTINATION ${tool_name}
        COMPONENT ${tool_name}
        PATTERN "*.exp" EXCLUDE
        PATTERN "*.ilk" EXCLUDE
        PATTERN "*.lib" EXCLUDE
        PATTERN "*.pdb" EXCLUDE
      )
    elseif(APPLE)
      get_target_property(_is_macos_bundle ${target_name} MACOSX_BUNDLE)
      get_target_property(_main_app ${target_name} MAIN_APP)
      if(${_is_macos_bundle})
        set(BUNDLE_NAME ${target_name})
        install(TARGETS ${target_name} BUNDLE
          DESTINATION ${tool_name}
          COMPONENT ${tool_name}
        )

        # MacOS can run app from the translocated temp folder that does not contain version.xml.
        # Add version.xml to Contents/Resources folder, as expected by CyVersion::loadFromVersionFile
        # ModusToolbox 3.0: version.xml is replaced by props.json
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/version.xml"
          DESTINATION "${tool_name}/${BUNDLE_NAME}.app/Contents/Resources"
          COMPONENT ${tool_name}
        )
        if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/props.json")
          install(FILES "${CMAKE_CURRENT_BINARY_DIR}/props.json"
            DESTINATION "${tool_name}/${BUNDLE_NAME}.app/Contents/Resources"
            COMPONENT ${tool_name}
          )
        elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/props.json")
          install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/props.json"
            DESTINATION "${tool_name}/${BUNDLE_NAME}.app/Contents/Resources"
            COMPONENT ${tool_name}
          )
        endif()
      elseif(TARGET ${_main_app})
        set(BUNDLE_NAME ${_main_app})
        mtb_install_binary(${tool_name} ${target_name} "${BUNDLE_NAME}.app/Contents/macOS")
      else()
        mtb_install_binary(${tool_name} ${target_name} "bin")
        install(DIRECTORY "$<TARGET_FILE_DIR:${target_name}>/../Frameworks"
          USE_SOURCE_PERMISSIONS
          DESTINATION ${tool_name}
          COMPONENT ${tool_name}
        )
        install(DIRECTORY "$<TARGET_FILE_DIR:${target_name}>/../PlugIns"
          USE_SOURCE_PERMISSIONS
          DESTINATION ${tool_name}
          COMPONENT ${tool_name}
          OPTIONAL
        )
      endif()

      # Install launcher script
      set(TARGET_NAME ${target_name})
      if(DEFINED BUNDLE_NAME)
        configure_file(
          "${CMAKE_CURRENT_SOURCE_DIR}/templates/launch-app.sh.in"
          "${CMAKE_CURRENT_BINARY_DIR}/${target_name}"
        )
      else()
        configure_file(
          "${CMAKE_CURRENT_SOURCE_DIR}/templates/launch-bin.sh.in"
          "${CMAKE_CURRENT_BINARY_DIR}/${target_name}"
        )
      endif()
      install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${target_name}"
        DESTINATION ${tool_name}
        COMPONENT ${tool_name}
      )
      unset(TARGET_NAME)
      unset(BUNDLE_NAME)
    else() # Linux
      # Install the target executable
      mtb_install_binary(${tool_name} ${target_name} "bin")
      # Install the shared libraries
      install(DIRECTORY "$<TARGET_FILE_DIR:${target_name}>/../lib/"
        USE_SOURCE_PERMISSIONS
        DESTINATION ${tool_name}/lib
        COMPONENT ${tool_name}
        # Exclude lib/test sub-directory (project-creator case)
        PATTERN "test" EXCLUDE
      )
      # Install the Qt platform plugins
      install(DIRECTORY "$<TARGET_FILE_DIR:${target_name}>/"
        USE_SOURCE_PERMISSIONS
        DESTINATION ${tool_name}/bin
        COMPONENT ${tool_name}
        FILES_MATCHING
          PATTERN "*.so"
          PATTERN "*.xml" # match CyBridge devdb.xml
      )
      # Install data files
      install(DIRECTORY "$<TARGET_FILE_DIR:${target_name}>/../share/"
        USE_SOURCE_PERMISSIONS
        DESTINATION ${tool_name}/share
        COMPONENT ${tool_name}
        OPTIONAL
      )

      # Install launcher script
      set(TARGET_NAME ${target_name})
      configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/templates/launch-bin.sh.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${target_name}"
      )
      install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${target_name}"
        DESTINATION ${tool_name}
        COMPONENT ${tool_name}
      )
      unset(TARGET_NAME)
    endif()

    # Install the configurator PNG file in case it exists in the build dir
    # (for tools with ICON_BASEPATH configured by mtb_set_target_properties)
    if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.png")
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.png"
        DESTINATION ${tool_name}
        COMPONENT ${tool_name}
      )
    endif()
  endforeach()

  # ${tool_name}_PACKAGE executes packaging for a given tool component
  add_custom_target(${tool_name}_PACKAGE
    DEPENDS ${ARGV}
    COMMAND "${CMAKE_COMMAND}" -DBUILD_TYPE=$<CONFIG> -DCOMPONENT=${tool_name}
                               -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake"
  )
endfunction()
