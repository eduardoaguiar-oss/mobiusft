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
project(mobius_python_io LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target: mobius_python_io.la
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(mobius_python_io STATIC
    module.cpp
    entry.cpp
    file.cpp
    folder.cpp
    func_get_current_folder.cpp
    func_join_path.cpp
    func_new_entry_by_path.cpp
    func_new_entry_by_url.cpp
    func_new_file_by_path.cpp
    func_new_file_by_url.cpp
    func_new_folder_by_path.cpp
    func_new_folder_by_url.cpp
    func_new_path_from_win.cpp
    func_new_uri_from_path.cpp
    func_reader.cpp
    func_to_win_path.cpp
    line_reader.cpp
    path.cpp
    reader.cpp
    sequential_reader_adaptor.cpp
    stream.cpp
    tempfile.cpp
    text_reader.cpp
    text_writer.cpp
    uri.cpp
    walker.cpp
    writer.cpp
)

target_include_directories(mobius_python_io PRIVATE
    ${MOBIUS_PYTHON_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Python3_INCLUDE_DIRS}
)

target_link_libraries(mobius_python_io PUBLIC Mobius::Core)

target_compile_options(mobius_python_io PRIVATE -fno-strict-aliasing -fPIC)
