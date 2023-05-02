################################################################################
# \file start.mk
#
# \brief
# This file is used to define the tools locations
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

# Define the default target as the help
.DEFAULT: help_start

help_start:
	@:
	$(info                                                                                    )
	$(info ==============================================================================     )
	$(info Getting started                                                                    )
	$(info ==============================================================================     )
	$(info 1. Run "make getlibs" to import the dependent libraries.                           )
	$(info 2. Run "make build" to build the application. Use -j for parallel builds.          )
	$(info 3. Run "make program" (or "make qprogram") to program a target board.              )
	$(info                                                                                    )
	$(info For more information, run "make help" to print the help documentation.             )
	$(info Note: The help documentation is available after running Step 1.                    )

include $(CY_TOOLS_DIR)/make/core_version3/tools_utils.mk

################################################################################
# Paths
################################################################################

# enable emergency patch support for MTB 3.0 core apps (mtbquery, etc.).
# mtbquery generates CY_TOOL and CY_OPEN entries for all other tools, but we may 
# need to patch mtbquery itself.
-include $(sort $(wildcard ../mtb-patch-*.mk))

#
# Set the location of top-level Makefile dir.
# $(CURDIR) is broken on some msys system, so use this instead
#
_MTB_TOOLS__MAKEFILE_DIR:=$(dir $(abspath $(firstword $(MAKEFILE_LIST))))

#
# Auto discovery root for the project.
#
ifeq ($(CY_APP_PATH),)
MTB_TOOLS__REL_PRJ_PATH:=.
else
MTB_TOOLS__REL_PRJ_PATH:=$(patsubst %/,%,$(CY_APP_PATH))
endif

#
# Calculate path when it's not secondstage (primarily for Windows cygpath optimization)
#
ifneq ($(CY_SECONDSTAGE),true)
#
# Set the build location. Append app dir if CY_BUILD_LOCATION is defined.
# This need to be defined here so that it can be passed as an ignore directory via get_app_info.
#
ifneq ($(CY_BUILD_LOCATION),)
MTB_TOOLS__OUTPUT_BASE_DIR:=$(call mtb__path_normalize,$(CY_BUILD_LOCATION)/$(notdir $(MTB_TOOLS__REL_PRJ_PATH)))
else
MTB_TOOLS__OUTPUT_BASE_DIR:=$(call mtb__path_normalize,$(MTB_TOOLS__REL_PRJ_PATH))/build
endif
export MTB_TOOLS__OUTPUT_BASE_DIR

# The tools directory
MTB_TOOLS__TOOLS_DIR:=$(call mtb__path_normalize,$(CY_TOOLS_DIR))
export MTB_TOOLS__TOOLS_DIR

# This can't be named MTB_TOOLS__PRJ_DIR since MTB_TOOLS__PRJ_DIR can be set from the enviromnent.
# We need to re-export this from each core in case of multi-core.
# For example if we were doing vscode export from the project, we need this variable to reset the enviromnent.
_MTB_TOOLS__ABS_PRJ_SEARCH_ROOT:=$(call mtb__path_normalize,$(_MTB_TOOLS__MAKEFILE_DIR))
export MTB_TOOLS__PRJ_DIR:=$(_MTB_TOOLS__ABS_PRJ_SEARCH_ROOT)
#
# Windows paths
#
ifeq ($(OS),Windows_NT)
MTB_TOOLS__USER_DIR:=$(subst \,/,$(USERPROFILE))/.modustoolbox
#
# CygWin/MSYS
#
ifneq ($(_MTB_TOOLS__WHICH_CYGPATH),)
# Note: Bash only needs to be set once
ifeq ($(MTB_TOOLS__BASH_CMD),)
MTB_TOOLS__BASH_CMD:="$(shell cygpath -m --absolute /usr/bin/bash)" --norc --noprofile
endif
#
# Other Windows environments (not officially supported)
#
else
MTB_TOOLS__BASH_CMD:=bash --norc --noprofile
endif
#
# Linux and macOS paths
#
else
MTB_TOOLS__USER_DIR:=$(HOME)/.modustoolbox
MTB_TOOLS__BASH_CMD:=bash --norc --noprofile
endif
export MTB_TOOLS__USER_DIR
export MTB_TOOLS__BASH_CMD
endif # ifneq ($(CY_SECONDSTAGE),true)

ifeq ($(CY_GETLIBS_CACHE_PATH),)
MTB_TOOLS__CACHE_DIR:=$(MTB_TOOLS__USER_DIR)/git/cache
else
MTB_TOOLS__CACHE_DIR:=$(CY_GETLIBS_CACHE_PATH)
endif

ifeq ($(CY_GETLIBS_OFFLINE_PATH),)
MTB_TOOLS__OFFLINE_DIR:=$(MTB_TOOLS__USER_DIR)/offline
else
MTB_TOOLS__OFFLINE_DIR:=$(CY_GETLIBS_OFFLINE_PATH)
endif

ifeq ($(CY_GETLIBS_GLOBAL_PATH),)
MTB_TOOLS__GLOBAL_DIR:=$(MTB_TOOLS__USER_DIR)/global
else
MTB_TOOLS__GLOBAL_DIR:=$(CY_GETLIBS_GLOBAL_PATH)
endif

CY_GETLIBS_DEPS_PATH?=$(MTB_TOOLS__PRJ_DIR)/deps
CY_GETLIBS_PATH?=$(MTB_TOOLS__PRJ_DIR)/libs

MTB_TOOLS__OUTPUT_TARGET_DIR:=$(MTB_TOOLS__OUTPUT_BASE_DIR)/$(TARGET)
MTB_TOOLS__OUTPUT_CONFIG_DIR:=$(MTB_TOOLS__OUTPUT_TARGET_DIR)/$(CONFIG)
MTB_TOOLS__OUTPUT_GENERATED_DIR:=$(MTB_TOOLS__OUTPUT_TARGET_DIR)/generated

MTB_TOOLS_make_BASE?=$(MTB_TOOLS__TOOLS_DIR)/make

ifneq ($(filter get_app_info,$(MAKECMDGOALS)),get_app_info)
include $(MTB_TOOLS_make_BASE)/core_version3/query-v3.mk
endif

ifneq ($(filter get_tool_info,$(MAKECMDGOALS)),get_tool_info)
include $(MTB_TOOLS_make_BASE)/core_version3/startex-v3.mk
endif

