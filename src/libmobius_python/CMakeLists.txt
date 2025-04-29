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

# Set C++ standard (@todo move to src/CMakeLists.txt)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Define top-level source and build directories (@todo move to main CMakeLists.txt)
set(TOP_SRC_DIR ${CMAKE_SOURCE_DIR}/../..)
set(TOP_BUILD_DIR ${CMAKE_BINARY_DIR}/../..)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Find dependencies (@todo move to src/CMakeLists.txt)
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
find_program(Python3_EXECUTABLE NAMES python3 REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

set(PYTHON_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages")
message(STATUS "PYTHON_INSTALL_DIR: ${PYTHON_INSTALL_DIR}")

find_library(MOBIUS_LIB mobius HINTS ${TOP_SRC_DIR}/src/mobius/.libs NO_DEFAULT_PATH)

if(NOT MOBIUS_LIB)
    message(FATAL_ERROR "libmobius not found in ${TOP_SRC_DIR}/src/mobius/.libs")
endif()

find_library(MOBIUS_CORE_LIB mobius_core
    HINTS ../libmobius_core/build
    NO_DEFAULT_PATH
)
if(NOT MOBIUS_CORE_LIB)
    message(FATAL_ERROR "libmobius_core not found in ../libmobius_core/build")
endif()

find_library(MOBIUS_FRAMEWORK_LIB mobius_framework
    HINTS ../libmobius_framework/build
    NO_DEFAULT_PATH
)
if(NOT MOBIUS_FRAMEWORK_LIB)
    message(FATAL_ERROR "libmobius_framework not found in ../libmobius_framework/build")
endif()

set(MOBIUS_INCLUDE_DIRS ${TOP_SRC_DIR}/include ${TOP_SRC_DIR}/src ${TOP_SRC_DIR}/src/libmobius_python)
set(MOBIUS_LIBRARIES ${MOBIUS_LIB} ${MOBIUS_CORE_LIB} ${MOBIUS_FRAMEWORK_LIB})

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
add_subdirectory(model)
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
    ${CMAKE_SOURCE_DIR}
    ${Python3_INCLUDE_DIRS}
)

target_compile_options(mobius PRIVATE -fno-strict-aliasing -flto)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

target_link_libraries(mobius PRIVATE
    ${MOBIUS_LIBRARIES}
    ${Python_LIBRARIES}
)

set_target_properties(mobius PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)

target_link_libraries(mobius PRIVATE
    libmobius-python-core
    libmobius-python-crypt
    libmobius-python-database
    libmobius-python-datasource
    libmobius-python-datetime
    libmobius-python-decoder
    libmobius-python-encoder
    libmobius-python-framework
    libmobius-python-io
    libmobius-python-kff
    libmobius-python-model
    libmobius-python-os
    libmobius-python-pod
    libmobius-python-system
    libmobius-python-turing
    mobius_python_ui
    libmobius-python-vfs
    ${MOBIUS_LIBRARIES}
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
    LIBRARY DESTINATION ${PYTHON_INSTALL_DIR}
    RUNTIME DESTINATION ${PYTHON_INSTALL_DIR}
)
