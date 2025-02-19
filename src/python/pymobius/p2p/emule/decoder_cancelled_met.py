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
import traceback
import mobius
from . import CTag

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
CANCELLED_HEADER_OLD = 0x0e
CANCELLED_HEADER = 0x0f
CANCELLED_VERSION = 0x01

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief generic data holder
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class dataholder (object):
  pass

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode (f):
  try:
    return decode_file (f) 
  except Exception as e:
    mobius.core.logf ('WRN %s %s' % (str (e), traceback.format_exc ()))

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode data from Cancelled.met
# @see CKnownFileList::LoadCancelledFiles - srchybrid/KnownFileList.cpp
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_file (f):

  # try to open file
  reader = f.new_reader ()
  if not reader:
    return

  # decode header
  decoder = mobius.decoder.data_decoder (reader)

  header = decoder.get_uint8 ()
  if header not in (CANCELLED_HEADER, CANCELLED_HEADER_OLD):
    return

  if header == CANCELLED_HEADER:
    version = decoder.get_uint8 ()

    if version > CANCELLED_VERSION:
      mobius.core.logf ("DEV Version %d > %d" % (version, CANCELLED_VERSION))

    seed = decoder.get_uint32_le ()

  # decode entries
  data = dataholder ()
  data.entries = []
  count = decoder.get_uint32_le ()

  for i in range (count):
    entry = dataholder ()
    entry.hash_ed2k = mobius.encoder.hexstring (decoder.get_bytearray_by_size (16))
    entry.tags = []

    tag_count = decoder.get_uint8 ()

    for j in range (tag_count):
      tag = CTag.decode (decoder)
      entry.tags.append (tag)

    if header == CANCELLED_HEADER_OLD:
      pass # @todo convert old hash to new hash

    data.entries.append (entry)

  # return data
  return data
