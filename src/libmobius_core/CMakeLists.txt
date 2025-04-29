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
project(libmobius-core LANGUAGES CXX)

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
# Find dependencies
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# find_package(PkgConfig REQUIRED)

# libz
find_package(ZLIB QUIET)
if(ZLIB_FOUND)
    message(STATUS "zlib found: ${ZLIB_LIBRARIES}")
else()
    message(SEND_ERROR "zlib not found. See www.zlib.net")
endif()

# libdl
find_package(dl QUIET)

if(TARGET dl::dl)
    message(STATUS "Using CMake's built-in dl package")
    set(DL_LIBRARY dl::dl)
else()
    find_library(DL_LIBRARY dl)
    if(DL_LIBRARY)
        message(STATUS "Found libdl: ${DL_LIBRARY}")
    else()
        message(SEND_ERROR "libdl not found")
    endif()
endif()

# old mobius library
find_library(MOBIUS_LIB mobius HINTS ${TOP_SRC_DIR}/src/mobius/.libs NO_DEFAULT_PATH)

if(NOT MOBIUS_LIB)
    message(FATAL_ERROR "libmobius not found in ${TOP_SRC_DIR}/src/mobius/.libs")
endif()

set(MOBIUS_INCLUDE_DIRS ${TOP_SRC_DIR}/include ${TOP_SRC_DIR}/src)
set(MOBIUS_LIBRARIES ${MOBIUS_LIB})

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Build sub-libraries
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_subdirectory(datasource)
add_subdirectory(encoder)
add_subdirectory(file_decoder)
add_subdirectory(jni)
add_subdirectory(kff)
add_subdirectory(turing)
add_subdirectory(ui)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
set(MOBIUS_VERSION "2.15") # @todo move to root/CMakeLists.txt
configure_file(application.cpp.in application.cpp @ONLY)

add_library(mobius_core SHARED
    ${CMAKE_CURRENT_BINARY_DIR}/application.cpp
    benchmark.cpp
    extension.cpp
    extension_set.cpp
    log.cpp
    mediator.cpp
    metadata.cpp
    resource.cpp
    thread_guard.cpp
    unittest.cpp
    zlib_functions.cpp
    $<TARGET_OBJECTS:mobius_core_datasource>
    $<TARGET_OBJECTS:mobius_core_encoder>
    $<TARGET_OBJECTS:mobius_core_file_decoder>
    $<TARGET_OBJECTS:mobius_core_jni>
    $<TARGET_OBJECTS:mobius_core_kff>
    $<TARGET_OBJECTS:mobius_core_turing>
    $<TARGET_OBJECTS:mobius_core_ui>
)

target_include_directories(mobius_core PUBLIC ${MOBIUS_INCLUDE_DIRS} ${TOP_SRC_DIR})

target_link_libraries(mobius_core PUBLIC
    ${MOBIUS_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${JNI_LIBRARIES}
    ${DL_LIBRARY}
)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS mobius_core
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
