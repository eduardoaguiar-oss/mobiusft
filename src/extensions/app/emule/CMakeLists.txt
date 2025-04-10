# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
set(SOURCES
    main.cpp
    ctag.cpp
    file_cancelled_met.cpp
    file_known_met.cpp
    file_stored_searches_met.cpp
    evidence_loader_impl.cpp
)

add_library(app_emule MODULE ${SOURCES})
set_target_properties(app_emule PROPERTIES OUTPUT_NAME "app-emule" PREFIX "")
target_include_directories(app_emule PRIVATE ${MOBIUS_INCLUDE_DIRS})
target_link_libraries(app_emule PRIVATE ${MOBIUS_LIBRARIES})

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: emule_cancelled_met
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
set(STORED_CANCELLED_MET_SOURCES
    tools/emule_cancelled_met.cpp
    ctag.cpp
    file_cancelled_met.cpp
)

add_executable(emule_cancelled_met ${STORED_CANCELLED_MET_SOURCES})
target_link_libraries(emule_cancelled_met PRIVATE ${MOBIUS_LIBRARIES})
target_include_directories(emule_cancelled_met PRIVATE ${MOBIUS_INCLUDE_DIRS})

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: emule_known_met
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
set(STORED_KNOWN_MET_SOURCES
    tools/emule_known_met.cpp
    ctag.cpp
    file_known_met.cpp
)

add_executable(emule_known_met ${STORED_KNOWN_MET_SOURCES})
target_link_libraries(emule_known_met PRIVATE ${MOBIUS_LIBRARIES})
target_include_directories(emule_known_met PRIVATE ${MOBIUS_INCLUDE_DIRS})

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: emule_stored_searches_met
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
set(STORED_SEARCHES_MET_SOURCES
    tools/emule_stored_searches_met.cpp
    ctag.cpp
    file_stored_searches_met.cpp
)

add_executable(emule_stored_searches_met ${STORED_SEARCHES_MET_SOURCES})
target_link_libraries(emule_stored_searches_met PRIVATE ${MOBIUS_LIBRARIES})
target_include_directories(emule_stored_searches_met PRIVATE ${MOBIUS_INCLUDE_DIRS})

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS
    app_emule LIBRARY DESTINATION share/mobiusft/extensions
)

install(TARGETS
    emule_cancelled_met
    emule_known_met
    emule_stored_searches_met
    RUNTIME DESTINATION share/mobiusft/tools)
