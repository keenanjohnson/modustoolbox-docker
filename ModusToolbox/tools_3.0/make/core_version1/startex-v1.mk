################################################################################
# \file startex.mk
#
# \brief
# This file is used as the starting point for an application build.
#
################################################################################
# \copyright
# Copyright 2018-2021 Cypress Semiconductor Corporation
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
################################################################################

ifeq ($(WHICHFILE),true)
$(info Processing $(lastword $(MAKEFILE_LIST)))
endif


################################################################################
# Globals
################################################################################

# Set the default shell to bash
SHELL=/bin/bash
CY_FIND=/usr/bin/find

# Define flow version for the app
FLOW_VERSION=$(if $(strip $(CY_GETLIBS_SHARED_PATH)),2,1)

# Platform-specific .modustoolbox location
ifeq ($(OS),Windows_NT)
CY_DOTMODUSTOOLBOX_PATH=$(subst \,/,$(USERPROFILE))/.modustoolbox
else
CY_DOTMODUSTOOLBOX_PATH=$(HOME)/.modustoolbox
endif

# Global search depth
CY_UTILS_SEARCH_DEPTH?=8

# Default library search depth
CY_LIBS_SEARCH_DEPTH?=8

# Wildcard depth for files
CY_WILDCARDS_1:=*
CY_WILDCARDS_2:=* */*
CY_WILDCARDS_3:=* */* */*/*
CY_WILDCARDS_4:=* */* */*/* */*/*/*
CY_WILDCARDS_5:=* */* */*/* */*/*/* */*/*/*/*
CY_WILDCARDS_6:=* */* */*/* */*/*/* */*/*/*/* */*/*/*/*/*
CY_WILDCARDS_7:=* */* */*/* */*/*/* */*/*/*/* */*/*/*/*/* */*/*/*/*/*/*
CY_WILDCARDS_8:=* */* */*/* */*/*/* */*/*/*/* */*/*/*/*/* */*/*/*/*/*/* */*/*/*/*/*/*/*
CY_WILDCARDS_9:=* */* */*/* */*/*/* */*/*/*/* */*/*/*/*/* */*/*/*/*/*/* */*/*/*/*/*/*/* */*/*/*/*/*/*/*/*

# Wildcard depth for hidden files
CY_WILDCARDS_HIDDEN_1:=/
CY_WILDCARDS_HIDDEN_2:=/ */
CY_WILDCARDS_HIDDEN_3:=/ */ */*/
CY_WILDCARDS_HIDDEN_4:=/ */ */*/ */*/*/
CY_WILDCARDS_HIDDEN_5:=/ */ */*/ */*/*/ */*/*/*/
CY_WILDCARDS_HIDDEN_6:=/ */ */*/ */*/*/ */*/*/*/ */*/*/*/*/
CY_WILDCARDS_HIDDEN_7:=/ */ */*/ */*/*/ */*/*/*/ */*/*/*/*/ */*/*/*/*/*/
CY_WILDCARDS_HIDDEN_8:=/ */ */*/ */*/*/ */*/*/*/ */*/*/*/*/ */*/*/*/*/*/ */*/*/*/*/*/*/
CY_WILDCARDS_HIDDEN_9:=/ */ */*/ */*/*/ */*/*/*/ */*/*/*/*/ */*/*/*/*/*/ */*/*/*/*/*/*/ */*/*/*/*/*/*/*/


################################################################################
# Macros
################################################################################

#
# Applies wildcards for files in specified directory levels
# $(1) : File
# $(2) : Base path
# $(3) : Wildcard pattern
#
CY_MACRO_WILDCARD=$(wildcard $(2)/$(3)$(1))

#
# Searches for files in each of the directory levels
# $(1) : File
# $(2) : Base path
#
CY_MACRO_SEARCH=$(foreach level,$(CY_WILDCARDS_$(CY_UTILS_SEARCH_DEPTH)),\
				$(call CY_MACRO_WILDCARD,$(1),$(2),$(level)))

#
# Searches for files in each of the directory levels
# $(1) : File
# $(2) : Base path
#
CY_MACRO_SEARCH_HIDDEN=$(call CY_MACRO_REMOVE_DOUBLESLASH,\
				$(foreach level,$(CY_WILDCARDS_HIDDEN_$(CY_UTILS_SEARCH_DEPTH)),\
				$(call CY_MACRO_WILDCARD,$(1),$(2),$(level))))

#
# Removes double slashes in paths
# $(1) : Path
#
CY_MACRO_REMOVE_DOUBLESLASH=$(subst //,/,$(1))

#
# Removes trailing slashes of path and then returns the path
# $(1) : Path
#
CY_MACRO_DIR=$(patsubst %/,%,$(dir $(patsubst %/,%,$(filter-out .,$(1)))))

#
# Detects whitespace in variables
# $(1) : Variable
#
CY_MACRO_WHITESPACE=$(if $(filter 1,$(words $($(1)))),,$(error Error: $(1) contains whitespace: "$($(1))"))


################################################################################
# Paths
################################################################################

#
# Include SEARCH variables
#
-include $(CY_INTERNAL_APPLOC)/libs/mtb.mk

#
# If the path to the BASELIB does not exist, set it to empty assuming the BSP will fill it in
#
ifneq ($(CY_BASELIB_PATH),)
ifeq ($(wildcard $(CY_BASELIB_PATH)),)
$(info INFO: The path '$(CY_BASELIB_PATH)' set for CY_BASELIB_PATH does not exist. Attempting to use BSP provided path instead)
CY_BASELIB_PATH=
endif
endif

#
# Search for TARGET.mk. It will define CY_BASELIB_PATH
#
ifeq ($(CY_BASELIB_PATH),)
# Few exceptions such as legacy wiced_sdk that don't define the TARGET
ifneq ($(TARGET),)

CY_TARGET_UNDERSCORE=$(subst -,_,$(TARGET))

ifneq ($(SEARCH_TARGET_$(TARGET)),)
CY_TARGET_MAKEFILE_UNDERSCORE_SEARCH:=$(call CY_MACRO_SEARCH,$(CY_TARGET_UNDERSCORE).mk,$(SEARCH_TARGET_$(TARGET)))
CY_TARGET_MAKEFILE_SEARCH:=$(call CY_MACRO_SEARCH,$(TARGET).mk,$(SEARCH_TARGET_$(TARGET)))
else
CY_TARGET_MAKEFILE_UNDERSCORE_SEARCH:=$(call CY_MACRO_SEARCH,$(CY_TARGET_UNDERSCORE).mk,$(CY_INTERNAL_APP_PATH))\
                    $(if $(CY_INTERNAL_EXTAPP_PATH),$(call CY_MACRO_SEARCH,$(CY_TARGET_UNDERSCORE).mk,$(CY_INTERNAL_EXTAPP_PATH)))\
                    $(if $(SEARCH_LIBS_AND_INCLUDES),$(foreach d,$(SEARCH_LIBS_AND_INCLUDES),$(call CY_MACRO_SEARCH,$(CY_TARGET_UNDERSCORE).mk,$(d))))\
					$(if $(SEARCH),$(foreach d,$(SEARCH),$(call CY_MACRO_SEARCH,$(CY_TARGET_UNDERSCORE).mk,$(d))))
CY_TARGET_MAKEFILE_SEARCH:=$(call CY_MACRO_SEARCH,$(TARGET).mk,$(CY_INTERNAL_APP_PATH))\
                    $(if $(CY_INTERNAL_EXTAPP_PATH),$(call CY_MACRO_SEARCH,$(TARGET).mk,$(CY_INTERNAL_EXTAPP_PATH)))\
                    $(if $(SEARCH_LIBS_AND_INCLUDES),$(foreach d,$(SEARCH_LIBS_AND_INCLUDES),$(call CY_MACRO_SEARCH,$(TARGET).mk,$(d))))\
					$(if $(SEARCH),$(foreach d,$(SEARCH),$(call CY_MACRO_SEARCH,$(TARGET).mk,$(d))))
endif

CY_TARGET_MAKEFILE=$(sort $(CY_TARGET_MAKEFILE_SEARCH) $(CY_TARGET_MAKEFILE_UNDERSCORE_SEARCH))

ifneq ($(words $(CY_TARGET_MAKEFILE)),0)
# Note: It's ok if TARGET.mk is not found since getlibs needs to be able to run without it
ifneq ($(words $(CY_TARGET_MAKEFILE)),1)
$(error Found multiple identical targets:$(CY_TARGET_MAKEFILE))
else
CY_TARGET_AVAILABLE=$(CY_TARGET_MAKEFILE)
CY_TARGET_DIR=$(call CY_MACRO_DIR,$(CY_TARGET_MAKEFILE))
CY_TARGET_GEN_DIR=$(CY_INTERNAL_APP_PATH)/TARGET_$(TARGET_GEN)
CY_TARGET_SKIP=true
include $(CY_TARGET_MAKEFILE)
$(info $(TARGET).mk: $(CY_TARGET_MAKEFILE))
endif
endif

endif # ifneq ($(TARGET),)
endif # ifeq ($(CY_BASELIB_PATH),)

CY_INTERNAL_BASELIB_PATH:=$(patsubst %/,%,$(CY_BASELIB_PATH))
CY_BASELIB_CORE_PATH?=$(CY_INTERNAL_BASELIB_PATH)

#
# Get a file or python script path based on its file name
# $(1) : A patch system CY_TOOL_... variable that points to the path if it's provided by the tools installer for the current platform.
# $(2) : (Optional/Fallback) Name of executable to look for in PATH if $(1) doesn't exist on the current platform.
# $(3) : (Optional/Fallback) Name of Python script to look for in CY_PYTHON_PATH.
#
CY_GET_FILE_PATH=$(strip $(if $(1),$(call CY_SEARCH_FOR_FILE_EVERYWHERE,$(1),$(2),$(3)),$(call CY_SEARCH_FOR_FILE_ENVIRONMENT,$(2),$(3))))

CY_SEARCH_FOR_FILE_EVERYWHERE=$(if $(wildcard $(CY_TOOLS_DIR)/$($(1))),$(CY_TOOLS_DIR)/$($(1)),$(call CY_SEARCH_FOR_FILE_ENVIRONMENT,$(2),$(3)))

CY_SEARCH_FOR_FILE_ENVIRONMENT=$(if $(1),$(if $(shell type -P $(1)),$(shell type -P $(1)),$(call CY_SEARCH_FOR_PY_SCRIPT,$(2))),$(call CY_SEARCH_FOR_PY_SCRIPT,$(2)))

CY_SEARCH_FOR_PY_SCRIPT=$(if $(1),\
					$(if $(filter $(OS),Windows_NT),\
					$(if $(findstring $(CY_TOOLS_DIR),$(dir $(dir $(CY_PYTHON_PATH)))),\
					$(dir $(CY_PYTHON_PATH))Scripts/$(1),\
					$(shell type -P $(1))),\
					$(shell type -P $(1))))

################################################################################
# Search cyignore files
################################################################################

# Search for .cyignore files
CY_CYIGNORE_FILES:=$(call CY_MACRO_SEARCH_HIDDEN,.cyignore,$(CY_INTERNAL_APP_PATH))\
					$(if $(CY_INTERNAL_EXTAPP_PATH),$(call CY_MACRO_SEARCH_HIDDEN,.cyignore,$(CY_INTERNAL_EXTAPP_PATH)))\
					$(if $(SEARCH),$(foreach d,$(SEARCH),$(call CY_MACRO_SEARCH_HIDDEN,.cyignore,$(d))))

# Ignore the build dir (if in app dir) and the dir and files listed in the .cyignore files
# Note: $$CY_APP_PATH should be kept for BWC
# Note: Do not prepend path if the entry begins with $( as that means it begins with a make variable
CY_IGNORE_MAKEVAR_PATTERN:=$$(
CY_IGNORE_DIRS:=$(patsubst %/,%,$(CY_IGNORE) $(CY_RELATIVE_BUILD_DIR) \
					$(foreach d,$(CY_CYIGNORE_FILES),\
						$(foreach entry,$(shell cat $(d) | grep -v '^\#' ),\
							$(if $(findstring $$CY_APP_PATH,$(entry)),\
							$(call CY_MACRO_REMOVE_DOUBLESLASH,$(subst $$CY_APP_PATH,$(CY_INTERNAL_APP_PATH),$(entry))),\
							$(if $(filter $(CY_IGNORE_MAKEVAR_PATTERN)%,$(entry)),$(entry),$(call CY_MACRO_DIR,$(d))/$(entry)))\
						)\
					)\
				)

# Need to expand out any make variable placed in .cyignore file
# Note: Use get_app_info to print this
$(eval CY_IGNORE_DIRS:=$(CY_IGNORE_DIRS))

################################################################################
# Includes
################################################################################

# Must come after cyignore search
include $(CY_TOOL_make_BASE)/core_version1/legacy-v1.mk

# Control build using baselib versus not including the baselib
CY_INCLUDE_BASELIB=true
ifneq ($(CY_SKIP_RECIPE),)
CY_INCLUDE_BASELIB=false
endif
ifeq ($(MAKECMDGOALS),)
CY_INCLUDE_BASELIB=false
endif

# Control build using getlibs versus not including the getlibs
CY_INCLUDE_GETLIBS=false
ifeq ($(filter getlibs,$(MAKECMDGOALS)),getlibs)
CY_INCLUDE_GETLIBS=true
CY_INCLUDE_BASELIB=false
# Check that getlibs is called without other targets
ifneq ($(filter-out getlibs,$(MAKECMDGOALS)),)
$(error Detected "$(MAKECMDGOALS)" make targets.\
The getlibs target must not be combined with other make targets)
endif
endif

ifeq ($(filter printlibs,$(MAKECMDGOALS)),printlibs)
CY_INCLUDE_GETLIBS=true
CY_INCLUDE_BASELIB=false
endif

# get_app_info needs CY_INTERNAL_GETLIBS_PATH
ifeq ($(filter get_app_info,$(MAKECMDGOALS)),get_app_info)
CY_INCLUDE_GETLIBS=true
endif

# import_deps needs CY_INTERNAL_GETLIBS_DEPS_PATH
ifeq ($(filter import_deps,$(MAKECMDGOALS)),import_deps)
CY_INCLUDE_GETLIBS=true
CY_INCLUDE_BASELIB=false
endif

# Define behavior if baselib is not found on disk
ifeq ($(wildcard $(CY_BASELIB_CORE_PATH)),)
ifeq ($(filter get_app_info,$(MAKECMDGOALS)),get_app_info)
CY_INCLUDE_BASELIB=false
# Needed for library-manager-cli when processing .mtb files
get_app_info:
	@:
	$(info )
	$(info "WARNING: Build support not available")
	$(info )
	$(info =======================================)
	$(info IDE variables)
	$(info =======================================)
	$(info CY_PROTOCOL=$(CY_PROTOCOL))
	$(info APP_NAME=$(APPNAME))
	$(info LIB_NAME=$(LIBNAME))
	$(info TARGET=$(TARGET))
	$(info TARGET_DEVICE=$(DEVICE))
	$(info CONFIGURATOR_FILES=$(CY_CONFIG_FILES))
	$(info SUPPORTED_TOOL_TYPES=$(CY_OPEN_FILTERED_SUPPORTED_TYPES))
	$(info CY_TOOLS_PATH=$(CY_TOOLS_DIR))
	$(info CY_GETLIBS_OFFLINE=$(CY_GETLIBS_OFFLINE))
	$(info CY_GETLIBS_PATH=$(CY_INTERNAL_GETLIBS_PATH))
	$(info CY_GETLIBS_DEPS_PATH=$(CY_INTERNAL_GETLIBS_DEPS_PATH))
	$(info CY_GETLIBS_CACHE_PATH=$(CY_INTERNAL_GETLIBS_CACHE_PATH))
	$(info CY_GETLIBS_OFFLINE_PATH=$(CY_INTERNAL_GETLIBS_OFFLINE_PATH))
	$(info SHAREDLIBS_ROOT=$(CY_SHARED_PATH))
	$(info SHAREDLIBS=$(SEARCH_LIBS_AND_INCLUDES))
	$(info SHAREDLIBS_FILES=$(CY_SHARED_USED_LIB_FILES))
	$(info CY_DEPENDENT_PROJECTS=$(CY_DEPENDENT_PROJECTS))
	$(info CY_GETLIBS_SHARED_PATH=$(CY_GETLIBS_SHARED_PATH))
	$(info CY_GETLIBS_SHARED_NAME=$(CY_GETLIBS_SHARED_NAME))
	$(info SEARCH=$(SEARCH))
	$(info FLOW_VERSION=$(FLOW_VERSION))
else
ifeq ($(CY_INCLUDE_BASELIB),true)
$(error Build support for the target device not found. Run "make getlibs" to ensure all required build and code dependencies are present.)
endif
endif
endif

#
# Getlibs variables
# Note: Used outside of getlibs.mk
# Note: CY_GETLIBS_SHARED_PATH must be set in app makefile to use flow v2
#
CY_GETLIBS_DEPS_PATH?=$(CY_APP_LOCATION)/deps
CY_GETLIBS_PATH?=$(CY_APP_LOCATION)/libs
ifeq ($(CY_GETLIBS_PATH),$(CY_APP_LOCATION)/libs)
CY_GETLIBS_SEARCH_PATH?=$(CY_APP_LOCATION)
else
CY_GETLIBS_SEARCH_PATH?=$(CY_INTERNAL_APP_PATH)
endif
CY_GETLIBS_CACHE_PATH?=$(CY_DOTMODUSTOOLBOX_PATH)/cache
CY_GETLIBS_OFFLINE_PATH?=$(CY_DOTMODUSTOOLBOX_PATH)/offline
CY_GETLIBS_SHARED_NAME?=mtb_shared
ifneq ($(CY_GETLIBS_SHARED_PATH),)
CY_GETLIBS_SHARED=$(patsubst %/,%,$(CY_GETLIBS_SHARED_PATH))/$(CY_GETLIBS_SHARED_NAME)
endif

#
# Check paths for whitespace issues
#
CY_WHITESPACE_VARS_LIST=CURDIR \
	CY_APP_PATH CY_BASELIB_PATH CY_BUILD_LOCATION CY_DEVICESUPPORT_PATH CY_EXTAPP_PATH \
	CY_GETLIBS_CACHE_PATH CY_GETLIBS_DEPS_PATH CY_GETLIBS_OFFLINE_PATH CY_GETLIBS_PATH CY_GETLIBS_SEARCH_PATH \
	CY_SHARED_PATH CY_TOOLS_DIR TOOLCHAIN_MK_PATH

$(foreach var,$(CY_WHITESPACE_VARS_LIST),$(if $($(var)),$(call CY_MACRO_WHITESPACE,$(var))))

ifeq ($(CY_INCLUDE_GETLIBS),true)
include $(CY_TOOL_make_BASE)/core_version1/getlibs-v1.mk
endif

ifeq ($(CY_INCLUDE_BASELIB),true)
include $(CY_BASELIB_CORE_PATH)/make/core/main.mk
endif

#
# Print deprecated tools message if any of them were used.
# Note: this does not work for variable expansion in rules. Even though this is not possible,
# chances are high that at least one of these variables will get expanded in the first phase.
#
ifneq ($(CY_MESSAGE_TOOLS),)
$(info )
$(info $(CY_MESSAGE_TOOLS))
endif


################################################################################
# Targets
################################################################################

# Note: Must be available in tools-make as it needs to be call-able from project-creator
import_deps:
ifneq ($(FLOW_VERSION),1)
	$(if $(IMPORT_PATH),,$(info )$(error IMPORT_PATH variable must be specified to import dependencies))
	$(if $(wildcard $(IMPORT_PATH)),,$(info )$(error "$(IMPORT_PATH)" directory does not exist))
	$(info )
	$(info Importing dependencies from "$(IMPORT_PATH)"...)
	@mtbxFiles=$$($(CY_FIND) $(IMPORT_PATH) -type f \( -name "*.mtbx" \));\
	for mtbxFile in $$mtbxFiles; do \
		mtbFile="$${mtbxFile%.mtbx}.mtb";\
		mtbFile="$${mtbFile##*/}";\
		if [[ -f $(CY_INTERNAL_GETLIBS_DEPS_PATH)/"$$mtbFile" ]]; then \
			if [[ $$(diff -w "$$mtbxFile" "$(CY_INTERNAL_GETLIBS_DEPS_PATH)/"$$mtbFile"") ]]; then \
				echo "Importing \""$$mtbxFile"\" conflicts with \"$(CY_INTERNAL_GETLIBS_DEPS_PATH)/"$$mtbFile"\".";\
				echo "    Keeping the existing \"$(CY_INTERNAL_GETLIBS_DEPS_PATH)/"$$mtbFile"\" file.";\
				echo "    To manually import the file, replace the application's .mtb file with the library's .mtbx file;";\
				continue;\
			fi;\
		fi;\
		cp "$$mtbxFile" $(CY_INTERNAL_GETLIBS_DEPS_PATH)/"$$mtbFile";\
		mtbFiles+=($$mtbFile);\
	done;\
	if [[ $$mtbxFiles ]]; then\
		if [[ $$mtbFiles ]]; then \
			echo "Dependent .mtb files created in \"$(CY_INTERNAL_GETLIBS_DEPS_PATH)\"";\
			printf '  %s\n' "$${mtbFiles[@]}";\
			echo;\
		fi;\
		echo "Import complete";\
		echo;\
	else\
		echo "Could not locate any .mtbx dependency files."; \
		echo "Check that the IMPORT_PATH variable contains the correct path.";\
		echo;\
		exit 1;\
	fi
else
	@:
	$(info $(MTB_NEWLINE)This application is using .lib files. All libraries and .lib files must be in the\
	application directory. These are automatically included and do not need to be imported.$(MTB_NEWLINE))
endif

.PHONY: getlibs printlibs vscode eclipse get_app_info build program import_deps help_start
