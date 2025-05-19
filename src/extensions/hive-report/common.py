# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023 Eduardo Aguiar
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
import re
import struct

import mobius
import mobius.core.datetime

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Known SIDS
# @reference http://support.microsoft.com/kb/243330
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
KNOWN_SIDS = {'S-1-5-6': 'service',
              'S-1-5-7': 'anonymous',
              'S-1-5-8': 'proxy',
              'S-1-5-18': 'local system',
              'S-1-5-19': 'local service',
              'S-1-5-20': 'network service'}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get data as string
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_data_as_string(data, value=''):
    return data.get_data_as_string('utf-16') if data else value


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get data as buffer
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_data_as_buffer(data, value=b''):
    return data.data if data else value


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get data as dword
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_data_as_dword(data, value=None):
    return data.get_data_as_dword() if data else value


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get data as qword
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_data_as_qword(data, value=None):
    return data.get_data_as_qword() if data else value


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get data as UNIX datetime
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_data_as_unix_datetime(data, value=None):
    v = None

    if data:
        timestamp = data.get_data_as_dword()
        d = mobius.core.datetime.new_datetime_from_unix_timestamp(timestamp)
        v = str(d)[:19]

    return v


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get data as NT datetime
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_data_as_nt_datetime(data, value=None):
    return get_nt_datetime(data.get_data_as_qword()) if data else value


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get NT datetime from timestamp
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_nt_datetime(timestamp):
    d = mobius.core.datetime.new_datetime_from_nt_timestamp(timestamp)
    return str(d)[:19]


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get computer name
# @param registry registry
# @reference http://support.microsoft.com/kb/102987/en-us
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_computer_name(registry):
    try:
        computername = get_data_as_string(registry.get_data_by_path(
            'HKLM\\SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ComputerName\\ComputerName'))
    except Exception as e:
        mobius.core.logf('WRN ' + str(e))
        computername = None

    return computername


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get username from SID
# @reference http://support.microsoft.com/kb/243330
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_username_from_sid(registry, sid):
    username = None

    # known SID
    if sid in KNOWN_SIDS:
        return KNOWN_SIDS.get(sid)

    # get username from RID
    v = registry.get_data_by_path('HKLM\\SAM\\SAM\\Domains\\Account\\V')
    if v:
        machine_sid_prefix = 'S-1-5-21-%d-%d-' % struct.unpack('<II', v.data[-12:-4])

        if sid.startswith(machine_sid_prefix):
            rid = int(sid.split('-')[-1])
            data = registry.get_data_by_path('HKLM\\SAM\\SAM\\Domains\\Account\\Users\\%08X\\V' % rid)

            if data:
                offset, size = struct.unpack('<II', data.data[12:20])
                username = data.data[0xcc + offset:0xcc + offset + size].decode('utf-16')

    # get username from NTUSER.DAT, if available
    else:
        user_key = registry.get_key_by_path('HKU\\%s' % sid)

        if user_key:
            username = user_key.get_data_by_path(
                'Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Logon User Name') or \
                       user_key.get_data_by_path(
                           'Software\\Microsoft\\Active Setup\\Installed Components\\{44BBA840-CC51-11CF-AAFA-00AA00B6015C}\\Username')

    # return username
    return username or sid


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief iterate through HKEY_USERS keys
# @reference http://www.systemlookup.com/ActiveSetup/292-setup50_exe.html
# GUID 44BBA840-CC51-11CF-AAFA-00AA00B6015C Microsoft Outlook Express setup library
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def iter_hkey_users(registry):
    for key in registry.get_key_by_mask('HKU\\*'):

        if key.name == '.DEFAULT':
            username = '.DEFAULT'

        else:
            username = get_data_as_string(
                key.get_data_by_path('Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Logon User Name')) or \
                       get_data_as_string(key.get_data_by_path(
                           'Software\\Microsoft\\Active Setup\\Installed Components\\{44BBA840-CC51-11CF-AAFA-00AA00B6015C}\\Username')) or \
                       get_username_from_sid(registry, key.name) or key.name

        yield username, key


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief iterate through Protected Storage SID keys
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def iter_pssp_keys(registry):
    for username, user_key in iter_hkey_users(registry):
        for sid_key in user_key.get_key_by_mask('Software\\Microsoft\\Protected Storage System Provider\\*'):
            yield username, sid_key


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief iterate through MRUList key structure
# @param key registry key
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def iter_mrulist(key):
    mrudata = get_data_as_string(key.get_data_by_name('MRUList'))

    for idx in reversed(mrudata):
        data = get_data_as_string(key.get_data_by_name(idx), '')
        yield data


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief iterate through MRUListEx key structure
# @param key registry key
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def iter_mrulistex(key):
    mrudata = get_data_as_buffer(key.get_data_by_name('MRUListEx'))
    mrusize = len(mrudata) // 4
    mrulist = struct.unpack('<%di' % mrusize, mrudata)

    for idx in reversed(mrulist):
        if idx != -1:
            data = get_data_as_string(key.get_data_by_name(str(idx)))
            yield data


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief expand EXPAND_SZ
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def expand_sz(value, environment_vars):
    # replace function
    def repl(matchobj):
        name = matchobj.group(1).lower()
        return environment_vars.get(name, '%%%s%%' % name)

    return re.sub(r'%(.*?)%', repl, value or '')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get product key from digital product ID
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_product_key(digital_product_id):
    if digital_product_id:
        digits = 'BCDFGHJKMPQRTVWXY2346789'
        data = list(digital_product_id[52:68])
        out = ''

        for i in range(29):
            if (i + 1) % 6 == 0:
                out = '-' + out
            else:
                idx = 0
                for j in range(14, -1, -1):
                    b = (idx << 8) | data[j]
                    data[j] = b // 24
                    idx = b % 24
                out = digits[idx] + out
    else:
        out = None

    return out
