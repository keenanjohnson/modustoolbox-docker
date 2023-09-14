################################################################################
# \file start.mk
#
# \brief
# This file is used to determine the flow version of the application.
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

# empty for MTB 3.0 or earlier
# 1 for MTB 3.1
MTB_TOOLS__INTERFACE_VERSION:=1

ifeq ($(WHICHFILE),true)
$(info Processing $(lastword $(MAKEFILE_LIST)))
endif

ifeq ($(strip $(CY_GETLIBS_SHARED_PATH)),)
FLOW_VERSION:=1
$(error This application uses an older build flow (from version 2.1 or earlier) that is not supported in this version of ModusToolbox. Please view KBA236134 in the Infineon Developer Community for instructions on updating your application)
else
FLOW_VERSION:=2
ifeq ($(strip $(MTB_TYPE)),)
include $(CY_TOOLS_DIR)/make/core_version1/start-v1.mk
else
include $(CY_TOOLS_DIR)/make/core_version3/start-v3.mk
endif
endif
