################################################################################
# \file query.mk
#
# \brief
# This file performs mtbquery-related routines
#
################################################################################
# \copyright
# Copyright 2022-2023 Cypress Semiconductor Corporation
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
# Paths
################################################################################

_MTB_TOOLS__GAI_TMP=$(word 1,$(CY_BUILD_LOCATION) $(CY_RELATIVE_BUILD_DIR))
_MTB_TOOLS__GAI_FILE=$(_MTB_TOOLS__GAI_TMP)/get_app_info.txt

ifeq ($(CY_SECONDSTAGE),)

_MTB_TOOLS__GAI_ARGS:=

ifneq ($(CY_GETLIBS_SHARED_PATH),)
_MTB_TOOLS__GAI_ARGS:=CY_PROTOCOL=2
endif

$(shell mkdir -p $(_MTB_TOOLS__GAI_TMP) ; $(MAKE) get_app_info $(_MTB_TOOLS__GAI_ARGS) $(MAKEOVERRIDES) >$(_MTB_TOOLS__GAI_FILE))
endif

CY_TOOL_mtbquery_BASE?=$(CY_INTERNAL_TOOLS)/mtbquery
ifeq ($(OS),Windows_NT)
CY_TOOL_mtbquery_EXE?=$(CY_TOOL_mtbquery_BASE)/mtbquery.exe
else
CY_TOOL_mtbquery_EXE?=$(CY_TOOL_mtbquery_BASE)/mtbquery
endif

# make sure mtbquery exists where we think it's supposed to be.
$(if $(wildcard $(CY_TOOL_mtbquery_EXE)),,$(error Unable to find mtbquery at $(CY_TOOL_mtbquery_EXE) -- ABORTING!))

_MTB_START_QUERY_TMP_FILE:=$(CY_RELATIVE_BUILD_DIR)/mtbquery-info.mk
_MTB_START_QUERY_TMP_DIR:=$(dir $(_MTB_START_QUERY_TMP_FILE))

_MTB_TOOLS__START_QUERY_CMD:=$(CY_TOOL_mtbquery_EXE) --toolopen $(CY_INTERNAL_APPLOC) @$(_MTB_TOOLS__GAI_FILE) > $(_MTB_START_QUERY_TMP_FILE)

# Generate CY_TOOL_ and CY_OPEN_ entries (unless target is get_app_info, which causes a recursive loop).
ifeq ($(filter get_app_info,$(MAKECMDGOALS)),)

# Should only need to generate the file during the first stage. The second stage can the generated file from the firs stage.
ifeq ($(CY_SECONDSTAGE),)
$(info Searching installed tools in progress...)
ifneq ($(VERBOSE),)
$(info $(_MTB_TOOLS__START_QUERY_CMD))
endif
$(shell mkdir -p $(dir $(_MTB_START_QUERY_TMP_FILE)); $(_MTB_TOOLS__START_QUERY_CMD))
$(info Searching installed tools complete)
endif
-include $(_MTB_START_QUERY_TMP_FILE)
endif # ($(filter get_app_info,$(MAKECMDGOALS)),)
