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
project(libmobius_python_core_decoder LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(libmobius_python_core_decoder STATIC
    module.cpp
    data_decoder.cpp
    func_base16.cpp
    func_base32.cpp
    func_base32hex.cpp
    func_base64.cpp
    func_base64url.cpp
    func_btencode.cpp
    func_get_filetype.cpp
    inifile.cpp
    lnk.cpp
    mfc.cpp
    qdatastream.cpp
    sourcecode.cpp
    tdf.cpp
    sgml/module.cpp
    sgml/parser.cpp
    sgml/tokenizer.cpp
    xml/module.cpp
    xml/dom.cpp
    xml/element.cpp
)

set_target_properties(libmobius_python_core_decoder PROPERTIES
    PREFIX ""
)

target_include_directories(libmobius_python_core_decoder PRIVATE
    ${MOBIUS_PYTHON_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Python3_INCLUDE_DIRS}
)

target_link_libraries(libmobius_python_core_decoder PUBLIC Mobius::Core)

target_compile_options(libmobius_python_core_decoder PRIVATE -fno-strict-aliasing -fPIC)
