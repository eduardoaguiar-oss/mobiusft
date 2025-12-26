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
import struct

import mobius
import mobius.core.crypt
import pymobius
from pymobius.registry import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief SAM's F structure decoder
# @author Eduardo Aguiar
# @see http://www.beginningtoseethelight.org/ntsecurity/index.htm
# @see https://github.com/keydet89/RegRipper2.8
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SAM_F(object):
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief data structure
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    structure = [
        [0, 'revision', 'uint32_le', 'data structure version'],
        [4, 'unknown1', 'uint32_le', '??? (either 65538 or 65539)'],
        [8, 'last_logon_timestamp', 'uint64_le', 'last logon NT datetime'],
        [16, 'unknown2', 'uint64_le', '??? (always 0)'],
        [24, 'last_password_set_timestamp', 'uint64_le', 'last password set NT datetime'],
        [32, 'password_expiration_timestamp', 'uint64_le', '??? (always either 0 or 0x7fffffffffffffff'],
        [40, 'last_failed_logon_timestamp', 'uint64_le', 'last failed logon NT datetime'],
        [48, 'rid', 'uint32_le', 'user RID'],
        [52, 'gid', 'uint32_le', 'user primary group ID'],
        [56, 'flags', 'uint32_le', 'ACB flags'],
        [60, 'country_code', 'uint16_le', 'country code (0 = system default)'],
        [62, 'unknown3', 'uint16_le', '??? (always 0)'],
        [64, 'failed_logon_count', 'uint16_le', 'failed logon count'],
        [66, 'logon_count', 'uint16_le', 'successful logon count'],
        [68, 'unknown4', 'uint32_le', '??? (either 0 or 1)'],
    ]

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # @param data F value data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, data):

        # check if data can be parsed
        if len(data) < 72:
            self.__is_valid = False
            return

        # parse data structure
        self.revision, \
            dummy1, \
            last_logon_timestamp, \
            last_logoff_timestamp, \
            last_password_set_timestamp, \
            password_expiration_timestamp, \
            last_failed_logon_timestamp, \
            self.rid, \
            self.gid, \
            self.flags, \
            self.country_code, \
            self.codepage, \
            self.failed_logon_count, \
            self.logon_count, \
            self.dummy4 = struct.unpack('<IIQQQQQIIIHHHHI', data[:72])

        if password_expiration_timestamp == 0x7fffffffffffffff:
            password_expiration_timestamp = 0

        # format data
        self.last_logon_datetime = get_nt_datetime(last_logon_timestamp)
        self.last_logoff_datetime = get_nt_datetime(last_logoff_timestamp)
        self.last_password_set_datetime = get_nt_datetime(last_password_set_timestamp)
        self.last_failed_logon_datetime = get_nt_datetime(last_failed_logon_timestamp)
        self.password_expiration_datetime = get_nt_datetime(password_expiration_timestamp)

        # @see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-samr/b10cfda1-f24f-441b-8f43-80cb93e786ec
        self.flag_account_enabled = not (self.flags & 0x0001)
        self.flag_home_dir_required = bool(self.flags & 0x0002)
        self.flag_password_required = not (self.flags & 0x0004)
        self.flag_temporary_duplicated_account = bool(self.flags & 0x0008)
        self.flag_normal_user_account = bool(self.flags & 0x0010)
        self.flag_mns_logon_user_account = bool(self.flags & 0x0020)
        self.flag_interdomain_trust_account = bool(self.flags & 0x0040)
        self.flag_workstation_trust_account = bool(self.flags & 0x0080)
        self.flag_server_trust_account = bool(self.flags & 0x0100)
        self.flag_password_expires = not (self.flags & 0x0200)
        self.flag_account_auto_locked = bool(self.flags & 0x0400)

        self.__is_valid = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Check if object is valid
    # @return true/false
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def is_valid(self):
        return self.__is_valid


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief SAM's V structure decoder
# @author Eduardo Aguiar
# @see http://www.beginningtoseethelight.org/ntsecurity/index.htm
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SAM_V(object):
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief data structure
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    structure = [
        [0, 'dummy1', 'uint32_le', '???'],
        [4, 'var_field_offset', 'uint32_le', 'var fields offset'],
        [8, 'revision', 'uint32_le', 'data structure version'],
        [12, 'name_offset', 'uint32_le', 'user name field offset'],
        [16, 'name_size', 'uint32_le', 'user name field size in bytes'],
        [20, 'dummy4', 'uint32_le', '???'],
        [24, 'fullname_offset', 'uint32_le', 'full user name field offset'],
        [28, 'fullname_size', 'uint32_le', 'full user name field size in bytes'],
        [32, 'dummy5', 'uint32_le', '???'],
        [36, 'comment_offset', 'uint32_le', 'comment field offset'],
        [40, 'comment_size', 'uint32_le', 'comment field size in bytes'],
        [44, 'dummy6', 'uint32_le', '???'],
        [48, 'user_comment_offset', 'uint32_le', 'user comment field offset'],
        [52, 'user_comment_size', 'uint32_le', 'user comment field size in bytes'],
        [56, 'dummy7', 'uint32_le', '???'],
        [60, 'ts_user_parm_offset', 'uint32_le', 'terminal server user parms field offset'],
        [64, 'ts_user_parm_size', 'uint32_le', 'terminal server user parms field size in bytes'],
        [68, 'dummy9', 'uint32_le', '???'],
        [72, 'home_dir_offset', 'uint32_le', 'home directory field offset'],
        [76, 'home_dir_size', 'uint32_le', 'home directory field size in bytes'],
        [80, 'dummy10', 'uint32_le', '???'],
        [84, 'home_dir_letter_offset', 'uint32_le', 'home directory letter field offset'],
        [88, 'home_dir_letter_size', 'uint32_le', 'home directory letter field size in bytes'],
        [92, 'dummy11', 'uint32_le', '???'],
        [96, 'logon_script_offset', 'uint32_le', 'logon script field offset'],
        [100, 'logon_script_size', 'uint32_le', 'logon script field size in bytes'],
        [104, 'dummy12', 'uint32_le', '???'],
        [108, 'profile_path_offset', 'uint32_le', 'profile path field offset'],
        [112, 'profile_path_size', 'uint32_le', 'profile path field size in bytes'],
        [116, 'dummy13', 'uint32_le', '???'],
        [120, 'workstation_offset', 'uint32_le', 'workstation field offset'],
        [124, 'workstation_size', 'uint32_le', 'workstation field size in bytes'],
        [128, 'dummy14', 'uint32_le', '???'],
        [132, 'hours_allowed_offset', 'uint32_le', 'Hours allowed offset'],
        [136, 'hours_allowed_size', 'uint32_le', 'Hours allowed size'],
        [140, 'dummy16', 'uint32_le', '???'],
        [144, 'dummy17_offset', 'uint32_le', '??? dummy17 field offset'],
        [148, 'dummy17_size', 'uint32_le', 'dummy17 field size in bytes'],
        [152, 'dummy18', 'uint32_le', '???'],
        [156, 'lm_hash_offset', 'uint32_le', 'LM hash field offset'],
        [160, 'lm_hash_size', 'uint32_le', 'LM hash field size in bytes'],
        [164, 'dummy19', 'uint32_le', '???'],
        [168, 'nt_hash_offset', 'uint32_le', 'NTLM hash field offset'],
        [172, 'nt_hash_size', 'uint32_le', 'NTLM hash field size in bytes'],
        [176, 'dummy20', 'uint32_le', '???'],
        [180, 'nt_hash_history_offset', 'uint32_le', 'NTLM hash history field offset'],
        [184, 'nt_hash_history_size', 'uint32_le', 'NTLM hash history field size in bytes'],
        [188, 'dummy22', 'uint32_le', '???'],
        [192, 'lm_hash_history_offset', 'uint32_le', 'LM hash history field offset'],
        [196, 'lm_hash_history_size', 'uint32_le', 'LM hash history field size in bytes'],
        [200, 'dummy24', 'uint32_le', '???'],
    ]

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # @param data V value data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, data):
        # check if data can be parsed
        if len(data) < 72:
            self.__is_valid = False
            return

        # parse data structure
        dummy1, \
            var_field_offset, \
            self.revision, \
            name_offset, \
            name_size, \
            dummy4, \
            fullname_offset, \
            fullname_size, \
            dummy5, \
            admin_comment_offset, \
            admin_comment_size, \
            dummy6, \
            user_comment_offset, \
            user_comment_size, \
            dummy7, \
            ts_user_parm_offset, \
            ts_user_parm_size, \
            dummy9, \
            home_dir_offset, \
            home_dir_size, \
            dummy10, \
            home_dir_letter_offset, \
            home_dir_letter_size, \
            dummy11, \
            logon_script_offset, \
            logon_script_size, \
            dummy12, \
            profile_path_offset, \
            profile_path_size, \
            dummy13, \
            workstation_offset, \
            workstation_size, \
            dummy14, \
            hours_allowed_offset, \
            hours_allowed_size, \
            self.dummy16, \
            dummy17_offset, \
            dummy17_size, \
            self.dummy18, \
            lm_hash_offset, \
            lm_hash_size, \
            dummy19, \
            nt_hash_offset, \
            nt_hash_size, \
            dummy20, \
            nt_hash_history_offset, \
            nt_hash_history_size, \
            dummy22, \
            lm_hash_history_offset, \
            lm_hash_history_size, \
            dummy24 = struct.unpack("<IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII", data[:204])

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Parse data fields
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        self.username = self.__get_text(data, name_offset, name_size)
        self.fullname = self.__get_text(data, fullname_offset, fullname_size)
        self.admin_comment = self.__get_text(data, admin_comment_offset, admin_comment_size)
        self.user_comment = self.__get_text(data, user_comment_offset, user_comment_size)
        self.ts_user_parm = TS_User_Parameters(self.__get_field(data, ts_user_parm_offset, ts_user_parm_size))
        self.home_dir = self.__get_text(data, home_dir_offset, home_dir_size)
        self.home_dir_letter = self.__get_text(data, home_dir_letter_offset, home_dir_letter_size)
        self.logon_script = self.__get_text(data, logon_script_offset, logon_script_size)
        self.profile_path = self.__get_text(data, profile_path_offset, profile_path_size)
        self.workstation = self.__get_text(data, workstation_offset, workstation_size)
        self.hours_allowed = self.__get_field(data, hours_allowed_offset, hours_allowed_size)
        self.dummy17 = pymobius.dump(self.__get_field(data, dummy17_offset, dummy17_size))

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Hashes
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        self.lm_hash = self.__get_field(data, lm_hash_offset, lm_hash_size)
        self.nt_hash = self.__get_field(data, nt_hash_offset, nt_hash_size)
        self.nt_hash_history = self.__get_field(data, nt_hash_history_offset, nt_hash_history_size)
        self.lm_hash_history = self.__get_field(data, lm_hash_history_offset, lm_hash_history_size)

        # account is valid
        self.__is_valid = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Check if object is valid
    # @return true/false
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def is_valid(self):
        return self.__is_valid

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get UTF-16 field as UTF-8 text
    # @param data, offset, size
    # @return text
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_text(self, data, offset, size):
        pos = 0xcc
        return data[pos + offset:pos + offset + size].decode('utf-16')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get data field
    # @param data, offset, size
    # @return data field
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_field(self, data, offset, size):
        pos = 0xcc
        return data[pos + offset:pos + offset + size]


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Terminal Server User Parameters decoder
# @author Eduardo Aguiar
# @see https://msdn.microsoft.com/en-us/library/ff635189.aspx
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class TS_User_Parameters(object):
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief data structure
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    structure = [
        [0, 'reserved', 'byte (96)', 'reserved data'],
        [96, 'signature', 'uint16_le', 'signature: letter "P" in UTF-16'],
        [98, 'property_count', 'uint16_le', 'property count']
    ]

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # @param data data buffer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, data):

        # check if data can be parsed
        if len(data) < 100:
            self.__is_valid = False
            return

        # parse data structure
        self.signature, property_count = struct.unpack('96xHH', data[:100])
        self.properties = []
        pos = 100

        for i in range(property_count):
            prop = TS_Property(data[pos:])
            self.properties.append(prop)
            pos += prop.size

        self.__is_valid = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief check if object is valid
    # @return true/false
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def is_valid(self):
        return self.__is_valid


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Terminal Server Property decoder
# @author Eduardo Aguiar
# @see https://msdn.microsoft.com/en-us/library/ff635169.aspx
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class TS_Property(object):
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief data structure
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    structure = [
        [0, 'name_length', 'uint16_le', 'name length'],
        [2, 'value_length', 'uint16_le', 'value length'],
        [4, 'type', 'uint16_le', 'type'],
        [6, 'name', 'var', 'property name'],
        [-1, 'value', 'var', 'property value'],
    ]

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # @param data data buffer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, data):

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # check if data can be parsed
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        if len(data) < 6:
            self.__is_valid = False
            return

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # parse data structure
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        name_length, value_length, self.type = struct.unpack('HHH', data[:6])
        pos = 6

        self.name = data[pos: pos + name_length].decode('utf-16')
        pos += name_length

        value = mobius.core.decoder.hexstring(data[pos: pos + value_length].decode('ascii'))
        pos += value_length

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # format value according to var name
        # @see https://msdn.microsoft.com/en-us/library/ff635169.aspx
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        if self.name in (
        'CtxCfgPresent', 'CtxCfgFlags1', 'CtxCallBack', 'CtxKeyboardLayout', 'CtxNWLogonServer', 'CtxMaxConnectionTime',
        'CtxMaxDisconnectionTime', 'CtxMaxIdleTime', 'CtxShadow'):
            self.value = struct.unpack('<I', value)[0]

        elif self.name in (
        'CtxWFHomeDir', 'CtxWFHomeDirDrive', 'CtxInitialProgram', 'CtxWFProfilePath', 'CtxWorkDirectory',
        'CtxCallbackNumber'):
            self.value = value.decode('cp1252').rstrip(' \0')

        elif self.name == 'CtxMinEncryptionLevel':
            self.value = value[0]

        elif self.name == 'msNPAllowDialin':
            self.value = value.decode('utf-16').rstrip('\0 ')

        else:
            self.value = value

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # set size and is_valid flag
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        self.size = pos
        self.__is_valid = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Check if object is valid
    # @return true/false
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def is_valid(self):
        return self.__is_valid


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ggenerate SAM cryptographic key
# @param registry registry object
# @return SAM key or None
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_sam_key(registry):
    sam_key = None

    data = registry.get_data_by_path('HKLM\\SAM\\SAM\\Domains\\Account\\F')

    if data and len(data.data) >= 4:
        syskey = registry.get_syskey()
        f = data.data
        revision = struct.unpack('<I', f[:4])[0]

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Revision 0x10002 (up to Win8.1)
        # @see http://moyix.blogspot.com.br/2008/02/syskey-and-sam.html
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        if revision == 0x10002:
            md5 = mobius.core.crypt.hash("md5")
            md5.update(f[0x70:0x80])
            md5.update(b'!@#$%^&*()qwertyUIOPAzxcvbnmQQQQQQQQQQQQ)(*@&%\0')
            md5.update(syskey)
            md5.update(b'0123456789012345678901234567890123456789\0')

            rc4 = mobius.core.crypt.new_cipher_stream("rc4", md5.get_digest())
            sam_key = rc4.encrypt(f[0x80:0xa0])[:16]

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Revision 0x10003 (Win10)
        # @see https://github.com/gentilkiwi/mimikatz/wiki (kuhl_m_lsadump_getSamKey)
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        elif revision == 0x10003:
            struct_revision, struct_size, check_len, data_len, salt = struct.unpack('<IIII16s', f[0x68:0x88])
            data = f[0x88:0x88 + data_len]  # followed by check

            c = mobius.core.crypt.new_cipher_cbc("aes", syskey, salt)
            sam_key = c.decrypt(data)[:16]  # AES-128 (16 bytes key)

        else:
            mobius.core.logf('DEV Domains\\Account\\F unknown revision: %08x' % revision)

    return sam_key


