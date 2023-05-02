################################################################################
# \file tools.mk
#
# \brief
# Specifies the tools in the installation
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


# Supported tool types
CY_OPEN_TYPE_LIST+=\
	device-configurator\
	capsense-configurator\
	capsense-tuner\
	qspi-configurator\
	seglcd-configurator\
	smartio-configurator\
	bt-configurator\
	usbdev-configurator\
	dfuh-tool\
	library-manager\
	project-creator\
	secure-policy-configurator\
	ez-pd-configurator\
	lin-configurator\
	ml-configurator


################################################################################
# Definition
################################################################################

##########################
# device-configurator
##########################

CY_OPEN_device_configurator_EXT=$(CY_CONFIG_MODUS_EXT)
CY_OPEN_device_configurator_FILE=$(CY_CONFIG_MODUS_FILE)
CY_OPEN_device_configurator_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_device-configurator_EXE)
CY_OPEN_device_configurator_TOOL_ARGS=$(CY_CONFIG_LIBFILE)
CY_OPEN_device_configurator_TOOL_FLAGS=$(CY_CONFIG_MODUS_GUI_FLAGS)
ifeq ($(CY_CONFIG_MODUS_FILE),)
CY_OPEN_device_configurator_TOOL_NEWCFG_FLAGS=$(CY_CONFIG_LIBFILE) --design $(CY_INTERNAL_APPLOC)/design.$(CY_OPEN_device_configurator_EXT)
endif


##########################
# capsense
##########################

