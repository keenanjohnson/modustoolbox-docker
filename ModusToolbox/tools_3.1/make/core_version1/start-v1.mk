################################################################################
# \file start.mk
#
# \brief
# This file is used to define the tools locations
#
################################################################################
# \copyright
# Copyright 2018-2023 Cypress Semiconductor Corporation
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

-include $(sort $(wildcard $(CY_TOOLS_DIR)/tools_update_*.mk))

#
# Set the location of top-level Makefile dir.
#
CY_APP_LOCATION:=$(CURDIR)

#
# Set the default app and base lib paths
#
ifeq ($(CY_APP_PATH),)
CY_INTERNAL_APP_PATH:=.
else
CY_INTERNAL_APP_PATH:=$(patsubst %/,%,$(CY_APP_PATH))
endif

ifeq ($(CY_EXTAPP_PATH),)
CY_INTERNAL_EXTAPP_PATH:=
else
CY_INTERNAL_EXTAPP_PATH:=$(patsubst %/,%,$(CY_EXTAPP_PATH))
endif

ifeq ($(OS),Windows_NT)
ifeq ($(CY_WHICH_CYGPATH),)
CY_WHICH_CYGPATH:=$(shell which cygpath)
export CY_WHICH_CYGPATH
endif
endif

#
# Calculate path when it's not secondstage (primarily for Windows cygpath optimization)
#
ifneq ($(CY_PATH_CONVERSION),false)

#
# Windows paths
#
ifeq ($(OS),Windows_NT)

#
# CygWin/MSYS
#
ifneq ($(CY_WHICH_CYGPATH),)

CY_INTERNAL_TOOLS:=$(shell cygpath -m --absolute $(subst \,/,$(CY_TOOLS_DIR)))
# This can't be named CY_INTERNAL_APPLOC since CY_INTERNAL_APPLOC can be set from the enviromnent.
# We need to re-export this from each core in case of multi-core.
# For example if we were doing vscode export from the project, we need this variable to reset the enviromnent.
CY_INTERNAL_APPLOC_EXPORT:=$(shell cygpath -m --absolute $(subst \,/,$(CY_APP_LOCATION)))
# Note: Bash only needs to be set once
ifeq ($(CY_BASH),)
CY_BASH:="$(shell cygpath -m --absolute /usr/bin/bash)" --norc --noprofile
endif

#
# Other Windows environments (not officially supported)
#
else
CY_INTERNAL_TOOLS:=$(abspath $(subst \,/,$(CY_TOOLS_DIR)))
# This can't be named CY_INTERNAL_APPLOC since CY_INTERNAL_APPLOC can be set from the enviromnent.
# We need to re-export this from each core in case of multi-core.
# For example if we were doing vscode export from the project, we need this variable to reset the enviromnent.
CY_INTERNAL_APPLOC_EXPORT:=$(abspath $(subst \,/,$(CY_APP_LOCATION)))
CY_BASH:=bash --norc --noprofile
endif

#
# Linux and macOS paths
#
else
CY_INTERNAL_TOOLS:=$(abspath $(CY_TOOLS_DIR))
# This can't be named CY_INTERNAL_APPLOC since CY_INTERNAL_APPLOC can be set from the enviromnent.
# We need to re-export this from each core in case of multi-core.
# For example if we were doing vscode export from the project, we need this variable to reset the enviromnent.
CY_INTERNAL_APPLOC_EXPORT:=$(abspath $(CY_APP_LOCATION))
CY_BASH:=bash --norc --noprofile
endif

export CY_INTERNAL_TOOLS
export CY_INTERNAL_APPLOC=$(CY_INTERNAL_APPLOC_EXPORT)
export CY_BASH

endif # ifneq ($(CY_PATH_CONVERSION),false)

#
# Relative build directory
#
CY_RELATIVE_BUILD_DIR:=$(CY_INTERNAL_APP_PATH)/build

CY_TOOL_make_BASE?=$(CY_INTERNAL_TOOLS)/make

ifneq ($(filter get_app_info,$(MAKECMDGOALS)),get_app_info)
include $(CY_TOOL_make_BASE)/core_version1/query-v1.mk
endif

ifneq ($(filter get_tool_info,$(MAKECMDGOALS)),get_tool_info)
include $(CY_TOOL_make_BASE)/core_version1/startex-v1.mk
endif
