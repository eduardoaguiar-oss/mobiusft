# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import pymobius


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get party data
# @param Party object
# @return Party data object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_party_data(party):
    data = pymobius.Data()
    data.party_id = None
    data.party_name = None
    data.role = None
    data.is_phone_owner = None
    data.label = None

    if party:
        metadata = dict(party.metadata)

        data.party_id = metadata.get('Identifier')
        data.party_name = metadata.get('Name')
        data.role = metadata.get('Role')
        data.is_phone_owner = bool(metadata.get('IsPhoneOwner'))

        if data.party_id and data.party_name:
            data.label = f"{data.party_name} (ID={data.party_id})"

        else:
            data.label = data.party_name or data.party_id or ''

        if data.is_phone_owner:
            if data.label:
                data.label += '*'
            else:
                data.label = '<this-phone>'

    return data