# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008-2026 Eduardo Aguiar
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Project
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cmake_minimum_required(VERSION 3.10)
project(app_emule LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(app_emule MODULE
    main.cpp
    common.cpp
    ctag.cpp
    file_cancelled_met.cpp
    file_key_index_dat.cpp
    file_known_met.cpp
    file_part_met.cpp
    file_part_met_txtsrc.cpp
    file_stored_searches_met.cpp
    profile.cpp
    vfs_processor_impl.cpp
)

set_target_properties(app_emule PROPERTIES OUTPUT_NAME "app-emule" PREFIX "")
target_link_libraries(app_emule PRIVATE Mobius::Framework Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: emule_cancelled_met
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_emule_cancelled_met
    tools/emule_cancelled_met.cpp
    ctag.cpp
    file_cancelled_met.cpp
)

set_target_properties(tools_emule_cancelled_met PROPERTIES OUTPUT_NAME "emule_cancelled_met")
target_link_libraries(tools_emule_cancelled_met PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: emule_key_index_dat
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_emule_key_index_dat
    tools/emule_key_index_dat.cpp
    ctag.cpp
    file_key_index_dat.cpp
)

set_target_properties(tools_emule_key_index_dat PROPERTIES OUTPUT_NAME "emule_key_index_dat")
target_link_libraries(tools_emule_key_index_dat PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: emule_known_met
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_emule_known_met
    tools/emule_known_met.cpp
    ctag.cpp
    file_known_met.cpp
)

set_target_properties(tools_emule_known_met PROPERTIES OUTPUT_NAME "emule_known_met")
target_link_libraries(tools_emule_known_met PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: emule_part_met
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_emule_part_met
    tools/emule_part_met.cpp
    ctag.cpp
    file_part_met.cpp
)

set_target_properties(tools_emule_known_met PROPERTIES OUTPUT_NAME "emule_part_met")
target_link_libraries(tools_emule_part_met PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: emule_part_met_txtsrc
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_emule_part_met_txtsrc
    tools/emule_part_met_txtsrc.cpp
    file_part_met_txtsrc.cpp
)

set_target_properties(tools_emule_known_met PROPERTIES OUTPUT_NAME "emule_part_met_txtsrc")
target_link_libraries(tools_emule_part_met_txtsrc PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: emule_stored_searches_met
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_emule_stored_searches_met
    tools/emule_stored_searches_met.cpp
    ctag.cpp
    file_stored_searches_met.cpp
)

set_target_properties(tools_emule_known_met PROPERTIES OUTPUT_NAME "emule_stored_searches")
target_link_libraries(tools_emule_stored_searches_met PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS
    app_emule LIBRARY DESTINATION share/mobiusft/extensions
)

install(TARGETS
    tools_emule_cancelled_met
    tools_emule_key_index_dat
    tools_emule_known_met
    tools_emule_part_met
    tools_emule_part_met_txtsrc
    tools_emule_stored_searches_met
    RUNTIME DESTINATION share/mobiusft/tools)
