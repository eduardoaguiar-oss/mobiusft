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
# How search is stored inside key_index.dat:
#
# Key: Each key stores the MD4 hash of lowercase text being searched. Usually,
# it is a substring of filenames, so we can recover the search text by hashing
# filenames' substrings and compare to the stored MD4 hash.
#
# Source: It is a hit file. A search often returns more than one hit.
#
# key_index.dat stores a chunk of Kademlia's Distributed Hash Table
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Generic data holder
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
# @brief Decode data from key_index.dat
# @see CIndexed::CLoadDataThread::Run (srchybrid/kademlia/kademlia/Indexed.cpp)
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_file (f):
  reader = f.new_reader ()
  if not reader:
    return

  decoder = mobius.decoder.data_decoder (reader)

  # decode header
  data = dataholder ()
  data.version = decoder.get_uint32_le ()
  data.republish_time = decoder.get_unix_datetime ()
  data.client_id = decode_kad_id (decoder)
  data.keys = []

  # decode keys
  count = decoder.get_uint32_le ()

  for i in range (count):
    key = decode_key (decoder, data.version)
    data.keys.append (key)

  return data

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode key entry
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_key (decoder, version):
  search = dataholder ()
  search.hash_value = decode_kad_id (decoder)
  search.text = None
  search.sources = []

  count = decoder.get_uint32_le ()
  for i in range (count):
    source = decode_source (decoder, version)
    search.sources.append (source)

  return search

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode source entry
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_source (decoder, version):
  source = dataholder ()
  source.hash_ed2k = decode_kad_id (decoder)		# ED2K HASH !!!!!!
  source.names = []

  count = decoder.get_uint32_le ()
  for i in range (count):
    name = decode_name (decoder, version)
    source.names.append (name)

  return source

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode name entry
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_name (decoder, version):
  name = dataholder ()
  name.lifetime = decoder.get_unix_datetime ()
  name.tags = []
  name.ips = []
  name.filenames = []

  if version >= 4:
    aich_hash_count = decoder.get_uint16_le ()

    for i in range (aich_hash_count):
      h = decoder.get_bytearray_by_size (20)

  if version >= 3:
    name_count = decoder.get_uint32_le ()

    for i in range (name_count):
      filename = decode_string_utf8 (decoder)
      popularity_index = decoder.get_uint32_le ()
      name.filenames.append (filename)

    ip_count = decoder.get_uint32_le ()

    for i in range (ip_count):
      ip = decoder.get_ipv4_le ()
      last_published = decoder.get_unix_datetime ()
      name.ips.append ((ip, last_published))

      if version >= 4:
        aich_hash_idx = decoder.get_uint16_le ()

  tag_count = decoder.get_uint8 ()
  for j in range (tag_count):
    tag = CTag.decode (decoder)
    name.tags.append (tag)

  return name

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode KAD ID
# @param decoder
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_kad_id (decoder):
  c1 = decoder.get_uint32_le ()
  c2 = decoder.get_uint32_le ()
  c3 = decoder.get_uint32_le ()
  c4 = decoder.get_uint32_le ()
  return '%08x%08x%08x%08x' % (c1, c2, c3, c4)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode UTF-8 string
# @param decoder
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_string_utf8 (decoder):
  siz = decoder.get_uint16_le ()
  return decoder.get_string_by_size (siz)
