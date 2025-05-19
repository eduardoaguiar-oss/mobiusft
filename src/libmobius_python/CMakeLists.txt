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
project(LibMobiusPython LANGUAGES CXX)

include(GNUInstallDirs)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(MOBIUS_PYTHON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Find dependencies
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
find_program(Python3_EXECUTABLE NAMES python3 REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Build sub-libraries
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_subdirectory(core)
add_subdirectory(framework)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target: 'mobius' python module
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(libmobius_python MODULE
    module.cpp
    pycallback.cpp
    pyfunction.cpp
    pymodule.cpp
    pyobject.cpp
    pymobius.cpp
    pytypeobject.cpp
    api_dataholder.cpp
    api_metadata.cpp
)

target_include_directories(libmobius_python PRIVATE
    ${MOBIUS_PYTHON_INCLUDE_DIRS}
    ${Python3_INCLUDE_DIRS}
)

target_compile_options(libmobius_python PRIVATE -fno-strict-aliasing -flto)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set_target_properties(libmobius_python PROPERTIES
    PREFIX ""
    OUTPUT_NAME "mobius"
    SUFFIX ".so"
)

target_link_libraries(libmobius_python PRIVATE
    libmobius_python_core
    libmobius_python_framework
    Python3::Module
)

if(UNIX AND NOT APPLE)
    target_link_options(libmobius_python PRIVATE -Wl,--export-dynamic-symbol=PyInit_mobius)
elseif(APPLE)
    target_link_options(libmobius_python PRIVATE -Wl,-exported_symbol,_PyInit_mobius)
endif()

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS libmobius_python
    LIBRARY DESTINATION ${MOBIUS_PYTHON_DIR}
    RUNTIME DESTINATION ${MOBIUS_PYTHON_DIR}
)
