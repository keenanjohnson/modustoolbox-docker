################################################################################
# \file application.mk
#
# \brief
# Include target for multi-core applications.
#
################################################################################
# \copyright
# Copyright 2022 Cypress Semiconductor Corporation
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

# Application level get_app_info 
# CY_PROTOCOl=2, MTB_QUERY=1. Supports modustoolbox 3.0
get_app_info:
	@:
	$(info MTB_QUERY=1)
	$(info MTB_TOOLS_DIR=$(CY_TOOLS_DIR))
	$(info MTB_TYPE=$(MTB_TYPE))
	$(info MTB_PROJECTS=$(MTB_PROJECTS))
	$(info MTB_BUILD_SUPPORT=$(MTB_BUILD_SUPPORT))

# Windows paths
ifeq ($(OS),Windows_NT)
_MTB_APP_DIR=$(notdir $(patsubst %/,%,$(dir $(realpath $(subst \,/, $(firstword $(MAKEFILE_LIST)))))))
# Linux and macOS paths
else
_MTB_APP_DIR=$(notdir $(patsubst %/,%,$(dir $(realpath $(firstword $(MAKEFILE_LIST))))))
endif # ifeq ($(OS),Windows_NT)

# Getlibs
CY_TOOL_mtbgetlibs_EXE?=$(CY_TOOLS_DIR)/mtbgetlibs/mtbgetlibs
getlibs:
	$(CY_TOOL_mtbgetlibs_EXE) --app .

# Select a list of projects to be merged into the combined application image
# When the application Makefile does not override this, merge all projects
MERGE?=$(MTB_PROJECTS)

# Recursive make targets.
_MTB_RECURSIVE_APPLICATION_MAKE_TARGET_COMMAND_LINE+=MTB_APPLICATION_SUBPROJECTS="$(MTB_PROJECTS)"
_MTB_RECURSIVE_APPLICATION_MAKE_TARGET_COMMAND_LINE+=MTB_APPLICATION_NAME="$(_MTB_APP_DIR)"
printlibs eclipse vscode uvision5 ewarm8:
	@:
	$(foreach project,$(MTB_PROJECTS),$(MAKE) -C $(project) $@ $(_MTB_RECURSIVE_APPLICATION_MAKE_TARGET_COMMAND_LINE) &&) true

clean:
	@:
	$(foreach project,$(MTB_PROJECTS),$(MAKE) -C $(project) $@_proj $(_MTB_RECURSIVE_APPLICATION_MAKE_TARGET_COMMAND_LINE) &&) true
	rm -rf build

all: build

build qbuild:
	@:
	$(foreach project,$(MTB_PROJECTS),$(MAKE) -C $(project) $@_proj $(_MTB_RECURSIVE_APPLICATION_MAKE_TARGET_COMMAND_LINE) $(if $(filter $(project),$(MERGE)),MTB_APPLICATION_PROMOTE=true) &&) true
	$(MAKE) -C $(firstword $(MTB_PROJECTS)) application_postbuild MTB_APPLICATION_SUBPROJECTS="$(MERGE)"

program: build qprogram

qprogram:
ifeq (,$(MERGE))
	$(error MERGE variable is empty, there are no application hex file to program. Call "make program_proj" from the project to program the project specific hex file)
else
	$(MAKE) -C $(firstword $(MTB_PROJECTS)) qprogram_proj $(_MTB_RECURSIVE_APPLICATION_MAKE_TARGET_COMMAND_LINE)
endif

help progtool:
	$(MAKE) -C $(firstword $(MTB_PROJECTS)) $@ $(_MTB_RECURSIVE_APPLICATION_MAKE_TARGET_COMMAND_LINE)

# Configurator targets.
_MTB_APP_MK_FILE=$(wildcard $(firstword $(MTB_PROJECTS))/*/app.mk)
ifeq ($(words $(_MTB_APP_MK_FILE)),1)
include $(_MTB_APP_MK_FILE)
else
ifeq ($(words $(_MTB_APP_MK_FILE)),0)
$(info NOTE: unable to find app.mk, make sure to run 'make getlibs'.)
else
$(warning found multiple 'app.mk' files: $(_MTB_APP_MK_FILE). Using the first.)
include $(firstword $(_MTB_APP_MK_FILE))
endif # ($(words $(_MTB_APP_MK_FILE)),)
endif # ($(words $(_MTB_APP_MK_FILE)),1)

.PHONY: get_app_info getlibs all build qbuild program qprogram clean printlibs eclipse vscode uvision5 ewarm8
