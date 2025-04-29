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
project(libmobius-core-vfs LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Find libtsk
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
if(LIBTSK_DIR)
    set(TSK_SEARCH_PATHS ${LIBTSK_DIR}/lib ${LIBTSK_DIR}/lib64)
    set(TSK_INCLUDE_PATHS ${LIBTSK_DIR}/include)
else()
    set(TSK_SEARCH_PATHS /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64)
    set(TSK_INCLUDE_PATHS /usr/include /usr/local/include)
endif()

find_library(TSK_LIBRARY
    NAMES tsk libtsk
    PATHS ${TSK_SEARCH_PATHS}
    NO_DEFAULT_PATH
)
find_path(TSK_INCLUDE_DIR
    NAMES tsk/libtsk.h
    PATHS ${TSK_INCLUDE_PATHS}
    NO_DEFAULT_PATH
)

if(NOT TSK_LIBRARY OR NOT TSK_INCLUDE_DIR)
    find_library(TSK_LIBRARY
        NAMES tsk libtsk
        PATHS /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64
    )
    find_path(TSK_INCLUDE_DIR
        NAMES tsk/libtsk.h
        PATHS /usr/include /usr/local/include
    )
endif()

if(TSK_LIBRARY AND TSK_INCLUDE_DIR)
    message(STATUS "Found libtsk:")
    message(STATUS "  Library: ${TSK_LIBRARY}")
    message(STATUS "  Include dir: ${TSK_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "libtsk not found! Please install libtsk-devel or specify --with-libtsk=DIR.")
endif()

add_library(TSK::TSK IMPORTED SHARED)
set_target_properties(TSK::TSK PROPERTIES
    IMPORTED_LOCATION "${TSK_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${TSK_INCLUDE_DIR}"
)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(mobius_core_vfs STATIC
    block.cpp
    block_impl_disk.cpp
    block_impl_slice.cpp
    disk.cpp
    disk_impl_device.cpp
    disk_impl_imagefile.cpp
    filesystem.cpp
    imagefile.cpp
    segment_array.cpp
    tsk/adaptor.cpp
    tsk/exception.cpp
    tsk/file_impl.cpp
    tsk/folder_impl.cpp
    tsk/fs_file.cpp
    tsk/reader_impl_file.cpp
    tsk/reader_impl_stream.cpp
    tsk/stream_impl.cpp
    util.cpp
    vfs.cpp
)

target_include_directories(mobius_core_vfs PRIVATE
    ${MOBIUS_INCLUDE_DIRS}
    ${TSK_INCLUDE_DIRS}
)

target_link_libraries(mobius_core_vfs PUBLIC
    TSK::TSK
)

target_compile_options(mobius_core_vfs PRIVATE -fno-strict-aliasing -fPIC)
