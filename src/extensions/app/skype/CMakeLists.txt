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
cmake_minimum_required(VERSION 3.25)
project(app_skype LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(app_skype MODULE
    main.cpp
    common.cpp
    file_main_db.cpp
    file_s4l_db.cpp
    file_skype_db.cpp
    message_parser.cpp
    profile.cpp
    vfs_processor_impl.cpp
)

set_target_properties(app_skype PROPERTIES OUTPUT_NAME "app-skype" PREFIX "")
target_link_libraries(app_skype PRIVATE Mobius::Framework Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: skype_main_db
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_skype_main_db
    tools/skype_main_db.cpp
    common.cpp
    file_main_db.cpp
    message_parser.cpp
)

set_target_properties(tools_skype_main_db PROPERTIES OUTPUT_NAME "skype_main_db")
target_link_libraries(tools_skype_main_db PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: skype_s4l_db
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_skype_s4l_db
    tools/skype_s4l_db.cpp
    common.cpp
    file_s4l_db.cpp
    message_parser.cpp
)

set_target_properties(tools_skype_s4l_db PROPERTIES OUTPUT_NAME "skype_s4l_db")
target_link_libraries(tools_skype_s4l_db PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: skype_skype_db
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_skype_skype_db
    tools/skype_skype_db.cpp
    common.cpp
    file_skype_db.cpp
    message_parser.cpp
)

set_target_properties(tools_skype_skype_db PROPERTIES OUTPUT_NAME "skype_skype_db")
target_link_libraries(tools_skype_skype_db PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS
    app_skype LIBRARY DESTINATION share/mobiusft/extensions
)

install(TARGETS
    tools_skype_main_db
    tools_skype_s4l_db
    tools_skype_skype_db
    RUNTIME DESTINATION share/mobiusft/tools
)

