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
project(datasource_ufdr LANGUAGES NONE)  # No compiled language needed

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file(GLOB RESOURCE_FILES resource/*.png)

set(SOURCE_FILES
    main.py
    metadata.py
    caseinfo_view.py
    extractions_view.py
    fileinfo_view.py
    icon.png
    ${RESOURCE_FILES}
)

# Custom command to zip files into datasource-ufdr.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/datasource-ufdr.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/datasource-ufdr.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into datasource-ufdr.mobius"
)

# Custom target to trigger the zip
add_custom_target(datasource_ufdr ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/datasource-ufdr.mobius)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/datasource-ufdr.mobius
        DESTINATION share/mobiusft/extensions)
