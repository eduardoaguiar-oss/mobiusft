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
project(libmobius-core-os LANGUAGES CXX)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Target
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_library(mobius_core_os STATIC
    win/credential.cpp
    win/hash_ie_entropy.cpp
    win/hash_lm.cpp
    win/hash_msdcc1.cpp
    win/hash_msdcc2.cpp
    win/hash_nt.cpp
    win/pbkdf2_hmac_ms.cpp
    win/dpapi/blob.cpp
    win/dpapi/cipher_info.cpp
    win/dpapi/credhist_entry.cpp
    win/dpapi/credhist_file.cpp
    win/dpapi/hash_info.cpp
    win/dpapi/master_key.cpp
    win/dpapi/master_key_file.cpp
    win/registry/hive_data.cpp
    win/registry/hive_decoder.cpp
    win/registry/hive_file.cpp
    win/registry/hive_key.cpp
    win/registry/hive_value.cpp
    win/registry/pssp_data2.cpp
    win/registry/registry.cpp
    win/registry/registry_data.cpp
    win/registry/registry_data_impl_hdata.cpp
    win/registry/registry_data_impl_lsa_poleklist.cpp
    win/registry/registry_data_impl_lsa_polseckey.cpp
    win/registry/registry_data_impl_msdcc.cpp
    win/registry/registry_data_impl_pssp.cpp
    win/registry/registry_file.cpp
    win/registry/registry_key.cpp
    win/registry/registry_key_impl_container.cpp
    win/registry/registry_key_impl_hkey.cpp
    win/registry/registry_key_impl_link.cpp
    win/registry/registry_key_impl_lsa.cpp
    win/registry/registry_key_impl_msdcc.cpp
    win/registry/registry_key_impl_pssp.cpp
    win/registry/registry_key_impl_user_assist.cpp
    win/registry/registry_key_list.cpp
    win/registry/registry_set_lsa_keys.cpp
    win/registry/registry_set_msdcc_key.cpp
    win/registry/registry_set_pssp_keys.cpp
    win/registry/registry_set_user_assist_keys.cpp
    win/registry/registry_value.cpp
    win/registry/registry_value_impl_container.cpp
    win/registry/registry_value_impl_hvalue.cpp
    win/trashbin/ifile.cpp
    win/trashbin/info2_entry.cpp
    win/trashbin/info2_file.cpp
)

target_include_directories(mobius_core_os PRIVATE
    ${MOBIUS_INCLUDE_DIRS}
)

target_compile_options(mobius_core_os PRIVATE -fno-strict-aliasing -fPIC)
