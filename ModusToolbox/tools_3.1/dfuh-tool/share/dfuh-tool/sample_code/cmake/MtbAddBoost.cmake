# Copyright 2018-2023 Cypress Semiconductor Corporation (an Infineon company)
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

include_guard()

# Note: BOOST_ROOT is set by build.sh

set(_boost_components "atomic;filesystem;program_options;thread")
set(Boost_USE_STATIC_LIBS ON)
unset(Boost_INCLUDE_DIR CACHE)
find_package(Boost COMPONENTS ${_boost_components} REQUIRED)

list(APPEND _boost_components headers)
foreach(_comp ${_boost_components})
  # This is required by mtb_add_clang_tidy to query Boost include directories
  set_target_properties(Boost::${_comp} PROPERTIES IMPORTED_GLOBAL ON)

  # Disable automatic linking with MSVC (adds `-DBOOST_ALL_NO_LIB`)
  target_link_libraries(Boost::${_comp} INTERFACE
    Boost::disable_autolinking
  )
endforeach()
unset(_boost_components)
