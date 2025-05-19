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
    metadata = mobius.core.decoder.btencode(reader)

    # create data object
    entries = []

    # decode entries
    for key, value in metadata.items():
        try:
            if key.endswith('.torrent'):
                entry = decode_entry(key, value)
                entry.resume_dat_path = mobius.core.io.to_win_path(f.path)
                entry.resume_dat_modification_time = f.modification_time
                entries.append(entry)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    return entries


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode entry from settings.dat file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_entry(key, value):
    data = pymobius.Data()
    data.name = key
    data.metadata = value
    data.download_url = data.metadata.pop('download_url', b'').decode('utf-8')
    data.caption = data.metadata.pop('caption', b'').decode('utf-8')
    data.path = data.metadata.pop('path', b'').decode('utf-8')
    data.seeded_seconds = data.metadata.pop('seedtime', 0)
    data.downloaded_seconds = data.metadata.pop('runtime', 0) - data.seeded_seconds
    data.blocksize = data.metadata.pop('blocksize', 0)
    data.bytes_downloaded = data.metadata.pop('downloaded', 0)
    data.bytes_uploaded = data.metadata.pop('uploaded', 0)

    timestamp = data.metadata.pop('time', 0)
    data.metadata_time = mobius.core.datetime.new_datetime_from_unix_timestamp(timestamp)

    timestamp = data.metadata.pop('added_on', 0)
    data.added_time = mobius.core.datetime.new_datetime_from_unix_timestamp(timestamp)

    timestamp = data.metadata.pop('completed_on', 0)
    data.completed_time = mobius.core.datetime.new_datetime_from_unix_timestamp(timestamp)

    timestamp = data.metadata.pop('last seen complete', 0)
    data.last_seen_complete_time = mobius.core.datetime.new_datetime_from_unix_timestamp(timestamp)

    # peers
    data.peers = []

    if 'peers' in data.metadata:
        peers = data.metadata.pop('peers')
        decoder = mobius.core.decoder.data_decoder(peers)

        while decoder.tell() < decoder.get_size():
            ip = decoder.get_ipv4_be()
            port = decoder.get_uint16_le()
            data.peers.append((ip, port))

    elif 'peers6' in data.metadata:
        peers6 = data.metadata.pop('peers6')
        decoder = mobius.core.decoder.data_decoder(peers6)

        while decoder.tell() < decoder.get_size():
            ip = decoder.get_ipv4_mapped_ipv6()
            port = decoder.get_uint16_le()
            data.peers.append((ip, port))

    return data
