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
project(libmobius_python_core_os LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(libmobius_python_core_os STATIC
    module.cpp
    win/module.cpp
    win/credential.cpp
    win/dpapi/module.cpp
    win/dpapi/blob.cpp
    win/dpapi/credhist_entry.cpp
    win/dpapi/credhist_file.cpp
    win/dpapi/master_key.cpp
    win/dpapi/master_key_file.cpp
    win/func_hash_ie_entropy.cpp
    win/func_hash_lm.cpp
    win/func_hash_msdcc1.cpp
    win/func_hash_msdcc2.cpp
    win/func_hash_nt.cpp
    win/func_pbkdf2_hmac_ms.cpp
    win/registry/module.cpp
    win/registry/registry.cpp
    win/registry/registry_data.cpp
    win/registry/registry_file.cpp
    win/registry/registry_key.cpp
    win/registry/registry_value.cpp
    win/trashbin/module.cpp
    win/trashbin/ifile.cpp
    win/trashbin/info2_entry.cpp
    win/trashbin/info2_file.cpp
)

set_target_properties(libmobius_python_core_os PROPERTIES
    PREFIX ""
)

target_include_directories(libmobius_python_core_os PRIVATE
    ${MOBIUS_PYTHON_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Python3_INCLUDE_DIRS}
)

target_link_libraries(libmobius_python_core_os PUBLIC Mobius::Core)

target_compile_options(libmobius_python_core_os PRIVATE -fno-strict-aliasing -fPIC)
