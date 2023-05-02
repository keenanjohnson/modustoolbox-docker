################################################################################
# \file checks.mk
#
# \brief
# This file performs environment, app and bwc checks
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
# Tools/toolchains definition (legacy support)
################################################################################

#
# Tool locations
#
CY_COMPILER_DEFAULT_DIR:=$(CY_TOOL_gcc_BASE_ABS)
CY_COMPILER_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_COMPILER_DEFAULT_DIR)

mtb_path_normalize=$(mtb__path_normalize)
mtb_error=$(mtb__equals)

################################################################################
# Legacy CY Names
################################################################################

# character variables
CY_EMPTY:=$(MTB__EMPTY)
CY_SPACE:=$(MTB__SPACE)
CY_INDENT:=$(MTB__INDENT)
CY_TAB:=$(MTB__TAB)
CY_NEWLINE:=$(MTB__NEWLINE)
CY_COMMA:=$(MTB__COMMA)
CY_NEWLINE_MARKER:=$(MTB__NEWLINE_MARKER)

# command variables
CY_NOISE:=$(MTB__NOISE)
CY_CMD_TERM:=$(MTB__SILENT_OUTPUT)
CY_CONFIG_JOB_CONTROL:=$(MTB__JOB_BACKGROUND)
CY_FIND:=$(MTB__FIND)
CY_BASH:=$(MTB_TOOLS__BASH_CMD)

# file ops functions
CY_MACRO_FILE_READ=$(mtb__file_read)
CY_MACRO_FILE_WRITE=$(mtb__file_write)
CY_GET_FILE_PATH=$(mtb__get_file_path)

# string functons
CY_MACRO_EQUALITY=$(mtb__equals)
CY_MACRO_LC=$(mtb__lower_case)
CY_MACRO_UC=$(mtb__upper_case)

# paths
CY_INTERNAL_BUILD_LOC:=$(MTB_TOOLS__OUTPUT_BASE_DIR)
CY_INTERNAL_APPLOC:=$(MTB_TOOLS__PRJ_DIR)
CY_INTERNAL_TOOLS:=$(MTB_TOOLS__TOOLS_DIR)
