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
project(libmobius-python-core LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Build sub-libraries
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_subdirectory(database)
add_subdirectory(file_decoder)
add_subdirectory(kff)
add_subdirectory(system)
add_subdirectory(turing)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(libmobius_python_core STATIC
    module.cpp
    application.cpp
    extension.cpp
    func_logf.cpp
    func_mediator.cpp
    func_resource.cpp
    func_set_logfile_path.cpp
    log.cpp
    resource.cpp
    thread_guard.cpp
)

set_target_properties(libmobius_python_core PROPERTIES
    PREFIX ""
)

target_include_directories(libmobius_python_core PRIVATE
    ${MOBIUS_PYTHON_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Python3_INCLUDE_DIRS}
)

target_link_libraries(libmobius_python_core
   PRIVATE
       "$<LINK_LIBRARY:WHOLE_ARCHIVE,libmobius_python_core_database>"
       "$<LINK_LIBRARY:WHOLE_ARCHIVE,libmobius_python_core_file_decoder>"
       "$<LINK_LIBRARY:WHOLE_ARCHIVE,libmobius_python_core_kff>"
       "$<LINK_LIBRARY:WHOLE_ARCHIVE,libmobius_python_core_system>"
       "$<LINK_LIBRARY:WHOLE_ARCHIVE,libmobius_python_core_turing>"
   PUBLIC
       Mobius::Core
)

target_compile_options(libmobius_python_core PRIVATE -fno-strict-aliasing -fPIC)
