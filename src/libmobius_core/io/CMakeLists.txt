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
project(libmobius-core-io LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Find libsmbclient
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
option(WITH_SAMBA "Enable Samba support" ON)

if(WITH_SAMBA)
  set(SAMBA_ROOT "" CACHE PATH "Custom Samba installation path")
  if(SAMBA_ROOT)
    list(APPEND CMAKE_PREFIX_PATH "${SAMBA_ROOT}")
  endif()

  find_package(PkgConfig)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(SMBCLIENT smbclient)
  endif()

  if(NOT SMBCLIENT_FOUND)
    find_library(SMBCLIENT_LIBRARIES smbclient)
    find_path(SMBCLIENT_INCLUDE_DIRS libsmbclient.h PATH_SUFFIXES samba-4.0)
  endif()

  if(SMBCLIENT_LIBRARIES AND SMBCLIENT_INCLUDE_DIRS)
    message(STATUS "Found libsmbclient:")
    message(STATUS "  Include dirs: ${SMBCLIENT_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${SMBCLIENT_LIBRARIES}")
  else()
    message(FATAL_ERROR "libsmbclient not found! Disable with -DWITH_SAMBA=OFF")
  endif()
endif()

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(mobius_core_io STATIC
    bytearray_io.cpp
    entry.cpp
    file.cpp
    folder.cpp
    line_reader.cpp
    path.cpp
    reader.cpp
    reader_impl_base.cpp
    reader_impl_bytearray.cpp
    reader_impl_slice.cpp
    sector_reader_adaptor.cpp
    sequential_reader_adaptor.cpp
    stream.cpp
    tempfile.cpp
    text_reader.cpp
    text_writer.cpp
    uri.cpp
    walker.cpp
    writer.cpp
    writer_impl_base.cpp
    writer_impl_bytearray.cpp
    local/file_impl.cpp
    local/folder_impl.cpp
    local/get_current_folder.cpp
    local/new_entry_by_path.cpp
    local/reader_impl.cpp
    local/writer_impl.cpp
    smb/file_impl.cpp
    smb/folder_impl.cpp
    smb/init.cpp
    smb/new_entry_by_url.cpp
    smb/reader_impl.cpp
    smb/writer_impl.cpp
)

target_include_directories(mobius_core_io PRIVATE
    ${MOBIUS_INCLUDE_DIRS}
    ${SMBCLIENT_INCLUDE_DIRS}
)

target_link_libraries(mobius_core_io PUBLIC
    ${SMBCLIENT_LIBRARIES}
)

target_compile_options(mobius_core_io PRIVATE -fno-strict-aliasing -fPIC)
