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
CMAKE_BUILD_DIR = build

all-local:
	@mkdir -p $(CMAKE_BUILD_DIR)
	@cd $(CMAKE_BUILD_DIR) && \
		$(CMAKE) -DCMAKE_INSTALL_PREFIX=$(prefix) -DCMAKE_BUILD_TYPE=Release .. && \
		$(MAKE)

install-exec-local:
	@cd $(CMAKE_BUILD_DIR) && $(MAKE) install DESTDIR=$(DESTDIR)

clean-local:
	@rm -rf $(CMAKE_BUILD_DIR)

EXTRA_DIST = 								\
    CMakeLists.txt							\
    application.cpp.in							\
    *.cpp								\
    datasource/CMakeLists.txt						\
    datasource/*.cpp							\
    datasource/ufdr/*.cpp						\
    encoder/CMakeLists.txt						\
    encoder/*.cpp							\
    file_decoder/CMakeLists.txt						\
    file_decoder/*.cpp							\
    jni/CMakeLists.txt							\
    jni/*.cpp								\
    kff/CMakeLists.txt							\
    kff/*.cpp								\
    turing/CMakeLists.txt						\
    turing/*.cpp							\
    ui/CMakeLists.txt							\
    ui/*.cpp
