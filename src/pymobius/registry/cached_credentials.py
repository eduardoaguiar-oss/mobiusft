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
import struct

from pymobius.registry import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief domain cached credentials decoder
# @author Eduardo Aguiar
# @see https://sec.ch9.ms/sessions/ignite/2016/BRK3292.pptx
# @see https://github.com/gentilkiwi/mimikatz
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CachedCredential(object):
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief data structure
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    structure = [
        [0, 'username_len', 'uint16_le', 'username length'],
        [2, 'domain_name_len', 'uint16_le', 'domain length'],
        [4, 'name_len', 'uint16_le', 'name length'],
        [6, 'fullname_len', 'uint16_le', 'fullname length'],
        [8, 'logon_script_len', 'uint16_le', 'logon script length'],
        [10, 'profile_path_len', 'uint16_le', 'profile path length'],
        [12, 'home_dir_len', 'uint16_le', 'home dir length'],
        [14, 'home_dir_letter_len', 'uint16_le', 'home dir logical drive'],
        [16, 'rid', 'uint32_le', 'user RID'],
        [20, 'gid', 'uint32_le', 'user GID'],
        [24, 'count', 'uint32_le', 'group count'],
        [28, 'logon_domain_name_len', 'uint16_le', 'logon domain name length'],
        [30, 'domain_sid_len', 'uint16_le', 'domain SID length'],
        [32, 'timestamp', 'uint64_le', 'last local logon'],
        [40, 'revision', 'uint32_le', 'revision'],
        [44, 'sid_count', 'uint32_le', 'SID count'],
        [48, 'encryption_flags', 'uint16_le', 'encryption flag (1 = encrypted)'],
        [50, 'encryption_algorithm', 'uint16_le', '(0 - MSDCC1, 10 - MSDCC2)'],
        [52, 'dummy1', 'uint32_le', '???'],
        [56, 'logon_package', 'uint32_le', '???'],
        [60, 'dns_domain_name_len', 'uint16_le', 'DNS domain name length'],
        [62, 'email_len', 'uint16_le', 'e-mail length'],
        [64, 'iv', 'byte (16)', 'cryptographic Initialization Vector (IV)'],
        [80, 'chksum', 'byte (16)', 'MD5 digest'],
        [96, 'mscachehash', 'byte (16)', 'MS cache hash'],
        [112, 'anotherhash', 'byte (16)', '???'],
        [128, 'dummy2', 'uint32_le', '???'],
        [132, 'dummy3', 'uint32_le', '???'],
        [136, 'cache_len', 'uint32_le', 'total cache data length'],
        [140, 'dummy4', 'uint32_le', '???'],
        [144, 'computer_len', 'uint32_le', 'computer name length'],
        [168, 'username', 'varlen', 'user name'],
    ]

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # @param data cached credential data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, data):

        # check if data can be parsed
        if len(data) < 148:
            self.__is_valid = False
            return

        # parse cache structure
        username_len, \
            domain_name_len, \
            name_len, \
            fullname_len, \
            logon_script_len, \
            profile_path_len, \
            home_dir_len, \
            home_dir_letter_len, \
            self.rid, \
            self.gid, \
            count, \
            logon_domain_name_len, \
            domain_sid_len, \
            timestamp, \
            self.revision, \
            sid_count, \
            self.encryption_flags, \
            self.encryption_algorithm, \
            dummy1, \
            logon_package, \
            dns_domain_name_len, \
            email_len, \
            self.iv, \
            self.chksum, \
            self.mscachehash, \
            anotherhash, \
            dummy2, \
            dummy3, \
            cache_len, \
            dummy4, \
            computer_len = struct.unpack("<HHHHHHHHIIIHHQIIHHIIHH16s16s16s16sIIIII", data[:148])

        # if cache credential is empty, return
        if username_len == 0:
            self.__is_valid = False
            return

        # parse data
        offset = 168
        self.last_logon_date = get_nt_datetime(timestamp)

        self.username = data[offset:offset + username_len].decode('utf-16-le')
        offset = self.__get_next_offset(offset, username_len)

        self.domain = data[offset:offset + domain_name_len].decode('utf-16-le')
        offset = self.__get_next_offset(offset, domain_name_len)

        self.dns_domain = data[offset:offset + dns_domain_name_len].decode('utf-16-le')
        offset = self.__get_next_offset(offset, dns_domain_name_len)

        self.email = data[offset:offset + email_len].decode('utf-16-le')
        offset = self.__get_next_offset(offset, email_len)

        self.name = data[offset:offset + name_len].decode('utf-16-le')
        offset = self.__get_next_offset(offset, name_len)

        self.fullname = data[offset:offset + fullname_len].decode('utf-16-le')
        offset = self.__get_next_offset(offset, fullname_len)

        self.logon_script = data[offset:offset + logon_script_len].decode('utf-16-le')
        offset = self.__get_next_offset(offset, logon_script_len)

        self.profile_path = data[offset:offset + profile_path_len].decode('utf-16-le')
        offset = self.__get_next_offset(offset, profile_path_len)

        self.home_dir = data[offset:offset + home_dir_len].decode('utf-16-le')
        offset = self.__get_next_offset(offset, home_dir_len)

        self.home_dir_letter = data[offset:offset + home_dir_letter_len].decode('utf-16-le')
        offset = self.__get_next_offset(offset, home_dir_letter_len) + 8 * count

        self.logon_domain = data[offset:offset + logon_domain_name_len].decode('utf-16-le')
        offset = self.__get_next_offset(offset, logon_domain_name_len)

        if dummy1 > 0:
            unknown1 = data[offset:offset + dummy1]
            offset = self.__get_next_offset(offset, dummy1)

        self.domain_sid = 'S-' + '-'.join(
            [str(i) for i in struct.unpack('<B6xBIIII', data[offset:offset + domain_sid_len])])
        offset = self.__get_next_offset(offset, domain_sid_len)

        self.computer = data[offset:offset + computer_len].decode('utf-16-le')

        self.__is_valid = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Check if object is valid
    # @return true/false
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def is_valid(self):
        return self.__is_valid

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get next offset, 32 bit aligned
    # @param offset current offset
    # @param fieldlen field length in bytes
    # @return next offset
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_next_offset(self, offset, fieldlen):
        return offset + fieldlen + (fieldlen & 0x02)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get cached credentials data
# @param registry registry object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get(registry):
    credentials = []

    for value in registry.get_value_by_mask('HKLM\\SECURITY\\Cache\\NL$[0-9]*'):
        cache = CachedCredential(value.data.data)

        if cache.is_valid():
            credentials.append((value.name, cache))

    return credentials
