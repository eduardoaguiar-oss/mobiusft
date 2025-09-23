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
cmake_minimum_required(VERSION 3.20)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Project
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
set(MOBIUS_VERSION 2.20)
set(MOBIUS_DESCRIPTION "Mobius Forensic Toolkit")

project(mobius
    VERSION ${MOBIUS_VERSION}
    DESCRIPTION ${MOBIUS_DESCRIPTION}
    LANGUAGES CXX
)

include(GNUInstallDirs)

set(MOBIUS_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(MOBIUS_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)
set(MOBIUS_SHARE_DIR "${CMAKE_INSTALL_DATADIR}/mobiusft" CACHE PATH "Installation directory for share/mobiusft")

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Set MOBIUS_PYTHON_DIR
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
find_program(Python3_EXECUTABLE NAMES python3 REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

set(MOBIUS_PYTHON_DIR "${CMAKE_INSTALL_LIBDIR}/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages")
message(STATUS "MOBIUS_PYTHON_DIR: ${MOBIUS_PYTHON_DIR}")

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Build modules
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_subdirectory(data)
add_subdirectory(docs)
add_subdirectory(include)
add_subdirectory(src)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/mobius_bin.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/mobius_bin.sh
    @ONLY
    NEWLINE_STYLE UNIX
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

install(
    FILES mobius_bin.py
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    PERMISSIONS 
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/mobius_bin.sh
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Package definition
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
set(CPACK_PACKAGE_NAME "mobiusft")
set(CPACK_PACKAGE_VENDOR "Eduardo Aguiar")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${MOBIUS_DESCRIPTION}")
set(CPACK_PACKAGE_VERSION "${MOBIUS_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_SOURCE_GENERATOR "TXZ;ZIP")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_SOURCE_STRIP_FILES TRUE)
set(CPACK_SOURCE_IGNORE_FILES
    "/\\\\.git/"          # Git directory
    "\\\\.gitignore"
    "\\\\.vscode/"
    "\\\\.idea/"
    "mobius.code-workspace"
    "FEATURES.md"
    "screenshots/"
)

include(CPack)
