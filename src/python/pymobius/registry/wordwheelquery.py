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
import pymobius
import pymobius.registry


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get WordWheelQuery key data
# @author Eduardo Aguiar
# @param registry Registry object
# @todo Handle HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Word\WordWheelQuery (Windows Vista)
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get(registry):
    for username, key in pymobius.registry.iter_hkey_users(registry):

        # Win 7 and above
        ww_key = key.get_key_by_path('Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\WordWheelQuery')
        if ww_key:
            for idx, reg_data in pymobius.registry.iter_mrulistex(ww_key):
                data = pymobius.Data()
                data.fieldname = 'search'
                data.value = pymobius.registry.get_data_as_string(reg_data)
                data.app_id = 'win'
                data.app_name = 'Windows OS'
                data.username = username
                data.idx = idx
                data.evidence_source = "Registry key <HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\WordWheelQuery>"
                yield data
