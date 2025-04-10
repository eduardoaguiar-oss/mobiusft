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
project(vfs_imagefile_ewf LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
set(SOURCES
    main.cpp
    imagefile_impl.cpp
    reader_impl.cpp
    segment_decoder.cpp
    segment_writer.cpp
    writer_impl.cpp
)

add_library(vfs_imagefile_ewf MODULE ${SOURCES})
set_target_properties(vfs_imagefile_ewf PROPERTIES OUTPUT_NAME "vfs-imagefile-ewf" PREFIX "")
target_include_directories(vfs_imagefile_ewf PRIVATE ${MOBIUS_INCLUDE_DIRS})
target_link_libraries(vfs_imagefile_ewf PRIVATE ${MOBIUS_LIBRARIES})

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools: ewf_decoder
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
set(EWF_DECODER_SOURCES
    tools/ewf_decoder.cpp
    imagefile_impl.cpp
    reader_impl.cpp
    segment_decoder.cpp
    segment_writer.cpp
    writer_impl.cpp
)

add_executable(ewf_decoder ${EWF_DECODER_SOURCES})
target_link_libraries(ewf_decoder PRIVATE ${MOBIUS_LIBRARIES})
target_include_directories(ewf_decoder PRIVATE ${MOBIUS_INCLUDE_DIRS})

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS vfs_imagefile_ewf LIBRARY DESTINATION share/mobiusft/extensions)

install(TARGETS
    ewf_decoder
    RUNTIME DESTINATION share/mobiusft/tools)
