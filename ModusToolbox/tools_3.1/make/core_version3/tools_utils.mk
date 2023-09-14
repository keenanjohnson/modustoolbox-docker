################################################################################
# \file utils.mk
#
# \brief
# This file contains utility functions
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


################################################################################
# Utility variables
################################################################################
# This setting allows the command to run in the background.  Some invocations of "make config"
# need to override this behavior (ex. VS Code in Windows)
MTB__JOB_BACKGROUND=&
MTB__EMPTY:=
# Create a make variable that contains a space
MTB__SPACE:=$(MTB__EMPTY) $(MTB__EMPTY)
MTB__OPEN_PAREN=(
MTB__CLOSE_PAREN=)
# Create a make variable that contains a soft tab
MTB__INDENT:=$(MTB__SPACE)$(MTB__SPACE)$(MTB__SPACE)$(MTB__SPACE)
# Create a make variable that contains a hard tab
MTB__TAB:=$(MTB__SPACE)	$(MTB__SPACE)
# Create a make variable that contains a comma
MTB__COMMA:=,
# Create a make variable that contains a line break
define MTB__NEWLINE


endef


# Displays/Hides the build steps
ifneq (,$(filter $(VERBOSE),true 1))
MTB__NOISE:=
MTB__SILENT_OUTPUT:=
else
MTB__NOISE:=@
MTB__SILENT_OUTPUT:= > /dev/null 2>&1
endif


################################################################################
# Environment-specific
################################################################################


# Extract the make version number
MTB__MAKE_MAJOR_VER:=$(word 1, $(subst ., ,$(MAKE_VERSION)))
MTB__MAKE_MINOR_VER:=$(word 2, $(subst ., ,$(MAKE_VERSION)))

# Set the default shell to bash
SHELL=/bin/bash
# TODO delete this variable
# Set the location of the find utility (Avoid conflict with Windows system32/find.exe)
MTB__FIND:=/usr/bin/find

# $(file > ) command is added in gnu make 4.0
# $(file < ) command is added in gnu make 4.2
# if version is older than 4.2, use cat and echo to read/write to file.
ifneq (,$(strip $(filter 3 2 1,$(MTB__MAKE_MAJOR_VER))))
# 1.X, 2.X, 3.X
MTB_FILE_TYPE:=cat
else
ifeq (4,$(MTB__MAKE_MAJOR_VER))
ifneq (,$(strip $(filter 1 0,$(MTB__MAKE_MINOR_VER))))
# 4.0, 4.1.
MTB_FILE_TYPE:=cat
else
# 4.X, X >=2
MTB_FILE_TYPE:=file
endif
else
# X.Y, X > 4
MTB_FILE_TYPE:=file
endif
endif

################################################################################
# Generic Macros
################################################################################

#
# Test for equality
# $(1) : Base path
# $(2) : Directory containing header file
#
mtb__equals=$(if $(and $(findstring $1,$2),$(findstring $2,$1)),TRUE)

ifeq ($(OS),Windows_NT)
ifeq ($(_MTB_TOOLS__WHICH_CYGPATH),)
ifneq (,$(findstring CYGWIN,$(shell uname)))
# TODO rename this
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

#
# Get a file or python script path based on its file name
# $(1) : A patch system CY_TOOL_... variable that points to the path if it's provided by the tools installer for the current platform.
# $(2) : (Optional/Fallback) Name of executable to look for in PATH if $(1) doesn't exist on the current platform.
# $(3) : (Optional/Fallback) Name of Python script to look for in CY_PYTHON_PATH.
#
mtb__get_file_path=$(call mtb__path_normalize,$(strip $(if $(1),$(call _mtb_tools__search_for_file_everywhere,$(1),$(2),$(3)),$(call _mtb_tools__search_for_file_enviromnent,$(2),$(3)))))
_mtb_tools__search_for_file_everywhere=$(if $(wildcard $(CY_TOOLS_DIR)/$($(1))),$(CY_TOOLS_DIR)/$($(1)),$(call _mtb_tools__search_for_file_enviromnent,$(2),$(3)))
_mtb_tools__search_for_file_enviromnent=$(if $(1),$(if $(shell type -P $(1)),$(shell type -P $(1)),$(call _mtb_tools__search_for_python_script,$(2))),$(call _mtb_tools__search_for_python_script,$(2)))
_mtb_tools__search_for_python_script=$(if $(1),\
					$(if $(filter $(OS),Windows_NT),\
					$(if $(findstring $(CY_TOOLS_DIR),$(dir $(dir $(CY_PYTHON_PATH)))),\
					$(dir $(CY_PYTHON_PATH))Scripts/$(1),\
					$(shell type -P $(1))),\
					$(shell type -P $(1))))

#
# Reads from file
# $(1) : File to read
#
ifeq ($(MTB_FILE_TYPE),file)
mtb__file_read=$(file <$1)
else
mtb__file_read=$(shell cat $1)
endif

#
# Writes to file
# $(1) : File to write
# $(2) : String
#
ifeq ($(MTB_FILE_TYPE),file)
mtb__file_write=$(file >$1,$2)
else
mtb__file_write=$(shell echo '$(subst ','"'"',$2)' >$1)
endif

#
# Appends to file
# $(1) : File to write
# $(2) : String
#
ifeq ($(MTB_FILE_TYPE),file)
mtb__file_append=$(file >>$1,$2)
else
mtb__file_append=$(shell echo '$(subst ','"'"',$2)' >>$1)
endif

#
# Convert to lower case
# $(1) : String to convert to lower case
#
mtb__lower_case=$(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst \
		H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst \
		Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst \
		W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

#
# Convert to upper case
# $(1) : String to convert to upper case
#
mtb__upper_case=$(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst \
		h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst \
		q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst \
		w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$1))))))))))))))))))))))))))

#
# Removes trailing slashes of path and then returns the path
# $(1) : Path
#
mtb__get_dir=$(patsubst %/,%,$(dir $(patsubst %/,%,$(filter-out .,$(1)))))

