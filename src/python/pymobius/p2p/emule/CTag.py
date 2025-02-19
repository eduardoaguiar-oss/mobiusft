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
import mobius
import datetime

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tag types
# @see srchybrid/opcodes.h
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
TAGTYPE_HASH = 0x01
TAGTYPE_STRING = 0x02
TAGTYPE_UINT32 = 0x03
TAGTYPE_FLOAT32 = 0x04
TAGTYPE_BOOL = 0x05
TAGTYPE_BOOLARRAY = 0x06
TAGTYPE_BLOB = 0x07
TAGTYPE_UINT16 = 0x08
TAGTYPE_UINT8 = 0x09
TAGTYPE_BSOB = 0x0A
TAGTYPE_UINT64 = 0x0B
TAGTYPE_STR1 = 0x11
TAGTYPE_STR2 = 0x12
TAGTYPE_STR3 = 0x13
TAGTYPE_STR4 = 0x14
TAGTYPE_STR5 = 0x15
TAGTYPE_STR6 = 0x16
TAGTYPE_STR7 = 0x17
TAGTYPE_STR8 = 0x18
TAGTYPE_STR9 = 0x19
TAGTYPE_STR10 = 0x1A
TAGTYPE_STR11 = 0x1B
TAGTYPE_STR12 = 0x1C
TAGTYPE_STR13 = 0x1D
TAGTYPE_STR14 = 0x1E
TAGTYPE_STR15 = 0x1F
TAGTYPE_STR16 = 0x20

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tags with special format (U - UTF-8 string, D - Unix date/time, S - seconds)
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SPECIAL_TAGS = {
  0x01 : 'U',
  0x05 : 'D',
  0x21 : 'D',
  0x23 : 'S',
  0x34 : 'D',
  0x92 : 'D',
  0xD0 : 'U',
  0xD2 : 'U',
  0xD3 : 'S'
}

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief read string from decoder
# @see CFileDataIO::ReadString (bool)
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def read_string (decoder):
  l = decoder.get_uint16_le ()

  if l > 0:
    b = decoder.get_bytearray_by_size (l)
    try:
      s = b.decode ('utf-8')
    except UnicodeDecodeError:
      s = b.decode ('iso-8859-1')

  else:
    s = ''

  return s

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief return datetime from Unix timestamp
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def datetime_from_unix (timestamp):
  if timestamp:
    value = datetime.datetime (1970,1,1,0,0,0) + datetime.timedelta (seconds=int (timestamp))
  else:
    value = None
  return value

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief convert duration from seconds to hh::mm:ss
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_duration_from_seconds (seconds):
  s = seconds % 60
  m = (seconds // 60) % 60
  h = (seconds // 3600)
  return '%02d:%02d:%02d' % (h, m , s)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief generic data holder
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class dataholder (object):
  pass

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Tag object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Tag (object):
  pass

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief decode CTag structure
# @see CTag::CTag - srchybrid/packets.cpp
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode (decoder):
  tag = Tag ()
  tag.id = 0
  tag.name = None
  tag.value = None

  # read tag id and name
  tag.type = decoder.get_uint8 ()

  if tag.type & 0x80:
    tag.type &= 0x7f
    tag.id = decoder.get_uint8 ()

  else:
    length = decoder.get_uint16_le ()

    if length == 1:
      tag.id = decoder.get_uint8 ()

    else:
      tag.id = 0
      tag.name = decoder.get_string_by_size (length)

  # read tag value
  if tag.type == TAGTYPE_STRING:
    tag.value = read_string (decoder)

  elif tag.type == TAGTYPE_UINT32:
    tag.value = decoder.get_uint32_le ()

  elif tag.type == TAGTYPE_UINT64:
    tag.value = decoder.get_uint64_le ()

  elif tag.type == TAGTYPE_UINT16:
    tag.value = decoder.get_uint16_le ()
    tag.type = TAGTYPE_UINT32

  elif tag.type == TAGTYPE_UINT8:
    tag.value = decoder.get_uint8 ()
    tag.type = TAGTYPE_UINT32

  elif tag.type == TAGTYPE_FLOAT32:
    tag.value = decoder.get_bytearray_by_size (4) #.get_float32_le ()

  elif tag.type >= TAGTYPE_STR1 and tag.type <= TAGTYPE_STR16:
    length = tag.type - TAGTYPE_STR1 + 1
    tag.value = decoder.get_string_by_size (length)
    tag.type = TAGTYPE_STRING

  elif tag.type == TAGTYPE_HASH:
    tag.value = mobius.encoder.hexstring (decoder.get_bytearray_by_size (16))

  elif tag.type == TAGTYPE_BOOL:
    decoder.skip (1)

  elif tag.type == TAGTYPE_BOOLARRAY:
    length = decoder.get_uint16_le ()
    decoder.skip (length // 8 + 1)
    # @todo 07-Apr-2004: eMule versions prior to 0.42e.29 used the formula "(len+7)/8"!

  elif tag.type == TAGTYPE_BLOB:
    # @todo 07-Apr-2004: eMule versions prior to 0.42e.29 handled the "len" as int16!
    size = decoder.get_uint32_le ()
    tag.value = decoder.get_bytearray_by_size (size)

  else:
    mobius.core.logf ("DEV Unknown tag: type=0x%02x" % tag.type)

  return tag

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get metadata from tags
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def format_tag_values (tags):
  for tag in tags:
    fmt = SPECIAL_TAGS.get (tag.id)

    if fmt == 'U':
      tag.value = tag.value

    elif fmt == 'D':
      tag.value = datetime_from_unix (tag.value)

    elif fmt == 'S':
      tag.value = get_duration_from_seconds (tag.value)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get metadata from tags
# @see CKnownFile::LoadTagsFromFile - srchybrid/KnownFile.cpp
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_metadata (tags):
  metadata = {}

  uploaded_bytes = 0
  not_counted_uploaded_bytes = 0
  total_gap_size = 0
  gap_start = 0

  for tag in tags:
    if tag.name and ord (tag.name[0]) == 0x09:		# FT_GAPSTART
      gap_start = tag.value

    elif tag.name and ord (tag.name[0]) == 0x0a:	# FT_GAPEND
      end = tag.value
      total_gap_size += (end - gap_start)

    elif tag.id == 0x01:
      metadata['name'] = tag.value

    elif tag.id == 0x02:
      metadata['size'] = tag.value

    elif tag.id == 0x03:
      metadata['filetype'] = tag.value

    elif tag.id == 0x04:
      metadata['fileformat'] = tag.value

    elif tag.id == 0x05:
      metadata['last_seen_time'] = datetime_from_unix (tag.value)

    elif tag.id == 0x06:
      metadata['part_path'] = tag.value

    elif tag.id == 0x07:
      metadata['part_hash'] = tag.value

    elif tag.id == 0x08:
      metadata['bytes_downloaded'] = tag.value

    elif tag.id == 0x0b:
      metadata['description'] = tag.value

    elif tag.id == 0x11:
      metadata['version'] = tag.value

    elif tag.id == 0x12:
      metadata['part_name'] = tag.value

    elif tag.id == 0x13:
      metadata['priority'] = tag.value

    elif tag.id == 0x14:
      metadata['status'] = tag.value

    elif tag.id == 0x15:
      metadata['sources'] = tag.value

    elif tag.id == 0x18:
      metadata['dl_priority'] = tag.value

    elif tag.id == 0x19:
      metadata['ul_priority'] = tag.value

    elif tag.id == 0x1a:
      metadata['compression_gain'] = tag.value

    elif tag.id == 0x1b:
      metadata['corrupted_loss'] = tag.value

    elif tag.id == 0x21:
      metadata['kad_last_published_time'] = datetime_from_unix (tag.value)

    elif tag.id == 0x22:
      metadata['flags'] = tag.value

    elif tag.id == 0x23:
      metadata['download_active_time'] = get_duration_from_seconds (tag.value)

    elif tag.id == 0x24:
      metadata['corrupted_parts'] = [ int (v) for v in tag.value.split (',') ]

    elif tag.id == 0x27:
      metadata['hash_aich'] = tag.value

    elif tag.id == 0x30:
      metadata['complete_sources'] = tag.value

    elif tag.id == 0x34:
      metadata['last_shared_time'] = datetime_from_unix (tag.value)

    elif tag.id == 0x35:
      pass		# AICH hashset

    elif tag.id == 0x50:
      uploaded_bytes = (uploaded_bytes & 0xffffffff00000000) | tag.value

    elif tag.id == 0x51:
      metadata['times_requested'] = tag.value

    elif tag.id == 0x52:
      metadata['times_accepted'] = tag.value

    elif tag.id == 0x53:
      metadata['category'] = tag.value

    elif tag.id == 0x54:
      uploaded_bytes = (uploaded_bytes & 0x00000000ffffffff) | (tag.value << 32)

    elif tag.id == 0x55:
      metadata['max_sources'] = tag.value

    elif tag.id == 0x90:
      not_counted_uploaded_bytes = (not_counted_uploaded_bytes & 0xffffffff00000000) | tag.value

    elif tag.id == 0x91:
      not_counted_uploaded_bytes = (not_counted_uploaded_bytes & 0x00000000ffffffff) | (tag.value << 32)

    elif tag.id == 0x92:
      metadata['last_update_time'] = datetime_from_unix (tag.value)

    elif tag.id == 0xd0:
      metadata['media_artist'] = tag.value

    elif tag.id == 0xd1:
      metadata['media_album'] = tag.value

    elif tag.id == 0xd2:
      metadata['media_title'] = tag.value

    elif tag.id == 0xd3:
      metadata['media_length'] = get_duration_from_seconds (tag.value)

    elif tag.id == 0xd4:
      metadata['media_bitrate'] = tag.value

    elif tag.id == 0xd5:
      metadata['media_codec'] = tag.value

    elif tag.id == 0xf6:
      metadata['file_comment'] = tag.value

    elif tag.id == 0xf7:
      metadata['file_rating'] = tag.value

    else:
      mobius.core.logf ('DEV Unknown tag ID %02x' % tag.id)

      if tag.type in (6, 7, 10):
        value = mobius.encoder.hexstring (tag.value)
      else:
        value = tag.value

  # fill other metadata
  metadata['total_gap_size'] = total_gap_size
  metadata['uploaded_bytes'] = uploaded_bytes
  metadata['not_counted_uploaded_bytes'] = not_counted_uploaded_bytes

  return metadata

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief set file attributes from tags
# @see CKnownFile::LoadTagsFromFile - srchybrid/KnownFile.cpp
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def set_file_attributes (f, tags):

  def add_metadata (f, name, value):
    if value != None:
      f.add_metadata (name, value)

  info = get_metadata (tags)
  f.name = info.get ('name')
  f.size = info.get ('size', -1)

  add_metadata (f, 'File type', info.get ('filetype'))
  add_metadata (f, 'File format', info.get ('fileformat'))
  add_metadata (f, 'Last seen complete date/time (UTC)', info.get ('last_seen_time'))
  add_metadata (f, '.part file path', info.get ('part_path'))
  add_metadata (f, 'Bytes actually downloaded', info.get ('bytes_downloaded'))
  add_metadata (f, 'Description', info.get ('description'))
  add_metadata (f, 'Version', info.get ('version'))
  add_metadata (f, '.part file name', info.get ('part_name'))
  add_metadata (f, 'Sources', info.get ('sources'))
  add_metadata (f, 'Download priority', info.get ('dl_priority'))
  add_metadata (f, 'Upload priority', info.get ('ul_priority'))
  add_metadata (f, 'Compression gain in bytes', info.get ('compression_gain'))
  add_metadata (f, 'Corrupted loss in bytes', info.get ('corrupted_loss'))
  add_metadata (f, 'KAD last published date/time (UTC)', info.get ('kad_last_published_time'))
  add_metadata (f, 'Flags', info.get ('flags'))
  add_metadata (f, 'Download active time', info.get ('download_active_time'))
  add_metadata (f, 'Complete sources', info.get ('complete_sources'))
  add_metadata (f, 'Last shared date/time (UTC)', info.get ('last_shared_time'))
  add_metadata (f, 'Times requested', info.get ('times_requested'))
  add_metadata (f, 'Times accepted', info.get ('times_accepted'))
  add_metadata (f, 'Category', info.get ('category'))
  add_metadata (f, 'Max sources', info.get ('max_sources'))
  add_metadata (f, 'Last data update date/time (UTC)', info.get ('last_update_time'))
  add_metadata (f, 'Media artist', info.get ('media_artist'))
  add_metadata (f, 'Media album', info.get ('media_album'))
  add_metadata (f, 'Media title', info.get ('media_title'))
  add_metadata (f, 'Media length', info.get ('media_length'))
  add_metadata (f, 'Media bitrate', info.get ('media_bitrate'))
  add_metadata (f, 'Media Codec', info.get ('media_codec'))
  add_metadata (f, 'File comment', info.get ('file_comment'))
  add_metadata (f, 'File rating', info.get ('file_rating'))

  # other metadata
  value = info.get ('uploaded_bytes', 0)
  if value > 0:
    f.add_metadata ('Bytes uploaded', value)

  value = info.get ('not_counted_uploaded_bytes', 0)
  if value > 0:
    f.add_metadata ('Bytes uploaded (not counted)', value)