class passwd_hash(object):
    pass


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief OS User account
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class user_account(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, sam_key, f, v):
        self.__sam_key = sam_key
        f = SAM_F(f.data)
        v = SAM_V(v.data)

        if not f.is_valid() or not v.is_valid():
            self.__is_valid = False
            return

        # set data from F value
        self.f_data_revision = f.revision
        self.last_logon_datetime = f.last_logon_datetime
        self.last_password_set_datetime = f.last_password_set_datetime
        self.last_failed_logon_datetime = f.last_failed_logon_datetime
        self.password_expiration_datetime = f.password_expiration_datetime
        self.rid = f.rid
        self.gid = f.gid
        self.country_code = f.country_code
        self.failed_logon_count = f.failed_logon_count
        self.logon_count = f.logon_count
        self.acb_flags = f.flags
        self.flag_account_enabled = f.flag_account_enabled
        self.flag_home_dir_required = f.flag_home_dir_required
        self.flag_password_required = f.flag_password_required
        self.flag_temporary_duplicated_account = f.flag_temporary_duplicated_account
        self.flag_normal_user_account = f.flag_normal_user_account
        self.flag_mns_logon_user_account = f.flag_mns_logon_user_account
        self.flag_interdomain_trust_account = f.flag_interdomain_trust_account
        self.flag_workstation_trust_account = f.flag_workstation_trust_account
        self.flag_server_trust_account = f.flag_server_trust_account
        self.flag_password_expires = f.flag_password_expires
        self.flag_account_auto_locked = f.flag_account_auto_locked

        # set data from V value
        self.v_data_revision = v.revision
        self.username = v.username
        self.fullname = v.fullname
        self.admin_comment = v.admin_comment
        self.user_comment = v.user_comment
        self.home_dir = v.home_dir
        self.home_dir_letter = v.home_dir_letter
        self.logon_script = v.logon_script
        self.profile_path = v.profile_path
        self.workstation = v.workstation

        if v.ts_user_parm.is_valid():
            self.ts_properties = v.ts_user_parm.properties
        else:
            self.ts_properties = []

        # decrypt password hashes
        self.hashes = []

        if v.nt_hash:
            self.hashes += self.__decrypt_user_hash(v.nt_hash, "nt", True)

        if v.lm_hash:
            self.hashes += self.__decrypt_user_hash(v.lm_hash, "lm", True)

        if v.nt_hash_history:
            self.hashes += self.__decrypt_user_hash(v.nt_hash_history, "nt", False)

        if v.lm_hash_history:
            self.hashes += self.__decrypt_user_hash(v.lm_hash_history, "lm", False)

        self.__is_valid = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Check if object is valid
    # @return true/false
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def is_valid(self):
        return self.__is_valid

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Decrypt user hash
    # @param hdata hash encrypted data
    # @param type (NT, LM)
    # @todo revision >= 0x20000 (AES)
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __decrypt_user_hash(self, data, htype, is_current):
        hashes = []
        pekid, revision = struct.unpack('<HH', data[:4])

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # DES final key1 and key2 values
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        rid_data = struct.pack('<I', self.rid)
        key1 = rid_data + rid_data[:3]
        key2 = rid_data[3:4] + rid_data + rid_data[:2]

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Revision 0x01 - Up to Win8.1
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        if revision == 1:
            if htype == "nt":
                if is_current:
                    ntlm_str = b"NTPASSWORD\0"
                else:
                    ntlm_str = b"NTPASSWORDHISTORY\0"

            elif htype == "lm":
                if is_current:
                    ntlm_str = b"LMPASSWORD\0"
                else:
                    ntlm_str = b"LMPASSWORDHISTORY\0"

            else:
                mobius.core.logf(f"DEV unhandled hash type: {htype}")
                return []

            md5 = mobius.core.crypt.hash("md5")
            md5.update(self.__sam_key)
            md5.update(rid_data)
            md5.update(ntlm_str)

            rc4 = mobius.core.crypt.new_cipher_stream("rc4", md5.get_digest())
            obfkey = rc4.decrypt(data[4:])

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Revision 0x02 - Win10
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        elif revision == 2:
            aes_revision, data_size = struct.unpack('<II', data[:8])

            if data_size:
                salt = data[8:24]
                hdata = data[24:24 + data_size]
                c = mobius.core.crypt.new_cipher_cbc("aes", self.__sam_key, salt)
                obfkey = c.decrypt(hdata)

            else:
                obfkey = b''

        else:
            mobius.core.logf(f"DEV SAM_HASH unknown revision: {revision:d}")
            return []

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Get hashes from obfkey
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        NTLM_HASH_SIZE = 16

        for i in range(0, len(obfkey), NTLM_HASH_SIZE):
            h = passwd_hash()
            h.revision = revision
            h.pekid = pekid
            h.type = htype
            h.is_current = is_current

            des1 = mobius.core.crypt.new_cipher_ecb("des", key1)
            des2 = mobius.core.crypt.new_cipher_ecb("des", key2)
            h.value = des1.decrypt(obfkey[i:i + 8]) + des2.decrypt(obfkey[i + 8:i + NTLM_HASH_SIZE])
            hashes.append(h)

        return hashes


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get user accounts
# @param registry registry object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get(registry):
    accounts = []
    sam_key = get_sam_key(registry)

    for key in registry.get_key_by_mask('HKLM\\SAM\\SAM\\Domains\\Account\\Users\\[0-9]*'):
        f = key.get_data_by_name('F')
        v = key.get_data_by_name('V')

        if f and v:
            account = user_account(sam_key, f, v)
            accounts.append(account)

    return accounts
