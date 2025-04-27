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
cmake_minimum_required(VERSION 3.20)
project(libmobius-framework LANGUAGES CXX)

include(GNUInstallDirs)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Define top-level source and build directories (@todo move to main CMakeLists.txt)
set(TOP_SRC_DIR ${CMAKE_SOURCE_DIR}/../..)
set(TOP_BUILD_DIR ${CMAKE_BINARY_DIR}/../..)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Find dependencies
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
find_library(MOBIUS_CORE_LIB mobius_core
    HINTS ../libmobius_core/build
    NO_DEFAULT_PATH
)
if(NOT MOBIUS_CORE_LIB)
    message(FATAL_ERROR "libmobius_core not found in ../libmobius_core/build")
endif()

find_library(MOBIUS_LIB mobius HINTS ${TOP_SRC_DIR}/src/mobius/.libs NO_DEFAULT_PATH)
if(NOT MOBIUS_LIB)
    message(FATAL_ERROR "libmobius not found in ${TOP_SRC_DIR}/src/mobius/.libs")
endif()

set(MOBIUS_INCLUDE_DIRS ${TOP_SRC_DIR}/include ${TOP_SRC_DIR}/src)
set(MOBIUS_LIBRARIES ${MOBIUS_CORE_LIB} ${MOBIUS_LIB})

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Build sub-libraries
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(mobius_framework SHARED
    attribute.cpp
    category.cpp
    config.cpp
    evidence_flag.cpp
    evidence_loader.cpp
)

target_include_directories(mobius_framework PUBLIC ${MOBIUS_INCLUDE_DIRS})

target_link_libraries(mobius_framework PUBLIC
    ${MOBIUS_LIBRARIES}
)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS mobius_framework
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
