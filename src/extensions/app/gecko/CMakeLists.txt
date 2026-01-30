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
cmake_minimum_required(VERSION 3.20)
project(app_gecko LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(app_gecko MODULE
    main.cpp
    common.cpp
    file_cookies_sqlite.cpp
    file_downloads_sqlite.cpp
    file_formhistory_sqlite.cpp
    profile.cpp
    vfs_processor_impl.cpp
)

set_target_properties(app_gecko PROPERTIES OUTPUT_NAME "app-gecko" PREFIX "")

target_link_libraries(app_gecko PRIVATE Mobius::Framework Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: gecko_cookies
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_gecko_cookies
    tools/gecko_cookies.cpp
    common.cpp
    file_cookies_sqlite.cpp
)

set_target_properties(tools_gecko_cookies PROPERTIES OUTPUT_NAME "gecko_cookies")
target_link_libraries(tools_gecko_cookies PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: gecko_downloads
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_gecko_downloads
    tools/gecko_downloads.cpp
    common.cpp
    file_downloads_sqlite.cpp
)

set_target_properties(tools_gecko_downloads PROPERTIES OUTPUT_NAME "gecko_downloads")
target_link_libraries(tools_gecko_downloads PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: gecko_formhistory
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_gecko_formhistory
    tools/gecko_formhistory.cpp
    common.cpp
    file_formhistory_sqlite.cpp
)

set_target_properties(tools_gecko_formhistory PROPERTIES OUTPUT_NAME "gecko_formhistory")
target_link_libraries(tools_gecko_formhistory PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS app_gecko LIBRARY DESTINATION share/mobiusft/extensions)

install(TARGETS
    tools_gecko_cookies
    tools_gecko_formhistory
    RUNTIME DESTINATION share/mobiusft/tools)