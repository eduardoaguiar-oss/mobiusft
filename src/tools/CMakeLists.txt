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
# Find dependencies
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
find_library(MOBIUS_CORE_LIB mobius_core
    HINTS ../libmobius_core/build
    NO_DEFAULT_PATH
)
if(NOT MOBIUS_CORE_LIB)
    message(FATAL_ERROR "libmobius_core not found in ../libmobius_core/build")
endif()

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: casetree - Builds a case tree structure
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(casetree casetree.cpp)
target_include_directories(casetree PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(casetree PRIVATE mobius_core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: dirfs - Directory filesystem operations
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(dirfs dirfs.cpp)
target_link_libraries(dirfs PRIVATE mobius_core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: dirtree - Displays directory tree
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(dirtree dirtree.cpp)
target_include_directories(dirtree PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(dirtree PRIVATE mobius_core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: filesystem_scan - Scans filesystem metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(filesystem_scan filesystem_scan.cpp)
target_include_directories(filesystem_scan PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(filesystem_scan PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB})
set_target_properties(filesystem_scan PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: hashfs - Hashes each file from disk
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(hashfs hashfs.cpp)
target_include_directories(hashfs PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(hashfs PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB} ${LIBTSK_LDFLAGS})
set_target_properties(hashfs PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: hive_info - Displays Windows Registry hive info
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(hive_info hive_info.cpp)
target_include_directories(hive_info PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(hive_info PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB} ${LIBTSK_LDFLAGS})
set_target_properties(hive_info PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: hive_scan - Scans Windows Registry hives
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(hive_scan hive_scan.cpp)
target_include_directories(hive_scan PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(hive_scan PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB} ${LIBTSK_LDFLAGS})
set_target_properties(hive_scan PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: imagefile_cat - Print image file to the standard output
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(imagefile_cat imagefile_cat.cpp)
target_include_directories(imagefile_cat PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(imagefile_cat PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB})
set_target_properties(imagefile_cat PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: imagefile_convert - Converts image file formats
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(imagefile_convert imagefile_convert.cpp)
target_include_directories(imagefile_convert PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(imagefile_convert PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB})
set_target_properties(imagefile_convert PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: imagefile_info - Displays image file info
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(imagefile_info imagefile_info.cpp)
target_include_directories(imagefile_info PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(imagefile_info PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB})
set_target_properties(imagefile_info PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: list_resources - Lists resources
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(list_resources list_resources.cpp)
target_include_directories(list_resources PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(list_resources PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB})
set_target_properties(list_resources PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: vfs_info - Displays virtual filesystem info
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(vfs_info vfs_info.cpp)
target_include_directories(vfs_info PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(vfs_info PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB} ${LIBTSK_LDFLAGS})
set_target_properties(vfs_info PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: device_list - Lists devices using libudev 
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(device_list device_list.cpp)
target_include_directories(device_list PRIVATE ${CMAKE_SOURCE_DIR} ${LIBUDEV_INCLUDE_DIRS})
target_compile_options(device_list PRIVATE ${LIBUDEV_CFLAGS})
target_link_libraries(device_list PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB} ${LIBUDEV_LIBRARIES})
set_target_properties(device_list PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: disk_list - Lists disks using libudev
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(disk_list disk_list.cpp)
target_include_directories(disk_list PRIVATE ${CMAKE_SOURCE_DIR} ${LIBUDEV_INCLUDE_DIRS})
target_compile_options(disk_list PRIVATE ${LIBUDEV_CFLAGS})
target_link_libraries(disk_list PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB} ${LIBUDEV_LIBRARIES})
set_target_properties(disk_list PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
set(TOOLS_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/tools")

install(TARGETS
    casetree
    device_list
    dirfs
    dirtree
    disk_list
    filesystem_scan
    hashfs
    hive_info
    hive_scan
    imagefile_cat
    imagefile_convert
    imagefile_info
    list_resources
    vfs_info
    DESTINATION ${TOOLS_INSTALL_DIR}
)
