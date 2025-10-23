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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Find zlib
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
find_package(ZLIB QUIET)

if(ZLIB_FOUND)
    message(STATUS "Found zlib: ${ZLIB_LIBRARIES}")
else()
    message(SEND_ERROR "zlib not found. See www.zlib.net")
endif()

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Find libdl
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
find_package(dl QUIET)

if(TARGET dl::dl)
    message(STATUS "Using CMake's built-in dl package")
    set(DL_LIBRARY dl::dl)
else()
    find_library(DL_LIBRARY dl)
    if(DL_LIBRARY)
        message(STATUS "Found libdl: ${DL_LIBRARY}")
    else()
        message(FATAL_ERROR "libdl not found")
    endif()
endif()

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Find libiconv
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
find_package(Iconv QUIET)

if(Iconv_FOUND)
    message(STATUS "Found libiconv: ${Iconv_LIBRARIES}")
else()
    message(FATAL_ERROR "Libiconv not found.")
endif()

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Build sub-libraries
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_subdirectory(crypt)
add_subdirectory(database)
add_subdirectory(datasource)
add_subdirectory(datetime)
add_subdirectory(decoder)
add_subdirectory(encoder)
add_subdirectory(file_decoder)
add_subdirectory(io)
add_subdirectory(kff)
add_subdirectory(os)
add_subdirectory(pod)
add_subdirectory(system)
add_subdirectory(turing)
add_subdirectory(ui)
add_subdirectory(vfs)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
configure_file(application.cpp.in application.cpp @ONLY)

add_library(mobius_core SHARED
    ${CMAKE_CURRENT_BINARY_DIR}/application.cpp
    benchmark.cpp
    bytearray.cpp
    charset.cpp
    exception.cpp
    extension.cpp
    extension_set.cpp
    log.cpp
    mediator.cpp
    metadata.cpp
    resource.cpp
    string_functions.cpp
    thread_guard.cpp
    unittest.cpp
    zlib_functions.cpp
)

target_include_directories(mobius_core PUBLIC ${MOBIUS_INCLUDE_DIRS})

target_link_libraries(mobius_core PRIVATE
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_crypt>"
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_database>"
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_datasource>"
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_datetime>"
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_decoder>"
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_encoder>"
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_file_decoder>"
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_io>"
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_kff>"
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_os>"
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_pod>"
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_system>"
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_turing>"
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_ui>"
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,mobius_core_vfs>"
)

target_link_libraries(mobius_core PUBLIC
    ${DL_LIBRARY}
    ZLIB::ZLIB
    Iconv::Iconv
)

add_library(Mobius::Core ALIAS mobius_core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS mobius_core
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
