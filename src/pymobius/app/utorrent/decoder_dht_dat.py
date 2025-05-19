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
        mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode data from dht.dat file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_file(f):
    # check file
    if not f or f.is_reallocated():
        return

    # try to open file
    reader = f.new_reader()

    if not reader:
        return

    # decode BTEncode data
    metadata = mobius.core.decoder.btencode(reader)

    # create data object
    data = pymobius.Data()
    data.metadata = metadata
    data.ip = None
    data.timestamp = None
    data.guid = None
    data.evidence_source = pymobius.get_evidence_source_from_file(f)

    # IP address
    if 'ip' in metadata:
        decoder = mobius.core.decoder.data_decoder(metadata.get('ip'))
        data.ip = decoder.get_ipv4_be()

    # Age
    data.timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(metadata.get('age'))

    # Client GUID
    if 'id' in metadata:
        data.guid = mobius.core.encoder.hexstring(metadata.get('id')).upper()

    return data
