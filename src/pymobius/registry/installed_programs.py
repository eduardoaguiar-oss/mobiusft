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
import traceback

import mobius
import pymobius
from pymobius.registry import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get installed programs from uninstall key
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_programs_from_uninstall_key(username, uninstall_key, key_path):
    programs = []

    if uninstall_key:
        for key in uninstall_key.subkeys:
            try:
                program = get_program_from_uninstall_subkey(key, username, f"{key_path}")
                programs.append(program)
            except Exception as e:
                mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

    return programs


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get installed program from uninstall subkey
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_program_from_uninstall_subkey(key, username, key_path):
    p = pymobius.Data()
    p.username = username
    p.last_modification_time = key.last_modification_time
    p.display_name = get_data_as_string(key.get_data_by_name('DisplayName')) or key.name
    p.name = p.display_name
    p.install_location = get_data_as_string(key.get_data_by_name('InstallLocation'))
    p.install_source = get_data_as_string(key.get_data_by_name('InstallSource'))
    p.uninstall_string = get_data_as_string(key.get_data_by_name('UninstallString'))
    p.publisher = get_data_as_string(key.get_data_by_name('Publisher'))
    p.comments = get_data_as_string(key.get_data_by_name('Comments'))
    p.url_info_about = get_data_as_string(key.get_data_by_name('URLInfoAbout'))
    p.url_update_info = get_data_as_string(key.get_data_by_name('URLUpdateInfo'))
    p.help_link = get_data_as_string(key.get_data_by_name('HelpLink'))
    p.evidence_source = f"Registry key {key_path}\\{key.name}"

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # version
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    p.version = ''

    display_version_data = key.get_data_by_name('DisplayVersion')
    if display_version_data:
        p.version = display_version_data.get_data_as_string('utf-16')

    if not p.version:
        version_data = key.get_data_by_name("Version")
        REG_SZ = 1
        REG_DWORD = 4

        if version_data:
            if version_data.type == REG_SZ:
                p.version = get_data_as_string(version_data)

            elif version_data.type == REG_DWORD:
                version = version_data.get_data_as_dword()
                p.version = f"{version >> 24:d}.{(version >> 16) & 0xff:d}.{version & 0xffff:d}"

            else:
                mobius.core.log(f"DEV unhandled version type: {version_data.type:d}. Key path: {key_path}")

    if not p.version:
        major_data = key.get_data_by_name('VersionMajor')
        minor_data = key.get_data_by_name('VersionMinor')

        if major_data and minor_data:
            p.version = f"{major_data.get_data_as_dword():d}.{minor_data.get_data_as_dword():d}"

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # installdate
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    data = key.get_data_by_name('InstallDate')

    if not data:
        p.install_date = ''

    elif data.size == 4:
        p.install_date = get_data_as_unix_datetime(data)

    else:
        p.install_date = get_data_as_string(data)

        if len(p.install_date) == 8:
            p.install_date = f"{p.install_date[:4]}-{p.install_date[4:6]}-{p.install_date[6:]}"

    return p


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get installed programs
# @author Eduardo Aguiar
# @param registry Registry object
# @reference Forensic Analysis of the Windows Registry, by Lih Wern Wong, p.8
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get(registry):
    programs = []

    programs += get_programs_from_uninstall_key(
        None,
        registry.get_key_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall'),
        'HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall')

    programs += get_programs_from_uninstall_key(
        None,
        registry.get_key_by_path('HKLM\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall'),
        'HKLM\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall')

    for username, key in iter_hkey_users(registry):
        programs += get_programs_from_uninstall_key(
            username,
            key.get_key_by_path('Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall'),
            'HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall')

        programs += get_programs_from_uninstall_key(
            username,
            key.get_key_by_path('Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall'),
            'HKCU\\Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall')

    return programs
