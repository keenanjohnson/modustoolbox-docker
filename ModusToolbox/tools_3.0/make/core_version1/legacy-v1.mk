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

get_app_info: get_app_info_patch

get_app_info_patch:
	$(info MTB_ADDITIONAL_DEVICES=$(ADDITIONAL_DEVICES))

################################################################################
# Macros
################################################################################

#
# Constructs deprecated variable message to print out at the end
#
define CY_MACRO_DEPRECATED_TOOLS
CY_MESSAGE_TOOLS=INFO: The build support (base-library such as psoc6make) is referencing a deprecated tools variable. \
Tools update/patch feature will not work with this version. Update it to a newer version to use this feature.
endef


################################################################################
# Tools/toolchains definition (legacy support)
################################################################################

CY_TOOL_cfg-backend-cli_BASE=$(CY_TOOL_device-configurator_BASE)
CY_TOOL_cfg-backend-cli_EXE=$(CY_TOOL_device-configurator-cli_EXE)
#
# Default installed tools
#
CY_BT_CONFIGURATOR_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_bt-configurator_BASE)
CY_CAPSENSE_CONFIGURATOR_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_capsense-configurator_BASE)
CY_CFG_BACKEND_CLI_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_cfg-backend-cli_BASE)
CY_MCUELFTOOL_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_cymcuelftool_BASE)
CY_DEVICE_CONFIGURATOR_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_device-configurator_BASE)
CY_DFUH_TOOL_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_dfuh-tool_BASE)
CY_DRIVER_MEDIA_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_driver_media_BASE)
CY_FW_LOADER_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_fw-loader_BASE)
CY_COMPILER_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_gcc_BASE)
CY_JRE_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_jre_BASE)
CY_LIBRARY_MANAGER_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_library-manager_BASE)
CY_MAKEFILES_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_make_BASE)
CY_MODUS_SHELL_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_modus-shell_BASE)
CY_OPENOCD_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_openocd_BASE)
CY_PROJECT_CREATOR_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_project-creator_BASE)
CY_PROXY_HELPER_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_proxy-helper_BASE)
CY_QSPI_CONFIGURATOR_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_qspi-configurator_BASE)
CY_SEGLCD_CONFIGURATOR_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_seglcd-configurator_BASE)
CY_SMARTIO_CONFIGURATOR_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_smartio-configurator_BASE)
CY_USBDEV_CONFIGURATOR_DEFAULT_DIR:=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_usbdev-configurator_BASE)

#
# Tool locations
#
CY_BT_CONFIGURATOR_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_BT_CONFIGURATOR_DEFAULT_DIR)
CY_CAPSENSE_CONFIGURATOR_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_CAPSENSE_CONFIGURATOR_DEFAULT_DIR)
CY_CFG_BACKEND_CLI_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_CFG_BACKEND_CLI_DEFAULT_DIR)
CY_MCUELFTOOL_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_MCUELFTOOL_DEFAULT_DIR)
CY_DEVICE_CONFIGURATOR_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_DEVICE_CONFIGURATOR_DEFAULT_DIR)
CY_DFUH_TOOL_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_DFUH_TOOL_DEFAULT_DIR)
CY_DRIVER_MEDIA_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_DRIVER_MEDIA_DEFAULT_DIR)
CY_FW_LOADER_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_FW_LOADER_DEFAULT_DIR)
CY_COMPILER_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_COMPILER_DEFAULT_DIR)
CY_JRE_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_JRE_DEFAULT_DIR)
CY_LIBRARY_MANAGER_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_LIBRARY_MANAGER_DEFAULT_DIR)
CY_MAKEFILES_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_MAKEFILES_DEFAULT_DIR)
CY_MODUS_SHELL_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_MODUS_SHELL_DEFAULT_DIR)
CY_OPENOCD_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_OPENOCD_DEFAULT_DIR)
CY_PROJECT_CREATOR_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_PROJECT_CREATOR_DEFAULT_DIR)
CY_PROXY_HELPER_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_PROXY_HELPER_DEFAULT_DIR)
CY_QSPI_CONFIGURATOR_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_QSPI_CONFIGURATOR_DEFAULT_DIR)
CY_SEGLCD_CONFIGURATOR_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_SEGLCD_CONFIGURATOR_DEFAULT_DIR)
CY_SMARTIO_CONFIGURATOR_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_SMARTIO_CONFIGURATOR_DEFAULT_DIR)
CY_USBDEV_CONFIGURATOR_DIR?=$(eval $(call CY_MACRO_DEPRECATED_TOOLS))$(CY_USBDEV_CONFIGURATOR_DEFAULT_DIR)


################################################################################
# Search cyignore files
################################################################################

# Note: this is only used in search.mk and hence it was moved there.
# But a copy is kept here for BWC with older apps. Remove in the future.
CY_IGNORE_FILES=$(filter %.$(CY_TOOLCHAIN_SUFFIX_C),$(CY_IGNORE_DIRS))\
                        $(filter %.$(CY_TOOLCHAIN_SUFFIX_S),$(CY_IGNORE_DIRS))\
                        $(filter %.$(CY_TOOLCHAIN_SUFFIX_s),$(CY_IGNORE_DIRS))\
                        $(filter %.$(CY_TOOLCHAIN_SUFFIX_CPP),$(CY_IGNORE_DIRS))\
                        $(filter %.$(CY_TOOLCHAIN_SUFFIX_A),$(CY_IGNORE_DIRS))\
                        $(filter %.$(CY_TOOLCHAIN_SUFFIX_H),$(CY_IGNORE_DIRS))\
                        $(filter %.$(CY_TOOLCHAIN_SUFFIX_HPP),$(CY_IGNORE_DIRS))
CY_IGNORE_PRUNE=$(filter-out cytemp-o,cytemp$(addprefix -o -path ,$(filter-out $(CY_IGNORE_FILES),$(CY_IGNORE_DIRS))))

################################################################################
# Targets
################################################################################

# Stub targets for the IDE
ifneq ($(CY_SKIP_RECIPE),)
vscode eclipse build program:
	@echo
endif