CY_INTERNAL_OPEN_capsense_EXT=cycapsense
CY_INTERNAL_OPEN_capsense_FILE=$(wildcard $(call CY_MACRO_DIR,$(CY_CONFIG_MODUS_FILE))/*.$(CY_INTERNAL_OPEN_capsense_EXT))
ifneq ($(words $(CY_INTERNAL_OPEN_capsense_FILE)),1)
ifneq ($(words $(CY_INTERNAL_OPEN_capsense_FILE)),0)
$(call CY_MACRO_ERROR,Multiple Capsense configuration files detected: $(CY_INTERNAL_OPEN_capsense_FILE))
endif
endif

##########################
# capsense-configurator
##########################

CY_OPEN_capsense_configurator_EXT=$(CY_INTERNAL_OPEN_capsense_EXT)
CY_OPEN_capsense_configurator_FILE=$(CY_INTERNAL_OPEN_capsense_FILE)
CY_OPEN_capsense_configurator_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_capsense-configurator_EXE)
CY_OPEN_capsense_configurator_TOOL_FLAGS=$(CY_CONFIG_LIBFILE) --config 
ifeq ($(CY_CONFIG_MODUS_FILE),)
CY_OPEN_capsense_configurator_TOOL_NEWCFG_FLAGS=$(CY_CONFIG_LIBFILE) --config $(CY_INTERNAL_APPLOC)/design.$(CY_OPEN_capsense_configurator_EXT)
else
CY_OPEN_capsense_configurator_TOOL_NEWCFG_FLAGS=$(CY_CONFIG_LIBFILE) --config $(call CY_MACRO_DIR,$(CY_CONFIG_MODUS_FILE))/design.$(CY_OPEN_capsense_configurator_EXT)
endif

##########################
# capsense-tuner
##########################

CY_OPEN_capsense_tuner_EXT=$(CY_INTERNAL_OPEN_capsense_EXT)
CY_OPEN_capsense_tuner_FILE=$(CY_INTERNAL_OPEN_capsense_FILE)
CY_OPEN_capsense_tuner_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_capsense-tuner_EXE)
CY_OPEN_capsense_tuner_TOOL_FLAGS=--config 
ifeq ($(CY_CONFIG_MODUS_FILE),)
CY_OPEN_capsense_tuner_TOOL_NEWCFG_FLAGS=--config $(CY_INTERNAL_APPLOC)/design.$(CY_OPEN_capsense_tuner_EXT)
else
CY_OPEN_capsense_tuner_TOOL_NEWCFG_FLAGS=--config $(call CY_MACRO_DIR,$(CY_CONFIG_MODUS_FILE))/design.$(CY_OPEN_capsense_tuner_EXT)
endif

##########################
# qspi-configurator
##########################

CY_OPEN_qspi_configurator_EXT=cyqspi
CY_OPEN_qspi_configurator_OUTPUT_DIR=$(call CY_MACRO_DIR,$(CY_CONFIG_MODUS_FILE))
CY_OPEN_qspi_configurator_FILE=$(wildcard $(CY_OPEN_qspi_configurator_OUTPUT_DIR)/*.$(CY_OPEN_qspi_configurator_EXT))
ifneq ($(words $(CY_OPEN_qspi_configurator_FILE)),1)
ifneq ($(words $(CY_OPEN_qspi_configurator_FILE)),0)
$(call CY_MACRO_ERROR,Multiple QSPI configuration files detected: $(CY_OPEN_qspi_configurator_FILE))
endif
endif
CY_OPEN_qspi_configurator_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_qspi-configurator_EXE)
CY_OPEN_qspi_configurator_TOOL_FLAGS=--config 

ifneq ($(CY_OPEN_qspi_configurator_FILE),)
ifneq ($(CY_QSPI_FLM_DIR),)
CY_OPEN_qspi_configurator_ADDITIONAL_ARGS=--flashloader-dir $(CY_QSPI_FLM_DIR)
endif
endif

ifeq ($(CY_CONFIG_MODUS_FILE),)
CY_OPEN_qspi_configurator_TOOL_NEWCFG_FLAGS=--config $(CY_INTERNAL_APPLOC)/design.$(CY_OPEN_qspi_configurator_EXT)
else
CY_OPEN_qspi_configurator_TOOL_NEWCFG_FLAGS=--config $(CY_OPEN_qspi_configurator_OUTPUT_DIR)/design.$(CY_OPEN_qspi_configurator_EXT)
endif

##########################
# seglcd-configurator
##########################

CY_OPEN_seglcd_configurator_EXT=cyseglcd
CY_OPEN_seglcd_configurator_FILE=$(wildcard $(call CY_MACRO_DIR,$(CY_CONFIG_MODUS_FILE))/*.$(CY_OPEN_seglcd_configurator_EXT))
ifneq ($(words $(CY_OPEN_seglcd_configurator_FILE)),1)
ifneq ($(words $(CY_OPEN_seglcd_configurator_FILE)),0)
$(call CY_MACRO_ERROR,Multiple SegLCD configuration files detected: $(CY_OPEN_seglcd_configurator_FILE))
endif
endif
CY_OPEN_seglcd_configurator_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_seglcd-configurator_EXE)
CY_OPEN_seglcd_configurator_TOOL_FLAGS=$(CY_CONFIG_LIBFILE) --config 
ifeq ($(CY_CONFIG_MODUS_FILE),)
CY_OPEN_seglcd_configurator_TOOL_NEWCFG_FLAGS=$(CY_CONFIG_LIBFILE) --config $(CY_INTERNAL_APPLOC)/design.$(CY_OPEN_seglcd_configurator_EXT)
else
CY_OPEN_seglcd_configurator_TOOL_NEWCFG_FLAGS=$(CY_CONFIG_LIBFILE) --config $(call CY_MACRO_DIR,$(CY_CONFIG_MODUS_FILE))/design.$(CY_OPEN_seglcd_configurator_EXT)
endif

##########################
# smartio-configurator
##########################

CY_OPEN_smartio_configurator_EXT=$(CY_CONFIG_MODUS_EXT)
CY_OPEN_smartio_configurator_FILE=$(CY_CONFIG_MODUS_FILE)
CY_OPEN_smartio_configurator_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_smartio-configurator_EXE)
CY_OPEN_smartio_configurator_TOOL_FLAGS=$(CY_CONFIG_LIBFILE) 
CY_OPEN_smartio_configurator_TOOL_NEWCFG_FLAGS=$(CY_CONFIG_LIBFILE) 

##########################
# bt-configurator
##########################

CY_OPEN_bt_configurator_EXT=$(CY_CONFIG_CYBT_EXT)
CY_OPEN_bt_configurator_FILE=$(CY_CONFIG_CYBT_FILE)
CY_OPEN_bt_configurator_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_bt-configurator_EXE)
CY_OPEN_bt_configurator_TOOL_FLAGS=--config 
CY_OPEN_bt_configurator_TOOL_NEWCFG_FLAGS=--config $(CY_INTERNAL_APPLOC)/design.$(CY_OPEN_bt_configurator_EXT) $(CY_OPEN_bt_configurator_DEVICE)

##########################
# usbdev-configurator
##########################

CY_OPEN_usbdev_configurator_EXT=$(CY_CONFIG_CYUSBDEV_EXT)
CY_OPEN_usbdev_configurator_FILE=$(CY_CONFIG_CYUSBDEV_FILE)
CY_OPEN_usbdev_configurator_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_usbdev-configurator_EXE)
CY_OPEN_usbdev_configurator_TOOL_FLAGS=--config 
CY_OPEN_usbdev_configurator_TOOL_NEWCFG_FLAGS=--config $(CY_INTERNAL_APPLOC)/design.$(CY_OPEN_usbdev_configurator_EXT)

##########################
# dfuh-tool
##########################

CY_OPEN_dfuh_tool_EXT=
CY_OPEN_dfuh_tool_FILE=
CY_OPEN_dfuh_tool_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_dfuh-tool_EXE)
CY_OPEN_dfuh_tool_TOOL_FLAGS=
CY_OPEN_dfuh_tool_TOOL_NEWCFG_FLAGS=

##########################
# library-manager
##########################

CY_OPEN_library_manager_EXT=
CY_OPEN_library_manager_FILE=
CY_OPEN_library_manager_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_library-manager_EXE)
CY_OPEN_library_manager_TOOL_FLAGS=--target-dir $(CY_INTERNAL_APPLOC)
CY_OPEN_library_manager_TOOL_NEWCFG_FLAGS=

##########################
# project-creator
##########################

CY_OPEN_project_creator_EXT=
CY_OPEN_project_creator_FILE=
CY_OPEN_project_creator_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_project-creator_EXE)
CY_OPEN_project_creator_TOOL_FLAGS=
CY_OPEN_project_creator_TOOL_NEWCFG_FLAGS=

##########################
# secure-policy-configurator
##########################

CY_OPEN_secure_policy_configurator_EXT=
CY_OPEN_secure_policy_configurator_FILE=
CY_OPEN_secure_policy_configurator_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_secure-policy-configurator_EXE)
CY_OPEN_secure_policy_configurator_TOOL_FLAGS=--target=$(CY_CYSECURETOOLS_TARGET)
CY_OPEN_secure_policy_configurator_TOOL_NEWCFG_FLAGS=--target=$(CY_CYSECURETOOLS_TARGET)

##########################
# ez-pd-configurator
##########################
CY_OPEN_ez_pd_configurator_EXT=$(CY_CONFIG_MTBEZPD_EXT)
CY_OPEN_ez_pd_configurator_FILE=$(CY_CONFIG_MTBEZPD_FILE)
CY_OPEN_ez_pd_configurator_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_ez-pd-configurator_EXE)
CY_OPEN_ez_pd_configurator_TOOL_FLAGS=--config 
CY_OPEN_ez_pd_configurator_TOOL_NEWCFG_FLAGS=--config $(CY_INTERNAL_APPLOC)/design.$(CY_OPEN_ez_pd_configurator_EXT)

##########################
# lin-configurator
##########################
CY_OPEN_lin_configurator_EXT=$(CY_CONFIG_MTBLIN_EXT)
CY_OPEN_lin_configurator_FILE=$(CY_CONFIG_MTBLIN_FILE)
CY_OPEN_lin_configurator_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_lin-configurator_EXE)
CY_OPEN_lin_configurator_TOOL_FLAGS=--config 
CY_OPEN_lin_configurator_TOOL_NEWCFG_FLAGS=--config $(CY_INTERNAL_APPLOC)/design.$(CY_OPEN_lin_configurator_EXT)

##########################
# ml-configurator
##########################
CY_OPEN_ml_configurator_EXT=mtbml
CY_CONFIG_MTBML_FILES:=$(call CY_CONFIG_FILES_SEARCHES,$(CY_OPEN_ml_configurator_EXT))
CY_SEARCH_PRUNED_MTBML_FILES:=$(filter-out $(foreach d,$(CY_IGNORE_DIRS),$(filter $(d)%,$(CY_CONFIG_MTBML_FILES))),$(CY_CONFIG_MTBML_FILES))
CY_OPEN_ml_configurator_FILE=$(call CY_MACRO_FILTER_FILES,MTBML)
ifneq ($(words $(CY_OPEN_ml_configurator_FILE)),1)
ifneq ($(words $(CY_OPEN_ml_configurator_FILE)),0)
$(call CY_MACRO_ERROR,Multiple ML configuration files detected: $(CY_OPEN_ml_configurator_FILE))
endif
endif
CY_OPEN_ml_configurator_TOOL=$(CY_INTERNAL_TOOLS)/$(CY_TOOL_ml-configurator_EXE)
CY_OPEN_ml_configurator_TOOL_FLAGS=--config 
CY_OPEN_ml_configurator_TOOL_NEWCFG_FLAGS=--config $(CY_INTERNAL_APPLOC)/design.$(CY_OPEN_ml_configurator_EXT)

################################################################################
# File type defaults
################################################################################

modus_DEFAULT_TYPE+=device-configurator
cycapsense_DEFAULT_TYPE+=capsense-configurator capsense-tuner
cyqspi_DEFAULT_TYPE+=qspi-configurator
cyseglcd_DEFAULT_TYPE+=seglcd-configurator
cybt_DEFAULT_TYPE+=bt-configurator
cyusbdev_DEFAULT_TYPE+=usbdev-configurator
cyacd2_DEFAULT_TYPE+=dfuh-tool
mtblin_DEFAULT_TYPE+=lin-configurator
mtbml_DEFAULT_TYPE+=ml-configurator
mtbezpd_DEFAULT_TYPE+=ez-pd-configurator
