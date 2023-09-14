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

# Suppress build system regeneration in the IDE.
# To reconfigure CMake projects when new source files are added,
# developer should run build.sh or build.bat in the terminal.
set(CMAKE_SUPPRESS_REGENERATION ON)

# Automatically add the current source and build directories to the include path
# This property is enabled globally, cannot be set at a target level
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set common properties for CMake target
function(mtb_set_target_properties target_name)
  # Parse the optional arguments
  cmake_parse_arguments(TARGET
    # Boolean options
    "NO_WERROR;NO_QT;ALLOW_QT_ASCII_CAST;QT_NO_DEBUG_OUTPUT"
    # Single-value keywords
    "MAIN_APP;CXX_STANDARD;DISPLAY_NAME;ICON_RESOURCE;PACKAGE_DOC"
    # Multi-value keywords
    ""
    ${ARGN}
  )

  if(DEFINED TARGET_MAIN_APP AND TARGET ${TARGET_MAIN_APP})
    # Main GUI depends on CLI target
    add_dependencies(${TARGET_MAIN_APP} ${target_name})
    set_target_properties(${target_name} PROPERTIES
      MAIN_APP ${TARGET_MAIN_APP}
    )

    # macOS: Copy tool-cli to tool.app/Contents/MacOS directory.
    # It is important to copy the CLI binary to GUI
    # runtime directory as part of GUI target post-build command,
    # so that DeployLibrary.cmake always tweaks the CLI RPATH.
    #
    # Correct ordering of post-build commands:
    # - build dfuh-cli
    # - tweak dfuh-cli RPATH with install_name_tool
    # - build dfuh-tool
    # - copy dfuh-cli to dfuh-tool.app
    # - tweak dfuh-tool RPATH with install_name_tool
    if(APPLE)
      set(_main_app_dir "$<TARGET_FILE_DIR:${TARGET_MAIN_APP}>")
      add_custom_command(TARGET ${TARGET_MAIN_APP} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${_main_app_dir}"
        COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${target_name}>" "${_main_app_dir}"
      )
      unset(_main_app_dir)
    endif()
  endif()


  # By default, C++17 standard is set for all targets
  if(NOT DEFINED TARGET_CXX_STANDARD)
    set(TARGET_CXX_STANDARD 17)
  endif()

  set_target_properties(${target_name} PROPERTIES
    # Set C++ language standard
    CXX_STANDARD ${TARGET_CXX_STANDARD}
  )

  mtb_get_project_version_short(_target_version_short)
  mtb_get_project_version(_target_version)

  set_target_properties(${target_name} PROPERTIES
    # Enable common options required for Qt-enabled targets
    # https://cmake.org/cmake/help/latest/manual/cmake-qt.7.html
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
  )

  # Force string literals to require wrapping by tr() or by QLatin1String(),
  # depending on whether it should be translated or not
  target_compile_definitions(${target_name} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
  )

  # Disable macros that conflict with other names
  if(WIN32)
    target_compile_definitions(${target_name} PUBLIC
      NOMINMAX
      NOGDI
    )
  endif()

  # Configure compiler warning level, treat all warnings as errors
  if(MSVC)
    target_compile_definitions(${target_name} PUBLIC
      _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
      _SCL_SECURE_NO_WARNINGS
      _CRT_SECURE_NO_WARNINGS
      # Prevent windows.h from including winsock.h, as Thrift use winsock2.h
      WIN32_LEAN_AND_MEAN
    )
    target_compile_options(${target_name} PUBLIC
      # By default MSVC always sets __cplusplus to 199711L.
      # https://blogs.msdn.microsoft.com/vcblog/2018/04/09/msvc-now-correctly-reports-__cplusplus/
      /Zc:__cplusplus
      /diagnostics:caret
      /MP /W3 /w34189 /utf-8 /FS
      /w34062  # C4062: enumerator 'identifier' in switch of enum 'enumeration' is not handled
      /w34189  # C4189: 'identifier': local variable is initialized but not referenced
      /w34254  # C4254: 'operator' : conversion from 'type1' to 'type2', possible loss of data
      /w34266  # C4266: 'virtual_function': no override available for virtual member function from base 'classname'; function is hidden
      /w34355  # C4344: 'this': used in base member initializer list
      /w34596  # C4596: 'name': illegal qualified name in member declaration
      /w34701  # C4701: Potentially uninitialized local variable 'name' used
      /w34800  # C4800: Implicit conversion from 'type' to bool. Possible information loss
      /w35038  # C5038: data member 'member1' will be initialized after data member 'member2'
    )
  else() # Linux/macOS
    target_compile_options(${target_name} PUBLIC
      -Wall
    )
  endif()

  if(APPLE)
    # XCode specific settings
    set_target_properties(${target_name} PROPERTIES
      # Get builds working on M1 Mac machines
      OSX_ARCHITECTURES "x86_64"
      # Don't strip symbols, this will be done at packaging step
      XCODE_ATTRIBUTE_COPY_PHASE_STRIP "NO"
      # Disable ad-hoc code signing
      # The binaries are properly signed by mtb_package_tool
      XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
      XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
    )

    # Check if the target is an application bundle
    get_target_property(_is_macos_bundle ${target_name} MACOSX_BUNDLE)
    if(${_is_macos_bundle})
      if(NOT TARGET_DISPLAY_NAME)
        message(FATAL_ERROR "mtb_set_target_properties: DISPLAY_NAME is not set for ${target_name}")
      endif()

      # Properties set in MacOSX Info.plist:
      # CFBundleName - Application name in QMenuBar (MODUS-1853)
      # CFBundleVersion - Application version (1.0.0)
      # CFBundleShortVersionString - Application short version (1.0.0)
      # CFBundleLongVersionString - Application long version (1.0.0.0)
      set_target_properties(${target_name} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/templates/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "${TARGET_DISPLAY_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${_target_version_short}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${_target_version_short}"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${_target_version}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.${target_name}"
      )
      unset(_short_ver)
      unset(_long_ver)
    endif()
    unset(_is_macos_bundle)

    # Set RUNPATH to lookup the shared libraries in ../Frameworks directory
    set_target_properties(${target_name} PROPERTIES
      INSTALL_RPATH "@executable_path/../Frameworks"
      BUILD_WITH_INSTALL_RPATH ON
    )
  elseif(UNIX) # Linux
    # Set RUNPATH to lookup the shared libraries in ../lib directory
    set_target_properties(${target_name} PROPERTIES
      INSTALL_RPATH "\$ORIGIN/../lib"
      BUILD_WITH_INSTALL_RPATH ON
    )
  endif()

  # All GUI targets should have icon configured (*.png, *.ico, *.icns)
  if(DEFINED TARGET_ICON_RESOURCE)
    if(WIN32)
      # Variables YEAR and TOOL_ICON are used in RC template
      string(TIMESTAMP YEAR "%Y")
      set(TOOL_ICON "${TARGET_ICON_RESOURCE}.ico")
      set(_target_rc "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.rc")
      if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TOOL_ICON}")
        message(FATAL_ERROR "Windows icon ${TOOL_ICON} cannot be found")
      endif()
      configure_file("${CMAKE_CURRENT_SOURCE_DIR}/templates/tool.rc.in" "${_target_rc}")
      # Add Windows resource file (RC) to target sources
      target_sources(${target_name} PRIVATE "${_target_rc}")
      unset(_target_rc)
      unset(TOOL_ICON)
      unset(YEAR)
    elseif(APPLE)
      # Add MacOS icon file (*.icns) to target sources
      set(_icns_path "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_ICON_RESOURCE}.icns")
      if(NOT EXISTS "${_icns_path}")
        message(FATAL_ERROR "MacOS icon ${_icns_path} cannot be found")
      endif()
      target_sources(${target_name} PRIVATE "${_icns_path}")
      # Get filename without path (resources/images/icon.icns -> icon.icns)
      get_filename_component(_icns_name "${_icns_path}" NAME)
      # Set MacOS app bundle icon file in Info.plist
      set_target_properties(${target_name} PROPERTIES
        RESOURCE "${_icns_path}"
        MACOSX_BUNDLE_ICON_FILE "${_icns_name}"
      )
      unset(_icns_name)
      unset(_target_icns)
    endif()

    # Copy the tool PNG icon to the target build directory
    set(_target_png "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_ICON_RESOURCE}.png")
    if(EXISTS "${_target_png}")
      # The PNG is installed by mtb_package_tool()
      configure_file("${_target_png}" "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.png" COPYONLY)
    endif()
    unset(_target_png)
  endif()
endfunction()
