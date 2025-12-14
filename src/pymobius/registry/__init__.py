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
import datetime
import re
import struct

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief known SIDs
# @reference http://support.microsoft.com/kb/243330
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
KNOWN_SIDS = {'S-1-5-6': 'service',
              'S-1-5-7': 'anonymous',
              'S-1-5-8': 'proxy',
              'S-1-5-18': 'local system',
              'S-1-5-19': 'local service',
              'S-1-5-20': 'network service'}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get data as dword
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_data_as_dword(data, value=None):
    if data is None:
        return value
    else:
        return data.get_data_as_dword()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get data as buffer
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_data_as_buffer(data, value=b''):
    return data.data if data else value


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get data as string
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_data_as_string(data, value=''):
    return data.get_data_as_string('utf-16') if data else value


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get data as UNIX datetime
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_data_as_unix_datetime(data, value=None):
    return get_unix_datetime(data.get_data_as_dword()) if data else value


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get data as NT datetime
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_data_as_nt_datetime(data, value=None):
    return get_nt_datetime(data.get_data_as_qword()) if data else value


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get NT datetime from timestamp
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_nt_datetime(timestamp):
    if timestamp:
        delta = datetime.timedelta(seconds=timestamp // 10000000)
        date = datetime.datetime(1601, 1, 1, 0, 0, 0) + delta
        text = str(date)[:19]
    else:
        text = ''

    return text


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get Unix datetime from registry value
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_unix_datetime(timestamp):
    if timestamp:
        delta = datetime.timedelta(seconds=timestamp)
        date = datetime.datetime(1970, 1, 1, 0, 0, 0) + delta
        text = date.strftime('%F %T')
    else:
        text = ''

    return text


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get computer name
# @param registry registry
# @reference http://support.microsoft.com/kb/102987/en-us
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_computer_name(registry):
    try:
        computername = get_data_as_string(registry.get_data_by_path(
            'HKLM\\SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ComputerName\\ComputerName'))
    except:
        computername = None

    return computername


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get username from SID
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_username_from_sid(registry, sid):
    username = KNOWN_SIDS.get(sid)

    if not username:
        rid = int(sid.split('-')[-1])
        data = registry.get_data_by_path(f'HKLM\\SAM\\SAM\\Domains\\Account\\Users\\{rid:08X}\\V')

        if data:
            offset, size = struct.unpack('II', data.data[12:20])
            username = data.data[0xcc + offset:0xcc + offset + size].decode('utf-16')

    return username


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get folder path from GUID
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_folder_path_from_guid(registry, guid):
    path = None
    key = registry.get_key_by_path(
        f'\\HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FolderDescriptions\\{{{guid}}}')

    if key:
        name = get_data_as_string(key.get_data_by_name('Name'))
        relative_path = get_data_as_string(key.get_data_by_name('RelativePath'))
        parent_folder = get_data_as_string(key.get_data_by_name('ParentFolder'))

        if relative_path:
            path = relative_path

            if parent_folder:
                parent_guid = parent_folder[1:-1]
                parent_path = get_folder_path_from_guid(registry, parent_guid)

                if parent_path:
                    path = f'{parent_path}\\{path}'

        elif name:
            path = f'%{name}%'

    return path


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get system folder path
# @param registry registry object
# @param name folder name
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_system_folder(registry, name):
    value = None

    # get folder path
    if name == 'PROGRAMFILES':
        value = get_data_as_string(
            registry.get_data_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ProgramFilesDir'))

    elif name == 'PROFILES':
        value = get_data_as_string(registry.get_data_by_path(
            'HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\ProfilesDirectory'))

    elif name == 'WINDOWS':
        value = get_data_as_string(
            registry.get_data_by_path('HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment\\windir'))

    elif name == 'SYSTEMROOT':
        value = get_data_as_string(
            registry.get_data_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SystemRoot'))

    # replace systemdrive and systemroot
    if value and '%system' in value.lower():
        systemroot = get_data_as_string(
            registry.get_data_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SystemRoot'))
        systemdrive = systemroot.split('\\')[0]
        value = expand_sz(value, {'systemdrive': systemdrive, 'systemroot': systemroot})

    return value


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Iterate through HKEY_USERS keys
# @reference http://www.systemlookup.com/ActiveSetup/292-setup50_exe.html
# GUID 44BBA840-CC51-11CF-AAFA-00AA00B6015C Microsoft Outlook Express setup library
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def iter_hkey_users(registry):
    for key in registry.get_key_by_mask('\\HKU\\*'):

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
# @brief Iterate through MRUList key structure
# @param key Registry key
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def iter_mrulist(key):
    mrudata = get_data_as_string(key.get_data_by_name('MRUList'))

    for idx in reversed(mrudata):
        yield key.get_data_by_name(idx)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Iterate through MRUListEx key structure
# @param key Registry key
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def iter_mrulistex(key):
    mrudata = get_data_as_buffer(key.get_data_by_name('MRUListEx'))
    mrusize = len(mrudata) // 4
    mrulist = struct.unpack('<%di' % mrusize, mrudata)

    for idx in reversed(mrulist):
        if idx != -1:
            yield idx, key.get_data_by_name(str(idx))


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief expand EXPAND_SZ
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def expand_sz(value, environment_vars):
    # replace function
    def repl(matchobj):
        name = matchobj.group(1).lower()
        return environment_vars.get(name, f'%{name}%')

    return re.sub(r'%(.*?)%', repl, value or '')
