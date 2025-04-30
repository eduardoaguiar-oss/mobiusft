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
import traceback

import mobius
import pymobius


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(f):
    try:
        return decode_file(f)

    except Exception as e:
        mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode data from settings.dat file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_file(f):
    # try to open file
    reader = f.new_reader()
    if not reader:
        return

    # decode BTEncode data
    metadata = mobius.decoder.btencode(reader)

    # create data object
    data = pymobius.Data()
    data.metadata = metadata

    # attributes
    data.autostart = metadata.pop('autostart', 0) == 1
    data.runs = metadata.pop('runs_since_born')
    data.total_downloaded = metadata.pop('td')
    data.total_uploaded = metadata.pop('tu')
    data.initial_version = get_version(metadata.pop('initial_install_version'))
    data.version = get_version(metadata.pop('v'))

    timestamp = metadata.pop('born_on', 0) * 10000000
    data.born_on_time = mobius.datetime.new_datetime_from_nt_timestamp(timestamp)

    timestamp = metadata.pop('bin_change', 0) * 10000000
    data.bin_change_time = mobius.datetime.new_datetime_from_nt_timestamp(timestamp)

    timestamp = metadata.pop('cold_on', 0) * 10000000
    data.cold_on_time = mobius.datetime.new_datetime_from_nt_timestamp(timestamp)

    timestamp = metadata.pop('settings_saved_systime', 0)
    data.last_settings_time = mobius.datetime.new_datetime_from_unix_timestamp(timestamp)

    return data


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode version
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_version(v):
    if v:
        build = v & 0xffff
        v = v >> 16
        return f'{v >> 9}.{(v >> 5) & 0xf}.{(v >> 1) & 0xf} build {build:d}'

    return None
