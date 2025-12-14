# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008-2026 Eduardo Aguiar
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
    file_searches_dat.cpp
    profile.cpp
    vfs_processor_impl.cpp
)

set_target_properties(app_shareaza PROPERTIES OUTPUT_NAME "app-shareaza" PREFIX "")
target_link_libraries(app_shareaza PRIVATE Mobius::Framework Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: shareaza_library_dat
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_shareaza_library_dat
    tools/shareaza_library_dat.cpp
    CLibrary.cpp
    CLibraryFolder.cpp
    CLibraryFile.cpp
    CSharedSource.cpp
    CXMLElement.cpp
)

set_target_properties(tools_shareaza_library_dat PROPERTIES OUTPUT_NAME "shareaza_library_dat")
target_link_libraries(tools_shareaza_library_dat PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: shareaza_sd
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_shareaza_sd
    tools/shareaza_sd.cpp
    CBTInfo.cpp
    CDownload.cpp
    CDownloadSource.cpp
    CShareaza.cpp
    CXMLElement.cpp
)

set_target_properties(tools_shareaza_sd PROPERTIES OUTPUT_NAME "shareaza_sd")
target_link_libraries(tools_shareaza_sd PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: shareaza_searches_dat
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(tools_shareaza_searches_dat
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

set_target_properties(tools_shareaza_searches_dat PROPERTIES OUTPUT_NAME "shareaza_searches_dat")
target_link_libraries(tools_shareaza_searches_dat PRIVATE Mobius::Core)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS app_shareaza LIBRARY DESTINATION share/mobiusft/extensions)

install(TARGETS
	tools_shareaza_library_dat
	tools_shareaza_sd
	tools_shareaza_searches_dat
        RUNTIME DESTINATION share/mobiusft/tools
)
