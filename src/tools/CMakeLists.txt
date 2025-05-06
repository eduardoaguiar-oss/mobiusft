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
project(mobius_tools LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: casetree - Builds a case tree structure
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_casetree casetree.cpp)
set_target_properties(tools_casetree PROPERTIES OUTPUT_NAME "casetree")
target_link_libraries(tools_casetree PRIVATE Mobius::Framework Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: dirfs - Directory filesystem operations
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_dirfs dirfs.cpp)
set_target_properties(tools_dirfs PROPERTIES OUTPUT_NAME "dirfs")
target_link_libraries(tools_dirfs PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: dirtree - Displays directory tree
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_dirtree dirtree.cpp)
set_target_properties(tools_dirtree PROPERTIES OUTPUT_NAME "dirtree")
target_link_libraries(tools_dirtree PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: filesystem_scan - Scans filesystem metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_filesystem_scan filesystem_scan.cpp)
set_target_properties(tools_filesystem_scan PROPERTIES OUTPUT_NAME "filesystem_scan")
target_link_libraries(tools_filesystem_scan PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: hashfs - Hashes each file from disk
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_hashfs hashfs.cpp)
set_target_properties(tools_hashfs PROPERTIES OUTPUT_NAME "hashfs")
target_link_libraries(tools_hashfs PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: hive_info - Displays Windows Registry hive info
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_hive_info hive_info.cpp)
set_target_properties(tools_hive_info PROPERTIES OUTPUT_NAME "hive_info")
target_link_libraries(tools_hive_info PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: hive_scan - Scans Windows Registry hives
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_hive_scan hive_scan.cpp)
set_target_properties(tools_hive_scan PROPERTIES OUTPUT_NAME "hive_scan")
target_link_libraries(tools_hive_scan PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: imagefile_cat - Print image file to the standard output
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_imagefile_cat imagefile_cat.cpp)
set_target_properties(tools_imagefile_cat PROPERTIES OUTPUT_NAME "imagefile_cat")
target_link_libraries(tools_imagefile_cat PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: imagefile_convert - Converts image file formats
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_imagefile_convert imagefile_convert.cpp)
set_target_properties(tools_imagefile_convert PROPERTIES OUTPUT_NAME "imagefile_convert")
target_link_libraries(tools_imagefile_convert PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: imagefile_info - Displays image file info
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_imagefile_info imagefile_info.cpp)
set_target_properties(tools_imagefile_info PROPERTIES OUTPUT_NAME "imagefile_info")
target_link_libraries(tools_imagefile_info PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: list_resources - Lists resources
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_list_resources list_resources.cpp)
set_target_properties(tools_list_resources PROPERTIES OUTPUT_NAME "list_resources")
target_link_libraries(tools_list_resources PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: torrent_info - Displays torrent file info
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_torrent_info torrent_info.cpp)
set_target_properties(tools_torrent_info PROPERTIES OUTPUT_NAME "torrent_info")
target_link_libraries(tools_torrent_info PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: vfs_info - Displays virtual filesystem info
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_vfs_info vfs_info.cpp)
set_target_properties(tools_vfs_info PROPERTIES OUTPUT_NAME "vfs_info")
target_link_libraries(tools_vfs_info PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: device_list - Lists devices using libudev 
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_device_list device_list.cpp)
set_target_properties(tools_device_list PROPERTIES OUTPUT_NAME "device_list")
target_link_libraries(tools_device_list PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: disk_list - Lists disks using libudev
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_disk_list disk_list.cpp)
set_target_properties(tools_disk_list PROPERTIES OUTPUT_NAME "disk_list")
target_link_libraries(tools_disk_list PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS
    tools_casetree
    tools_device_list
    tools_dirfs
    tools_dirtree
    tools_disk_list
    tools_filesystem_scan
    tools_hashfs
    tools_hive_info
    tools_hive_scan
    tools_imagefile_cat
    tools_imagefile_convert
    tools_imagefile_info
    tools_list_resources
    tools_torrent_info
    tools_vfs_info
    DESTINATION share/mobiusft/tools
)
