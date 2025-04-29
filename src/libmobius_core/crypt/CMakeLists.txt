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
project(libmobius-core-crypt LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Find libgcrypt
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGCRYPT REQUIRED libgcrypt>=1.9.0)

if(LIBGCRYPT_FOUND)
  message(STATUS "Found libgcrypt (>= 1.9.0):")
  message(STATUS "  Version: ${LIBGCRYPT_VERSION}")
  message(STATUS "  Includes: ${LIBGCRYPT_INCLUDE_DIRS}")
  message(STATUS "  Libraries: ${LIBGCRYPT_LIBRARIES}")
else()
    find_library(LIBGCRYPT_LIBRARIES gcrypt)
    find_path(LIBGCRYPT_INCLUDE_DIRS gcrypt.h)

    if(LIBGCRYPT_LIBRARIES AND LIBGCRYPT_INCLUDE_DIRS)
      # Extract version from gcrypt.h
      file(READ "${LIBGCRYPT_INCLUDE_DIRS}/gcrypt.h" GCRYPT_H_CONTENTS)
      string(REGEX MATCH "GCRYPT_VERSION \"([0-9.]+)\"" _ ${GCRYPT_H_CONTENTS})
      set(LIBGCRYPT_VERSION ${CMAKE_MATCH_1})

      # Version check (>= 1.9.0)
      if(LIBGCRYPT_VERSION VERSION_LESS "1.9.0")
        message(FATAL_ERROR "libgcrypt version ${LIBGCRYPT_VERSION} found, but >= 2.9.0 required")
      else()
        message(STATUS "Found libgcrypt (>= 1.9.0): ${LIBGCRYPT_VERSION}")
        add_library(LibGcrypt::LibGcrypt INTERFACE IMPORTED)
      endif()
    else()
      message(FATAL_ERROR "Libgcrypt 1.9.0 or higher is required.")
    endif()
endif()

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(mobius_core_crypt STATIC
    cipher.cpp
    cipher_impl_block.cpp
    cipher_impl_ige.cpp
    cipher_impl_stream.cpp
    cipher_impl_zip.cpp
    crc32.cpp
    hash.cpp
    hash_digest.cpp
    hash_impl_adler32.cpp
    hash_impl_ed2k.cpp
    hash_impl_zip.cpp
    hmac.cpp
    hmac_impl_default.cpp
    pkcs5.cpp
    rot13.cpp
    gcrypt/cipher_impl.cpp
    gcrypt/hash_impl.cpp
    gcrypt/hmac_impl.cpp
    gcrypt/util.cpp
)

target_include_directories(mobius_core_crypt PRIVATE
    ${MOBIUS_INCLUDE_DIRS}
    ${LIBGCRYPT_INCLUDE_DIRS}
)

target_link_libraries(mobius_core_crypt PUBLIC ${LIBGCRYPT_LIBRARIES})

target_compile_options(mobius_core_crypt PRIVATE -fno-strict-aliasing -fPIC)
