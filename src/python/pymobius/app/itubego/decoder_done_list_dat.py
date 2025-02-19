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
import json
import traceback

import mobius
import pymobius
import pymobius.app.itubego


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(f):
    try:
        return decode_file(f)

    except Exception as e:
        print(e)
        mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode data from done_list.dat file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_file(f):
    entries = []

    # try to open file
    reader = f.new_reader()
    if not reader:
        return entries

    # decode file
    decoder = mobius.decoder.qdatastream(reader)
    num_entries = decoder.get_qint32()

    for i in range(num_entries):
        entry = decode_entry(decoder)
        entry.idx = i + 1
        entry.source = f"{f.path} (i-node: {f.inode})"
        entry.is_deleted = f.is_deleted()

        entries.append(entry)

    return entries


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode entry from done_list.dat file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_entry(decoder):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Create entry
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    data = pymobius.Data()
    data.metadata = mobius.pod.map()
    data.file_path = None
    data.file_size = -1
    data.bytes_downloaded = -1
    data.file_type = None
    data.temp_file_path = None
    data.thumbnail_url = None
    data.title = None
    data.download_state = 'Completed'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Decode data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    data.url = decoder.get_qstring()

    version = decoder.get_qint32()
    if version != 4:
        mobius.core.logf(f"DEV unknown version: {version}")

    sniff_data = decoder.get_qvariant()
    finished_data = decoder.get_qvariant()
    data.dst_folder = decoder.get_qstring()

    # u1 is probably one of the following: quint32,qbitarray,qdate,qhash,qicon,
    # qimage,qlist,qmap,qpicture,qtime,qvector
    u1 = decoder.get_qint32()
    mobius.core.logf(f"DEV u1: {u1}")

    data.thumbnail_path = decoder.get_qstring()
    downloading_data = decoder.get_qvariant()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Retrieve data from sniff data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for name, value in decode_json(sniff_data, "sniff"):
        if name == "title":
            data.title = value
        elif name == "thumbnail":
            data.thumbnail_url = value
        elif name == "filepath":
            data.file_path = value
        else:
            data.metadata.set(name, value)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Retrieve data from finished data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for name, value in decode_json(finished_data, "finished"):
        if name == "filesize":
            data.file_size = value
            data.bytes_downloaded = value
        elif name == "filetype":
            data.file_type = value
        elif name == "filepath":
            data.temp_file_path = value
        else:
            data.metadata.set(name, value)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Retrieve data from downloading data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for name, value in decode_json(downloading_data, "downloading"):
        data.metadata.set(name, value)

    return data


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode entry from done_list.dat file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_json(data, record_type):
    d = json.loads(data) or {}
    d_type = d.get("type")
    d_msg = d.get("msg", {})

    if d_type is not None and d_type != record_type:
        mobius.core.logf(f"DEV expect record_type '{record_type}', but received '{d_type}'")

    return d_msg.items()
