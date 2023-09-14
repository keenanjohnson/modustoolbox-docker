################################################################################
# \file startex.mk
#
# \brief
# This file is used as the starting point for an application build.
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


################################################################################
# PATHS
################################################################################

#
# Include SEARCH variables
#
-include $(CY_GETLIBS_PATH)/mtb.mk

# TODO see IROS 8.2.2
# This must be after mtb.mk is included. This the set of directories that autodiscovery needs to search.
MTB_TOOLS__SEARCH:=$(SEARCH)

# include the BSP makefile
MTB_TOOLS__TARGET_DIR:=$(SEARCH_TARGET_$(TARGET))
-include $(MTB_TOOLS__TARGET_DIR)/bsp.mk
-include $(MTB_TOOLS__TARGET_DIR)/$(TARGET).mk

MTB_TOOLS__CORE_DIR:=$(SEARCH_core-make)
MTB_TOOLS__RECIPE_DIR:=$(firstword $(strip $(CY_BASELIB_PATH) $(RECIPE_DIR)))

################################################################################
# Includes
################################################################################

# Must come after cyignore search
include $(MTB_TOOLS_make_BASE)/core_version3/legacy-v3.mk

# Control build using baselib versus not including the baselib
_MTB_TOOLS__MISSING_BASELIB:=
_MTB_TOOLS__REQUIRE_BASELIB:=true
_MTB_TOOLS__INCLUDE_GETLIBS:=false

ifneq ($(CY_SKIP_RECIPE),)
_MTB_TOOLS__REQUIRE_BASELIB:=false
endif

ifeq ($(wildcard $(MTB_TOOLS__TARGET_DIR)),)
_MTB_TOOLS__MISSING_BASELIB+=bsp
endif

ifeq ($(wildcard $(MTB_TOOLS__CORE_DIR)),)
_MTB_TOOLS__MISSING_BASELIB+=core-make
endif

ifeq ($(wildcard $(MTB_TOOLS__RECIPE_DIR)),)
_MTB_TOOLS__MISSING_BASELIB+=recipe-make
endif

ifeq ($(MAKECMDGOALS),)
_MTB_TOOLS__REQUIRE_BASELIB:=false
endif

# Control build using getlibs versus not including the getlibs
ifeq ($(filter getlibs,$(MAKECMDGOALS)),getlibs)
_MTB_TOOLS__INCLUDE_GETLIBS:=true
_MTB_TOOLS__REQUIRE_BASELIB:=false
# Check that getlibs is called without other targets
ifneq ($(filter-out getlibs,$(MAKECMDGOALS)),)
$(error Detected "$(MAKECMDGOALS)" make targets.\
The getlibs target must not be combined with other make targets)
endif
endif

ifeq ($(filter printlibs,$(MAKECMDGOALS)),printlibs)
_MTB_TOOLS__INCLUDE_GETLIBS:=true
_MTB_TOOLS__REQUIRE_BASELIB:=false
endif

ifeq ($(filter get_app_info,$(MAKECMDGOALS)),get_app_info)
# get_app_info is defined both in tools-make and core-make.
# Only define get_app_info in tools-make if core-make is missing.
ifneq ($(_MTB_TOOLS__MISSING_BASELIB),)
_MTB_TOOLS__REQUIRE_BASELIB:=false

# This is an ordered list of supported version.
_MTB_TOOLS__MAKE_SUPPORTED_MTB_QUERY_VERSIONS=1
# Protocol 2: Supports separate /libs and /deps directories
CY_PROTOCOL:=2
# CY_PROTOCOl=2, MTB_QUERY=1. Supports modustoolbox 3.0
get_app_info:
	@:
	$(info MTB_MPN_LIST=$(MPN_LIST))
	$(info MTB_DEVICE_LIST=$(DEVICE_LIST))
	$(info MTB_DEVICE=$(DEVICE))
	$(info MTB_SEARCH=$(MTB_TOOLS__SEARCH))
	$(info MTB_TOOLCHAIN=$(TOOLCHAIN))
	$(info MTB_TARGET=$(TARGET))
	$(info MTB_CONFIG=$(CONFIG))
	$(info MTB_APP_NAME=$(APPNAME)$(LIBNAME))
	$(info MTB_COMPONENTS=$(MTB_CORE__FULL_COMPONENT_LIST))
	$(info MTB_DISABLED_COMPONENTS=$(DISABLE_COMPONENTS))
	$(info MTB_ADDITIONAL_DEVICES=$(ADDITIONAL_DEVICES))
	$(info MTB_LIBS=$(CY_GETLIBS_PATH))
	$(info MTB_DEPS=$(CY_GETLIBS_DEPS_PATH))
	$(info MTB_WKS_SHARED_NAME=$(CY_GETLIBS_SHARED_NAME))
	$(info MTB_WKS_SHARED_DIR=$(CY_GETLIBS_SHARED_PATH))
	$(info MTB_FLOW_VERSION=$(FLOW_VERSION))
ifneq ($(MTB_QUERY),)
	$(info MTB_QUERY=$(MTB_QUERY))
else
	$(info MTB_QUERY=$(_MTB_TOOLS__MAKE_SUPPORTED_MTB_QUERY_VERSIONS))
endif
	$(info MTB_TOOLS_DIR=$(MTB_TOOLS__TOOLS_DIR))
	$(info MTB_DEVICE_PROGRAM_IDS=)
	$(info MTB_BSP_TOOL_TYPES=)
	$(info MTB_MW_TOOL_TYPES=)
	$(info MTB_IGNORE=$(strip $(CY_IGNORE) $(MTB_TOOLS__OUTPUT_BASE_DIR)))
	$(info MTB_TYPE=$(MTB_TYPE))
	$(info MTB_CORE_TYPE=$(MTB_RECIPE__CORE))
	$(info MTB_CORE_NAME=$(MTB_RECIPE__CORE_NAME))
	$(info MTB_BUILD_SUPPORT=$(MTB_BUILD_SUPPORT))
	$(info MTB_CACHE_DIR=$(MTB_TOOLS__CACHE_DIR))
	$(info MTB_OFFLINE_DIR=$(MTB_TOOLS__OFFLINE_DIR))
	$(info MTB_GLOBAL_DIR=$(MTB_TOOLS__GLOBAL_DIR))
	$(info MTB_APP_PATH=$(MTB_TOOLS__REL_PRJ_PATH))
	$(info MTB_TOOLS_MAKE_VER=3.0)

endif
endif

ifeq ($(_MTB_TOOLS__INCLUDE_GETLIBS),true)
include $(MTB_TOOLS_make_BASE)/core_version3/getlibs-v3.mk
endif

ifeq ($(_MTB_TOOLS__REQUIRE_BASELIB),true)
ifeq ($(_MTB_TOOLS__MISSING_BASELIB),)
include $(MTB_TOOLS__CORE_DIR)/make/core/main.mk
else
$(error Libraries: "$(_MTB_TOOLS__MISSING_BASELIB)" not found. Run "make getlibs" to ensure all required build and code dependencies are present.)
endif
endif

.PHONY: get_app_info
