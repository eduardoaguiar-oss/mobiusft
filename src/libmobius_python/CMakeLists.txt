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
cmake_minimum_required(VERSION 3.15)
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
add_subdirectory(crypt)
add_subdirectory(database)
add_subdirectory(datasource)
add_subdirectory(datetime)
add_subdirectory(decoder)
add_subdirectory(encoder)
add_subdirectory(framework)
add_subdirectory(io)
add_subdirectory(kff)
add_subdirectory(os)
add_subdirectory(pod)
add_subdirectory(system)
add_subdirectory(turing)
add_subdirectory(ui)
add_subdirectory(vfs)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target: 'mobius' python module
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(mobius MODULE
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

target_include_directories(mobius PRIVATE
    ${MOBIUS_INCLUDE_DIRS}
    ${MOBIUS_PYTHON_INCLUDE_DIRS}
    ${Python3_INCLUDE_DIRS}
)

target_compile_options(mobius PRIVATE -fno-strict-aliasing -flto)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

target_link_libraries(mobius PRIVATE
    mobius_core
    mobius_framework
    ${Python_LIBRARIES}
)

set_target_properties(mobius PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)

target_link_libraries(mobius PRIVATE
    mobius_python_core
    mobius_python_crypt
    mobius_python_database
    mobius_python_datasource
    mobius_python_datetime
    mobius_python_decoder
    mobius_python_encoder
    mobius_python_framework
    mobius_python_io
    mobius_python_kff
    mobius_python_os
    mobius_python_pod
    mobius_python_system
    mobius_python_turing
    mobius_python_ui
    mobius_python_vfs
    Mobius::Core
    Mobius::Framework
    Python3::Module
)

if(UNIX AND NOT APPLE)
    target_link_options(mobius PRIVATE -Wl,--export-dynamic-symbol=PyInit_mobius)
elseif(APPLE)
    target_link_options(mobius PRIVATE -Wl,-exported_symbol,_PyInit_mobius)
endif()

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS mobius
    LIBRARY DESTINATION ${MOBIUS_PYTHON_DIR}
    RUNTIME DESTINATION ${MOBIUS_PYTHON_DIR}
)
