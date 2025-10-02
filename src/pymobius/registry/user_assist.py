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
from pymobius.registry import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Known GUIDs
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
GUID = {'5E6AB780-7743-11CF-A12B-00AA004AE837': 'Microsoft Internet Toolbar',
        '75048700-EF1F-11D0-9888-006097DEACF9': 'Active Desktop'}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get user assist data
# @todo use KNOWNFOLDERID
# @see https://msdn.microsoft.com/en-us/library/windows/desktop/bb776911(v=vs.85).aspx
# @see https://msdn.microsoft.com/en-us/library/windows/desktop/dd378457(v=vs.85).aspx
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get(registry):
    folder_paths = {}

    for username, user_key in iter_hkey_users(registry):

        for guid_key in user_key.get_key_by_mask(
                'Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist\\*'):
            guid = guid_key.name[1:-1]
            guid = GUID.get(guid, guid)
            version_data = guid_key.get_data_by_name('Version')
            version = version_data.get_data_as_dword()

            for value in guid_key.get_value_by_mask('Count\\*'):
                location, count, date = decode_user_assist(value, version)

                # translate Vista and later GUID folders
                if location.startswith('{'):
                    pos = location.find('}')
                    folder_guid = location[1:pos].lower()

                    folder_path = folder_paths.get(folder_guid)
                    if not folder_path:
                        folder_path = get_folder_path_from_guid(registry, folder_guid)
                        folder_paths[folder_guid] = folder_path

                    if folder_path:
                        location = folder_path + location[pos + 1:]

                yield username, guid, date, count, location


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief decode user assist structure
# @see https://www.aldeid.com/wiki/Windows-userassist-keys
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_user_assist(value, version):
    location = value.name

    data = value.data.data
    count = -1
    date = ''

    # up to Vista
    if version == 3:

        if location == 'UEME_CTLSESSION':
            unknown1, count = struct.unpack('<II', data)

        elif len(data) == 16:
            session, count, timestamp = struct.unpack('<IIQ', data)
            date = get_nt_datetime(timestamp)

            if date:
                count = count - 5  # counter starts at 5

    # Win7 and later
    elif version == 5:

        if location == 'UEME_CTLSESSION':
            count = struct.unpack('<I', data[4:8])[0]

        else:
            count = struct.unpack('<I', data[4:8])[0]
            timestamp = struct.unpack('<Q', data[60:68])[0]
            date = get_nt_datetime(timestamp)

    return location, count, date
