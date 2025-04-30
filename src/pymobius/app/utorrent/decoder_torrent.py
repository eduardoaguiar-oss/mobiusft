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
import os.path
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
# @brief Decode data from .torrent file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_file(f):
    # try to open file
    reader = f.new_reader()
    if not reader:
        return

    # decode BTEncode data
    metadata = mobius.decoder.btencode(reader)

    if 'info' not in metadata:
        return

    info = metadata.get('info')

    # create data object
    files = []
    name = (info.get('name.utf-8') or info.get('name') or b'').decode('utf-8')

    if 'files' in info:

        for f_metadata in info.get('files'):
            paths = info.get('path.utf-8') or info.get('path') or []

            for path in paths:
                t_file = pymobius.Data()
                t_file.size = f_metadata.get('length')
                t_file.path = path.decode('utf-8')
                t_file.name = os.path.basename(t_file.path)
                t_file.torrent_path = mobius.io.to_win_path(f.path)
                t_file.torrent_name = f.name
                files.append(t_file)

    else:
        t_file = pymobius.Data()
        t_file.name = name
        t_file.path = ''
        t_file.size = info.get('length')
        t_file.torrent_path = mobius.io.to_win_path(f.path)
        t_file.torrent_name = f.name
        files.append(t_file)

    return files
