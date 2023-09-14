################################################################################
# \file getlibs.mk
#
# \brief
# This file performs mtbgetlibs-related routines
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

ifneq (,$(CY_GETLIBS_OFFLINE)$(CY_GETLIBS_OFFLINE_PATH))
$(info WARNING: CY_GETLIBS_OFFLINE and CY_GETLIBS_OFFLINE_PATH are no longer support in ModusToolbox 3.1 and has been replaced with the Local Content Storage feature. See ModusToolbox User Guide for more information.)
endif

#
# Windows paths
#
ifeq ($(OS),Windows_NT)

#
# CygWin/MSYS
#
ifneq ($(CY_WHICH_CYGPATH),)
# CY_GETLIBS_PATH can be relative in certain cases. Return as is in such scenarios
ifeq ($(filter .%,$(CY_GETLIBS_PATH)),)
CY_INTERNAL_GETLIBS_PATH:=$(shell cygpath -m --absolute $(subst \,/,$(CY_GETLIBS_PATH)))
else
CY_INTERNAL_GETLIBS_PATH:=$(subst \,/,$(CY_GETLIBS_PATH))
endif
CY_INTERNAL_GETLIBS_DEPS_PATH:=$(shell cygpath -m --absolute $(subst \,/,$(CY_GETLIBS_DEPS_PATH)))

#
# Other Windows environments
#
else
ifeq ($(filter .%,$(CY_GETLIBS_PATH)),)
CY_INTERNAL_GETLIBS_PATH:=$(abspath $(subst \,/,$(CY_GETLIBS_PATH)))
else
CY_INTERNAL_GETLIBS_PATH:=$(subst \,/,$(CY_GETLIBS_PATH))
endif
CY_INTERNAL_GETLIBS_DEPS_PATH:=$(abspath $(subst \,/,$(CY_GETLIBS_DEPS_PATH)))
endif

#
# Linux and macOS paths
#
else
ifeq ($(filter .%,$(CY_GETLIBS_PATH)),)
CY_INTERNAL_GETLIBS_PATH:=$(abspath $(CY_GETLIBS_PATH))
else
CY_INTERNAL_GETLIBS_PATH:=$(CY_GETLIBS_PATH)
endif
CY_INTERNAL_GETLIBS_DEPS_PATH:=$(abspath $(CY_GETLIBS_DEPS_PATH))
endif


################################################################################
# Target variables
################################################################################

ifneq ($(wildcard $(CY_TOOL_proxy-helper_EXE)),)
_MTB_TOOLS__PROXY_HELPER_CMD=$(CY_TOOL_proxy-helper_EXE) --exec --
endif

ifneq ($(CY_GETLIBS_NO_CACHE),)
_MTB_TOOLS__GETLIBS_NO_CACHE_FLAG=--direct
else
_MTB_TOOLS__GETLIBS_NO_CACHE_FLAG=
endif

ifneq ($(MTB_USE_LOCAL_CONTENT),)
_MTB_TOOLS__USE_LOCAL_CONTENT_FLAGS:=--local
else
_MTB_TOOLS__USE_LOCAL_CONTENT_FLAGS:=
endif

# Note: Intentionally use CY_GETLIBS_PATH as it can be a relative path.
# Absolute path conversion for Windows environments happen in getlibs.bash
_MTB_TOOLS__EXECUTE_GETLIBS=$(_MTB_TOOLS__PROXY_HELPER_CMD) $(CY_TOOL_getlibs_EXE_ABS) --project $(CY_INTERNAL_APPLOC) $(_MTB_TOOLS__GETLIBS_NO_CACHE_FLAG) $(_MTB_TOOLS__USE_LOCAL_CONTENT_FLAGS)

################################################################################
# Targets
################################################################################

getlibs:
	@echo;\
	echo ==============================================================================;\
	echo =                          Importing libraries                               =;\
	echo ==============================================================================;
	@[ -d $(CY_GETLIBS_PATH) ] || mkdir -p $(CY_GETLIBS_PATH);
	$(if $(VERBOSE),,@)$(_MTB_TOOLS__EXECUTE_GETLIBS)
	@echo ==============================================================================;\
	echo =                          Import complete                                   =;\
	echo ==============================================================================;\
	echo

printlibs:
	@echo;\
	echo ==============================================================================;\
	echo =                     Printing status of libraries                           =;\
	echo ==============================================================================;\
    $(CY_TOOL_mtbquery_EXE_ABS) --printlibs . ;\
	echo ==============================================================================;\
	echo =                          Print complete                                    =;\
	echo ==============================================================================;\
	echo

.PHONY: getlibs printlibs
