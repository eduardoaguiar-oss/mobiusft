# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008-2026 Eduardo Aguiar
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
set(LIBTSK_ROOT "" CACHE PATH "Custom libtsk installation path")
if(LIBTSK_ROOT)
  list(APPEND CMAKE_PREFIX_PATH "${LIBTSK_ROOT}")
endif()

if(LIBTSK_ROOT)
    set(TSK_SEARCH_PATHS ${LIBTSK_ROOT}/lib ${LIBTSK_ROOT}/lib64)
    set(TSK_INCLUDE_PATHS ${LIBTSK_ROOT}/include)
else()
    set(TSK_SEARCH_PATHS /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64)
    set(TSK_INCLUDE_PATHS /usr/include /usr/local/include)
endif()

find_library(TSK_LIBRARIES
    NAMES tsk libtsk
    PATHS ${TSK_SEARCH_PATHS}
    NO_DEFAULT_PATH
)
find_path(TSK_INCLUDE_DIRS
    NAMES tsk/libtsk.h
    PATHS ${TSK_INCLUDE_PATHS}
    NO_DEFAULT_PATH
)

if(NOT TSK_LIBRARIES OR NOT TSK_INCLUDE_DIRS)
    find_library(TSK_LIBRARIES
        NAMES tsk libtsk
        PATHS /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64
    )
    find_path(TSK_INCLUDE_DIRS
        NAMES tsk/libtsk.h
        PATHS /usr/include /usr/local/include
    )
endif()

if(TSK_LIBRARIES AND TSK_INCLUDE_DIRS)
    message(STATUS "Found libtsk:")
    message(STATUS "  Library: ${TSK_LIBRARIES}")
    message(STATUS "  Include dir: ${TSK_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "libtsk not found! Please install libtsk-devel or specify -DLIBTSK_ROOT=DIR.")
endif()

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
    ${TSK_LIBRARIES}
)

target_compile_options(mobius_core_vfs PRIVATE -fno-strict-aliasing -fPIC)
