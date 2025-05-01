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
project(app_shareaza LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(app_shareaza MODULE
    main.cpp
    CBTInfo.cpp
    CDownload.cpp
    CDownloadSource.cpp
    CLibrary.cpp
    CLibraryFile.cpp
    CLibraryFolder.cpp
    CManagedSearch.cpp
    CMatchFile.cpp
    CMatchList.cpp
    CQueryHit.cpp
    CQuerySearch.cpp
    CShareaza.cpp
    CSharedSource.cpp
    CThumbCache.cpp
    CWndSearch.cpp
    CXMLElement.cpp
    evidence_loader_impl.cpp
    file_searches_dat.cpp
)

set_target_properties(app_shareaza PROPERTIES OUTPUT_NAME "app-shareaza" PREFIX "")
target_link_libraries(app_shareaza PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: shareaza_library_dat
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(shareaza_library_dat
    tools/shareaza_library_dat.cpp
    CLibrary.cpp
    CLibraryFolder.cpp
    CLibraryFile.cpp
    CSharedSource.cpp
    CXMLElement.cpp
)

target_link_libraries(shareaza_library_dat PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: shareaza_sd
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(shareaza_sd
    tools/shareaza_sd.cpp
    CBTInfo.cpp
    CDownload.cpp
    CDownloadSource.cpp
    CShareaza.cpp
    CXMLElement.cpp
)

target_link_libraries(shareaza_sd PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: shareaza_searches_dat
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(shareaza_searches_dat
        tools/shareaza_searches_dat.cpp
        CManagedSearch.cpp
        CMatchFile.cpp
        CMatchList.cpp
        CQueryHit.cpp
        CQuerySearch.cpp
        CShareaza.cpp
        CWndSearch.cpp
        CXMLElement.cpp
)

target_link_libraries(shareaza_searches_dat PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS app_shareaza LIBRARY DESTINATION share/mobiusft/extensions)

install(TARGETS
	shareaza_library_dat
	shareaza_sd
	shareaza_searches_dat
        RUNTIME DESTINATION share/mobiusft/tools
)
