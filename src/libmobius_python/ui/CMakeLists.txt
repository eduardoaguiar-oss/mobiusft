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
project(mobius_python_ui LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Find dependencies
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 IMPORTED_TARGET REQUIRED gtk+-3.0)
pkg_check_modules(PYGOBJECT IMPORTED_TARGET REQUIRED pygobject-3.0)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target: mobius_python_ui.la
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(mobius_python_ui STATIC
    module.cpp
    box.cpp
    button.cpp
    container.cpp
    func_ui.cpp
    icon.cpp
    label.cpp
    message_dialog.cpp
    stacked_container.cpp
    widget.cpp
    window.cpp
)

target_include_directories(mobius_python_ui PRIVATE
    ${MOBIUS_PYTHON_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
    ${GTK3_INCLUDE_DIRS}
    ${PYGOBJECT_INCLUDE_DIRS}
    ${Python3_INCLUDE_DIRS}
)

target_link_libraries(mobius_python_ui PUBLIC
    Mobius::Core
    PkgConfig::GTK3
    PkgConfig::PYGOBJECT
)

target_compile_options(mobius_python_ui PRIVATE -fno-strict-aliasing -fPIC)
