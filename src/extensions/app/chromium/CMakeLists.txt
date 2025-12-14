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
project(app_chromium LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(app_chromium MODULE
    main.cpp
    common.cpp
    file_bookmarks.cpp
    file_cookies.cpp
    file_history.cpp
    file_local_state.cpp
    file_login_data.cpp
    file_preferences.cpp
    file_web_data.cpp
    post_processor_impl.cpp
    profile.cpp
    vfs_processor_impl.cpp
)

set_target_properties(app_chromium PROPERTIES OUTPUT_NAME "app-chromium" PREFIX "")
target_link_libraries(app_chromium PRIVATE Mobius::Framework Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: chromium_bookmarks
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_chromium_bookmarks
    tools/chromium_bookmarks.cpp
    common.cpp
    file_bookmarks.cpp
)

set_target_properties(tools_chromium_bookmarks PROPERTIES OUTPUT_NAME "chromium_bookmarks")
target_link_libraries(tools_chromium_bookmarks PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: chromium_cookies
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_chromium_cookies
    tools/chromium_cookies.cpp
    common.cpp
    file_cookies.cpp
)

set_target_properties(tools_chromium_cookies PROPERTIES OUTPUT_NAME "chromium_cookies")
target_link_libraries(tools_chromium_cookies PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: chromium_history
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_chromium_history
    tools/chromium_history.cpp
    common.cpp
    file_history.cpp
)

set_target_properties(tools_chromium_history PROPERTIES OUTPUT_NAME "chromium_history")
target_link_libraries(tools_chromium_history PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: chromium_local_state
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_chromium_local_state
    tools/chromium_local_state.cpp
    common.cpp
    file_local_state.cpp
)

set_target_properties(tools_chromium_local_state PROPERTIES OUTPUT_NAME "chromium_local_state")
target_link_libraries(tools_chromium_local_state PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: chromium_login_data
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_chromium_login_data
    tools/chromium_login_data.cpp
    common.cpp
    file_login_data.cpp
)

set_target_properties(tools_chromium_login_data PROPERTIES OUTPUT_NAME "chromium_login_data")
target_link_libraries(tools_chromium_login_data PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: chromium_preferences
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_chromium_preferences
    tools/chromium_preferences.cpp
    common.cpp
    file_preferences.cpp
)

set_target_properties(tools_chromium_preferences PROPERTIES OUTPUT_NAME "chromium_preferences")
target_link_libraries(tools_chromium_preferences PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: chromium_web_data
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_chromium_web_data
    tools/chromium_web_data.cpp
    common.cpp
    file_web_data.cpp
)

set_target_properties(tools_chromium_web_data PROPERTIES OUTPUT_NAME "chromium_web_data")
target_link_libraries(tools_chromium_web_data PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS
    app_chromium LIBRARY DESTINATION share/mobiusft/extensions
)

install(TARGETS
    tools_chromium_cookies
    tools_chromium_history
    tools_chromium_login_data
    tools_chromium_preferences
    tools_chromium_web_data
    RUNTIME DESTINATION share/mobiusft/tools
)
