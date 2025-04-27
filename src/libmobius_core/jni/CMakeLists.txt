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
cmake_minimum_required(VERSION 3.15)
project(libmobius-core-jni LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Dependency: JNI
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
set(JAVA_HOME "$ENV{JAVA_HOME}" CACHE PATH "Path to JDK")

# Fallback to `find_package(Java)` if JAVA_HOME is unset
if(NOT JAVA_HOME)
    find_package(Java COMPONENTS Development REQUIRED)
    set(JAVA_HOME "${Java_Development_ROOT_DIR}" CACHE PATH "Path to JDK" FORCE)
endif()

# Hint CMake to search in JAVA_HOME first
if(JAVA_HOME)
    list(PREPEND CMAKE_PREFIX_PATH "${JAVA_HOME}")
    list(PREPEND CMAKE_LIBRARY_PATH "${JAVA_HOME}/lib/server")  # Linux/macOS
    # Windows: "${JAVA_HOME}/lib"
endif()

find_package(JNI REQUIRED)

if(JNI_FOUND)
    message(STATUS "JNI found: ${JNI_LIBRARIES}")
else()
    message(FATAL_ERROR "JNI not found! Tried JAVA_HOME=${JAVA_HOME}")
endif()

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(mobius_core_jni STATIC
    local_ref.cpp
    string.cpp
)

target_include_directories(mobius_core_jni PRIVATE
    ${MOBIUS_INCLUDE_DIRS}
    ${JNI_INCLUDE_DIRS}
)

target_compile_options(mobius_core_jni PRIVATE -fno-strict-aliasing -fPIC)
