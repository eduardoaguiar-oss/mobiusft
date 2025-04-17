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
project(extensions LANGUAGES CXX)

# Set C++ standard (@todo move to main CMakeLists.txt)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define top-level source and build directories (@todo move to main CMakeLists.txt)
set(TOP_SRC_DIR ${CMAKE_SOURCE_DIR}/../..)
set(TOP_BUILD_DIR ${CMAKE_BINARY_DIR}/../..)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Find dependencies (@todo move to main CMakeLists.txt)
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

find_package(ZLIB QUIET)  # QUIET suppresses warnings if not found
if(ZLIB_FOUND)
    message(STATUS "zlib found: ${ZLIB_LIBRARIES}")
else()
    message(WARNING "zlib not found; skipping submodules requiring libz")
endif()

find_library(MOBIUS_LIB mobius HINTS ${TOP_SRC_DIR}/src/mobius/.libs NO_DEFAULT_PATH)
find_library(MOBIUS_FRAMEWORK_LIB mobius_framework HINTS ${TOP_SRC_DIR}/src/mobius/framework/.libs NO_DEFAULT_PATH)

if(NOT MOBIUS_LIB)
    message(FATAL_ERROR "libmobius not found in ${TOP_SRC_DIR}/src/mobius/.libs")
endif()
if(NOT MOBIUS_FRAMEWORK_LIB)
    message(FATAL_ERROR "libmobius_framework not found in ${TOP_SRC_DIR}/src/mobius/framework/.libs")
endif()

set(MOBIUS_INCLUDE_DIRS ${TOP_SRC_DIR}/src)
set(MOBIUS_LIBRARIES ${MOBIUS_LIB} ${MOBIUS_FRAMEWORK_LIB})

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Build extensions
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_subdirectory(app/ares)
add_subdirectory(app/emule)
add_subdirectory(app/emuletorrent)
add_subdirectory(app/shareaza)
add_subdirectory(attribute-viewer)
add_subdirectory(category-manager)
add_subdirectory(datasource-ufdr)
add_subdirectory(datasource-vfs)
add_subdirectory(datasource-viewer)
add_subdirectory(date-code)
add_subdirectory(event-viewer)
add_subdirectory(evidence-password-hashes)
add_subdirectory(evidence-viewer)
add_subdirectory(extension-manager)
add_subdirectory(file-explorer)
add_subdirectory(floppy-imager)
add_subdirectory(gigatribe-agent)
add_subdirectory(gtk-ui)
add_subdirectory(hive)
add_subdirectory(hive-report)
add_subdirectory(ice)
add_subdirectory(imagefile-dnd)
add_subdirectory(iped-frontend)
add_subdirectory(kff-manager)
add_subdirectory(lshw-agent)
add_subdirectory(part-catalogue)
add_subdirectory(part-model)
add_subdirectory(report-model)
add_subdirectory(report-wizard)
add_subdirectory(sdi-window-manager)
add_subdirectory(skype-agent)
add_subdirectory(skype-agent-sqlite)
add_subdirectory(turing)
add_subdirectory(ui/gtk3)
add_subdirectory(vfs-block-view-bitlocker)
add_subdirectory(vfs-block-view-partition-system)
add_subdirectory(vfs-item-attributes)
add_subdirectory(vfs/block/bitlocker)
add_subdirectory(vfs/block/filesystems)
add_subdirectory(vfs/block/partition_system_apm)
add_subdirectory(vfs/block/partition_system_dos)
add_subdirectory(vfs/block/partition_system_gpt)
add_subdirectory(vfs/filesystem/exfat)
add_subdirectory(vfs/filesystem/ext2)
add_subdirectory(vfs/filesystem/hfs)
add_subdirectory(vfs/filesystem/iso)
add_subdirectory(vfs/filesystem/ntfs)
add_subdirectory(vfs/filesystem/vfat)
add_subdirectory(vfs/imagefile/dossier)
add_subdirectory(vfs/imagefile/msr)
add_subdirectory(vfs/imagefile/raw)
add_subdirectory(vfs/imagefile/solo)
add_subdirectory(vfs/imagefile/split)
add_subdirectory(vfs/imagefile/talon)
add_subdirectory(vfs/imagefile/vhd)
add_subdirectory(vfs/imagefile/vhdx)

if(ZLIB_FOUND)
  add_subdirectory(vfs/imagefile/ewf)
endif()
