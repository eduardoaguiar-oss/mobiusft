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
import pymobius.p2p
from pymobius.p2p.local_file import *
import pymobius.p2p.remote_file
from . import CTag

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PARTFILE_VERSION = 0xe0
PARTFILE_SPLITTEDVERSION = 0xe1
PARTFILE_VERSION_LARGEFILE = 0xe2

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief generic data holder
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class dataholder (object):
  pass

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from .part.met
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve (item, model, f, sources, username):

  # try to open file
  if not f:
    return

  try:
    retrieve_data (item, model, f, sources, username)
  except Exception as e:
    mobius.core.logf ('WRN %s %s' % (str (e), traceback.format_exc ()))

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from .part.met
# @see CPartFile::LoadPartFile - srchybrid/PartFile.cpp
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve_data (item, model, f, sources, username):

  reader = f.new_reader ()
  if not reader:
    return

  # decode header
  decoder = mobius.decoder.data_decoder (reader)
  version = decoder.get_uint8 ()

  if version not in (PARTFILE_VERSION, PARTFILE_SPLITTEDVERSION, PARTFILE_VERSION_LARGEFILE):
    return

  is_new_style = (version == PARTFILE_SPLITTEDVERSION)

  data = dataholder ()
  data.part_met_path = f.path

  # check eDonkey "old part style"
  if not is_new_style:
    decoder.seek (24)
    value = decoder.get_uint32_le ()
    decoder.seek (1)

    if value == 0x01020000:
      is_new_style = True

  # decode data
  if is_new_style:
    temp = decoder.get_uint32_le ()

    if temp == 0:
      data.hash_ed2k = mobius.encoder.hexstring (decoder.get_bytearray_by_size (16))

    else:
      decoder.seek (2)
      data.timestamp = decoder.get_unix_datetime ()
      data.hash_ed2k = decode_md4_hashset (decoder)

  else:
    data.timestamp = decoder.get_unix_datetime ()
    data.hash_ed2k = decode_md4_hashset (decoder)

  # decode tags
  tagcount = decoder.get_uint32_le ()
  data.tags = []

  for i in range (tagcount):
    tag = CTag.decode (decoder)
    data.tags.append (tag)

  # add data to model and set file as handled
  add_data_to_model (model, data, sources, username)
  pymobius.p2p.set_handled (item, f)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief decode MD4 hash set
# @param decoder
#
# For now, ignore hash set and return only ED2K hash
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_md4_hashset (decoder):
  data = dataholder ()
  data.hash_ed2k = mobius.encoder.hexstring (decoder.get_bytearray_by_size (16))
  data.hashes = []

  count = decoder.get_uint16_le ()

  for i in range (count):
    data.hashes.append (mobius.encoder.hexstring (decoder.get_bytearray_by_size (16)))

  return data.hash_ed2k

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief add data to model
# @param data
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def add_data_to_model (model, data, sources, username):

  # get info from CTags
  info = pymobius.p2p.emule.CTag.get_metadata (data.tags)

  # local file
  f = local_file ()
  f.app_id = 'emule'
  f.app = 'Emule'
  f.username = username
  f.path = data.part_met_path[:-4].replace ('/', '\\')
  f.name = info.get ('name')
  f.size = info.get ('size', 0)

  CTag.set_file_attributes (f, data.tags)
  bytes_to_download = info.get ('total_gap_size', 0)
  bytes_downloaded = f.size - bytes_to_download

  # flags
  f.flag_shared = STATE_ALWAYS
  f.flag_corrupted = STATE_YES if info.get ('corrupted_parts') else STATE_NO
  f.flag_downloaded = STATE_YES if bytes_to_download < f.size else STATE_NO
  f.flag_completed = STATE_YES if bytes_to_download == 0 else STATE_NO
  f.flag_uploaded = STATE_YES if info.get ('uploaded_bytes') > 0 or info.get ('times_accepted', 0) > 0 else STATE_NO

  # hashes (ED2K, AICH)
  f.set_hash ('ed2k', data.hash_ed2k)
  if 'hash_aich' in info:
    f.set_hash ('aich', info.get ('hash_aich'))

  # metadata
  if f.size:
    bytes_downloaded = '%d (%.2f%%)' % (bytes_downloaded, float (bytes_downloaded) * 100.0 / float (f.size))
    bytes_to_download = '%d (%.2f%%)' % (bytes_to_download, float (bytes_to_download) * 100.0 / float (f.size))
    f.add_metadata ('Bytes downloaded', bytes_downloaded)
    f.add_metadata ('Bytes to download', bytes_to_download)

  f.add_metadata ('.part.met path', data.part_met_path)

  model.local_files.append (f)

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # remote file
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for source in sources:

    rf = pymobius.p2p.remote_file.remote_file ()
    rf.username = f.username
    rf.timestamp = source.timestamp
    rf.app_id = f.app_id
    rf.app = f.app
    rf.size = f.size
    rf.name = f.name
    rf.path = f.path

    for id, name in f.get_hashes ():
      rf.set_hash (id, name)

    rf.add_metadata ('.part.met.txtsrc path', data.part_met_path + '.txtsrc')

    # peer data
    rf.peer.ip = source.ip
    rf.peer.port = source.port

    model.remote_files.append (rf)
