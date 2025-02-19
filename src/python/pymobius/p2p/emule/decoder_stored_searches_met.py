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
MET_HEADER = 0x0e
MET_HEADER_I64TAGS = 0x0f
MET_LAST_VERSION = 1

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
# @brief Decode data from StoredSearches.met
# @see CSearchList::LoadSearches - srchybrid/SearchList.cpp
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_file (f):

  # try to open file
  reader = f.new_reader ()
  if not reader:
    return

  # decode header
  decoder = mobius.decoder.data_decoder (reader)

  header = decoder.get_uint8 ()
  if header != MET_HEADER_I64TAGS:
    return

  version = decoder.get_uint8 ()
  if version > MET_LAST_VERSION:
    mobius.core.logf ("DEV Version %d > %d" % (version, MET_LAST_VERSION))

  # create data structure
  data = dataholder ()
  data.csearches = []

  # decode searches
  count = decoder.get_uint16_le ()

  for i in range (count):
    params = decode_ssearch_params (decoder)
    data.csearches.append (params)

  return data

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief read string from decoder
# @see CFileDataIO::ReadString (bool)
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def read_string (decoder):
  l = decoder.get_uint16_le ()
  if l > 0:
    s = decoder.get_string_by_size (l)
  else:
    s = ''
  return s

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief decode data from SSearchParams structure
# @see SSearchParams::SSearchParams - srchybrid/SearchParams.h
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_ssearch_params (decoder):
  params = dataholder ()
  params.search_id = decoder.get_uint32_le ()
  params.etype = decoder.get_uint8 ()
  params.b_client_search_files = decoder.get_uint8 () > 0
  params.special_title = read_string (decoder)
  params.expression = read_string (decoder)
  params.filetype = read_string (decoder)
  params.files = []

  # decode files
  file_count = decoder.get_uint32_le ()

  for j in range (file_count):
    cfile = decode_csearch_file (decoder)
    params.files.append (cfile)

  return params

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief decode data from CSearchFile structure
# @see CSearchFile::CSearchFile - srchybrid/CSearchFile.cpp
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_csearch_file (decoder):
  f = dataholder ()
  f.hash_ed2k = mobius.encoder.hexstring (decoder.get_bytearray_by_size (16))
  f.user_ip = decoder.get_ipv4 ()
  f.user_port = decoder.get_uint16_le ()
  f.tags = []

  # decode tags
  tagcount = decoder.get_uint32_le ()

  for i in range (tagcount):
    tag = CTag.decode (decoder)
    f.tags.append (tag)

  return f
