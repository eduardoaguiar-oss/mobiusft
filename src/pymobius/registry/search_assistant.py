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
import mobius
import pymobius
import pymobius.registry

XP_TARGETS = {'5001': 'internet',
              '5603': 'filename',
              '5604': 'file-content',
              '5647': 'computers'}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get Search Assistant data
# @author Eduardo Aguiar
# @param registry Registry object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get(registry):
    for username, key in pymobius.registry.iter_hkey_users(registry):

        for search_key in key.get_key_by_mask("Software\\Microsoft\\Search Assistant\\ACMru\\*"):
            if search_key.name in XP_TARGETS:
                for value in search_key.values:
                    data = pymobius.Data()
                    data.username = username
                    data.app_id = 'win'
                    data.app_name = 'Windows OS'
                    data.fieldname = XP_TARGETS.get(search_key.name)
                    data.value = pymobius.registry.get_data_as_string(value.data)
                    data.evidence_source = f"Registry key <HKCU\\Software\\Microsoft\\Search Assistant\\ACMru\\{search_key.name}>"
                    yield data

            else:
                mobius.core.logf(f"DEV ACMru unknown subkey: {search_key.name}")
