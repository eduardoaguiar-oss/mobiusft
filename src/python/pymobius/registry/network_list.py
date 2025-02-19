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
import datetime

import mobius
import pymobius
from pymobius.registry import get_data_as_dword
from pymobius.registry import get_data_as_string


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Return datetime for a given profile date
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_nl_profile_datetime(data):
    value = None

    if data:
        decoder = mobius.decoder.data_decoder(data.data)

        year = decoder.get_uint16_le()
        month = decoder.get_uint16_le()
        decoder.skip(2)     # weekday
        day = decoder.get_uint16_le()
        hh = decoder.get_uint16_le()
        mm = decoder.get_uint16_le()
        ss = decoder.get_uint16_le()
        ms = decoder.get_uint16_le()

        value = datetime.datetime(year, month, day, hh, mm, ss, ms * 1000)

    return value


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get Network profiles (Wireless)
# @author Eduardo Aguiar
# @param registry Registry object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get(registry):
    profiles = []
    network_list_path = 'HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkList'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # get BSSIDs
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bssids = {}

    for name in ('Managed', 'Unmanaged'):
        for key in registry.get_key_by_mask(f'{network_list_path}\\Signatures\\{name}\\*'):
            profile_guid = get_data_as_string(key.get_data_by_name('ProfileGuid'))[1:-1]
            default_gateway_mac = key.get_data_by_name('DefaultGatewayMac')

            if profile_guid and default_gateway_mac:
                bssid = mobius.encoder.hexstring(default_gateway_mac.data[:6], ':')
                bssids[profile_guid] = bssid

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # get profiles
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for key in registry.get_key_by_mask(f'{network_list_path}\\Profiles\\*'):
        profile = pymobius.Data()
        profile.guid = key.name[1:-1]
        profile.ssid = get_data_as_string(key.get_data_by_name('ProfileName'))
        profile.bssid = bssids.get(profile.guid)
        profile.description = get_data_as_string(key.get_data_by_name('Description'))
        profile.is_managed = get_data_as_dword(key.get_data_by_name('Managed'))
        profile.date_created = get_nl_profile_datetime(key.get_data_by_name('DateCreated'))
        profile.date_last_connected = get_nl_profile_datetime(key.get_data_by_name('DateLastConnected'))
        profile.evidence_source = f"Registry Key {network_list_path}\\Profiles\\{key.name}"

        profiles.append(profile)

    return profiles
