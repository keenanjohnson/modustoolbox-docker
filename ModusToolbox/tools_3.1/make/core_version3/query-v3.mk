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

_MTB_TOOLS_MAKE_CMDLINE=$(sort $(foreach var,$(.VARIABLES),$(if $(filter command line,$(origin $(var))),$(var)="$($(var))")))
# Clear the mtbqueryapi cache on the first build stage
ifeq ($(CY_SECONDSTAGE),)
ifneq ($(_MTB_TOOLS_MAKE_CMDLINE),)
ifneq ($(notdir $(dir $(firstword $(MAKEFILE_LIST)))/.mtbqueryapi),)
$(info Removing $(notdir $(dir $(firstword $(MAKEFILE_LIST)))/.mtbqueryapi) file...)
endif
$(shell rm -f $(notdir $(dir $(firstword $(MAKEFILE_LIST)))/.mtbqueryapi))
endif
endif

CY_TOOL_mtbquery_BASE_ABS?=$(MTB_TOOLS__TOOLS_DIR)/mtbquery
ifeq ($(OS),Windows_NT)
CY_TOOL_mtbquery_EXE_ABS?=$(CY_TOOL_mtbquery_BASE_ABS)/mtbquery.exe
else
CY_TOOL_mtbquery_EXE_ABS?=$(CY_TOOL_mtbquery_BASE_ABS)/mtbquery
endif

# make sure mtbquery exists where we think it's supposed to be.
$(if $(wildcard $(CY_TOOL_mtbquery_EXE_ABS)),,$(error Unable to find mtbquery at $(CY_TOOL_mtbquery_EXE_ABS) -- ABORTING!))

_MTB_START_QUERY_TMP_FILE=$(MTB_TOOLS__OUTPUT_BASE_DIR)/mtbquery-info.mk
_MTB_START_QUERY_TMP_DIR=$(dir $(_MTB_START_QUERY_TMP_FILE))

_MTB_TOOLS__START_QUERY_CMD:=$(CY_TOOL_mtbquery_EXE_ABS) --alltools @MTB_TOOLS_DIR=$(CY_TOOLS_DIR) > $(_MTB_START_QUERY_TMP_FILE)

# Generate CY_TOOL_ and CY_OPEN_ entries (unless target is get_app_info, which causes a recursive loop).
ifeq ($(filter get_app_info,$(MAKECMDGOALS)),)

# Should only need to generate the file during the first stage. The second stage can the generated file from the firs stage.
ifeq ($(CY_SECONDSTAGE),)
$(info Searching installed tools in progress...)
ifneq ($(VERBOSE),)
$(info $(_MTB_TOOLS__START_QUERY_CMD))
endif
_MTB_TOOLS__MTB_QUERY_ERROR_CODE:=$(shell mkdir -p $(_MTB_START_QUERY_TMP_DIR) && $(_MTB_TOOLS__START_QUERY_CMD) > $(_MTB_START_QUERY_TMP_FILE) ; echo $$?)
ifeq (0,$(_MTB_TOOLS__MTB_QUERY_ERROR_CODE))
$(info Searching installed tools complete)
else
$(error Searching installed tools failed with error code: $(_MTB_TOOLS__MTB_QUERY_ERROR_CODE))
endif
endif
-include $(_MTB_START_QUERY_TMP_FILE)
endif # ($(filter get_app_info,$(MAKECMDGOALS)),)
