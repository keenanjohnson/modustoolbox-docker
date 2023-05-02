################################################################################
# \file get_tool_info.mk
#
# \brief
# Used by tools to update their dependencies
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

ifeq ($(OS),Windows_NT)
ifeq ($(_MTB_TOOLS__WHICH_CYGPATH),)
ifneq (,$(findstring CYGWIN,$(shell uname)))
_MTB_TOOLS__WHICH_CYGPATH:=true
export _MTB_TOOLS__WHICH_CYGPATH
endif
endif
endif

#
# convert and normaize path.
#
ifeq ($(OS),Windows_NT)
ifneq ($(_MTB_TOOLS__WHICH_CYGPATH),)
mtb__path_normalize=$(if $(1),$(patsubst %/,%,$(shell cygpath -m --absolute $(subst \,/,$(1)))))
else
mtb__path_normalize=$(patsubst %/,%,$(abspath $(subst \,/,$(1))))
endif
else
mtb__path_normalize=$(patsubst %/,%,$(abspath $(1)))
endif

# Note: Path is necessary as this file is called using -f option
CY_CURRENT_DIR=$(dir $(lastword $(MAKEFILE_LIST)))
CY_TOOLS_DIR?=$(call mtb__path_normalize,$(CY_CURRENT_DIR)/..)

CY_TOOL_mtbquery_BASE_ABS?=$(CY_TOOLS_DIR)/mtbquery
CY_TOOL_mtbquery_EXE_ABS?=$(CY_TOOL_mtbquery_BASE_ABS)/mtbquery

# make sure mtbquery exists where we think it's supposed to be.
$(if $(wildcard $(CY_TOOL_mtbquery_EXE_ABS)),,$(error Unable to find mtbquery at $(CY_TOOL_mtbquery_EXE_ABS) -- ABORTING!))

_MTB_TOOLS__START_QUERY_CMD:=$(CY_TOOL_mtbquery_EXE_ABS) --alltools @MTB_TOOLS_DIR=$(CY_TOOLS_DIR)

get_tool_info:
	@:
	$(_MTB_TOOLS__START_QUERY_CMD)
	
.PHONY:get_tool_info
