# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008-2026 Eduardo Aguiar
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

import mobius

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants (MSIE 4-9)
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
BLOCK_SIZE = 128


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Index.dat record
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class record(object):
    pass


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief MSIE 4-9 index.dat decoder
# @author Eduardo Aguiar
# @see MSIE Cache File (index.dat) format specification v0.0.16. By Joachim Metz
# @see http://www.stevebunting.org/udpd4n6/forensics/index_dat2.htm
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class decoder(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, reader):
        self.is_valid = False

        # check reader
        self.__reader = reader

        if not reader:
            return

        # check signature
        decoder = mobius.core.decoder.data_decoder(reader)
        signature = decoder.get_string_by_size(24)

        if signature != 'Client UrlCache MMF Ver ':
            return

        # decode other structures
        self.__decode_header(decoder)
        self.__decode_directory_table(decoder)

        # set attributes
        self.is_valid = True
        self.__records = []
        self.__record_loaded = False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Iterate through records
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_records(self):
        if not self.__record_loaded:
            self.__decode_hash_table()

        return iter(self.__records)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Decode header
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __decode_header(self, decoder):
        self.version = decoder.get_string_by_size(4)
        self.size = decoder.get_uint32_le()
        self.hash_table_offset = decoder.get_uint32_le()
        self.total_blocks = decoder.get_uint32_le()
        self.allocated_blocks = decoder.get_uint32_le()
        self.cache_size_limit = decoder.get_uint64_le()
        self.cache_size = decoder.get_uint64_le()
        self.cache_size_non_releasable = decoder.get_uint64_le()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Decode header
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __decode_directory_table(self, decoder):
        self.__directories = []
        count = decoder.get_uint32_le()

        for i in range(count):
            file_count = decoder.get_uint32_le()
            dirname = decoder.get_string_by_size(8)
            self.__directories.append(dirname)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Decode hash table
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __decode_hash_table(self):
        if self.__record_loaded:
            return

        # decode hash table blocks
        self.__records = []

        try:
            offset = self.hash_table_offset
            decoder = mobius.core.decoder.data_decoder(self.__reader)

            while offset != 0:
                decoder.seek(offset)
                signature = decoder.get_string_by_size(4)

                if signature == 'HASH':
                    offset = self.__decode_hash_block(decoder)

                else:
                    offset = 0  # error
        except Exception as e:
            mobius.core.logf('Error: ' + str(e))

        # set records loaded
        self.__record_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Decode hash block
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __decode_hash_block(self, decoder):
        blocks = decoder.get_uint32_le()
        offset = decoder.get_uint32_le()
        seq_number = decoder.get_uint32_le()

        # decoder hash block data
        rec_data = []
        rec_offsets = set()
        count = (blocks * BLOCK_SIZE - 16) // 8

        for i in range(count):
            rec_hash = decoder.get_uint32_le()
            rec_offset = decoder.get_uint32_le()

            if rec_offset not in rec_offsets and rec_hash != rec_offset and rec_offset != 0 and rec_hash not in (
                    0x0badf00d, 0xdeadbeef):
                rec_data.append((rec_hash, rec_offset))
                rec_offsets.add(rec_offset)

        # decode records
        for rec_hash, rec_offset in rec_data:
            r = self.__decode_hash_record(decoder, rec_offset)
            r.hash_value = rec_hash
            r.is_active = rec_hash not in (1, 3)

            self.__records.append(r)

        return offset

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Decode hash record
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __decode_hash_record(self, decoder, offset):
        decoder.seek(offset)

        r = record()
        r.offset = offset
        r.type = decoder.get_string_by_size(4).rstrip()
        r.size = decoder.get_uint32_le() * BLOCK_SIZE - 8

        if r.type == 'URL':
            self.__decode_url(decoder, r)

            # debug
            if offset == 0x21f80:
                decoder.seek(offset)
                data = decoder.get_bytearray_by_size(r.size)

        elif r.type == 'REDR':
            self.__decode_redr(decoder, r)

        return r

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Decode URL record
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __decode_url(self, decoder, r):
        r.secondary_time = decoder.get_nt_datetime()
        r.primary_time = decoder.get_nt_datetime()
        r.location = None
        r.filename = None
        r.tags = None

        if self.version == '4.7':
            r.expiration_time = decoder.get_nt_datetime()
            r.cached_file_size = decoder.get_uint32_le()
            r.release_seconds = 0
            decoder.skip(20)
            d1_offset = 0

        else:
            r.expiration_time = decoder.get_fat_datetime()
            decoder.skip(4)
            r.cached_file_size = decoder.get_uint64_le()
            group_offset = decoder.get_uint32_le()
            r.release_seconds = decoder.get_uint32_le()
            d1_offset = decoder.get_uint32_le()

        location_offset = decoder.get_uint32_le()
        cache_dir_index = decoder.get_uint16_le()
        d2 = decoder.get_uint16_le()
        filename_offset = decoder.get_uint32_le()
        r.flags = decoder.get_uint32_le()
        tags_offset = decoder.get_uint32_le()
        tags_size = decoder.get_uint32_le()
        decoder.skip(4)
        r.last_sync_time = decoder.get_fat_datetime()
        r.hits = decoder.get_uint32_le()

        # get d1 (maybe left node and right node hashes from a (flat) binary tree?)
        if d1_offset:
            decoder.seek(r.offset + d1_offset)
            d1a = decoder.get_uint32_le()
            d1b = decoder.get_uint32_le()

            if d1a not in (0, 0x0badf00d, 0xdeadbeef):
                mobius.core.logf('decoder_index_dat: Dummy A value = %08x' % d1a)

            if d1b not in (0, 0x0badf00d, 0xdeadbeef):
                mobius.core.logf('decoder_index_dat: Dummy B value = %08x' % d1b)

        # get location
        if location_offset:
            decoder.seek(r.offset + location_offset)
            r.location = decoder.get_c_string()
            r.location = r.location[:r.size - location_offset]
            r.location = r.location.rstrip('\x0b\xad\xf0\x0d\x00\xde\xbe\xef')
            r.location = r.location.decode('cp1252')

        # get filename
        if filename_offset:
            decoder.seek(r.offset + filename_offset)
            filename = decoder.get_c_string()
            filename = filename[:r.size - filename_offset]
            filename = filename.rstrip('\x0b\xad\xf0\x0d\x00')

            if cache_dir_index == 0xfe:
                r.filename = filename
            else:
                dirname = self.__directories[cache_dir_index]
                r.filename = os.path.join(dirname, filename)

        # get http headers
        if tags_offset:
            r.tags = self.__decode_tags(decoder, r.offset + tags_offset, tags_size)
        else:
            r.tags = {}

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Decode tags
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __decode_tags(self, decoder, offset, size):
        tags = {}

        decoder.seek(offset)
        end_pos = decoder.tell() + size
        tag_size = 4
        tag_type = -1

        while decoder.tell() < end_pos and tag_type != 0:
            offset = decoder.tell()
            tag_size = decoder.get_uint16_le()
            tag_type = int(decoder.get_uint8())
            value_type = decoder.get_uint8()
            value = None

            if value_type in (0x01, 0x1f):
                value = decoder.get_string_by_size(tag_size - 8, 'utf-16')
                v1 = decoder.get_uint32_le()

            elif value_type == 0x03:  # dword
                value = decoder.get_uint32_le()
                v1 = decoder.get_uint32_le()

            elif value_type == 0x40:  # datetime
                value = decoder.get_nt_datetime()
                v1 = decoder.get_uint32_le()

            elif value_type == 0x42:  # strings array
                value = decoder.get_bytearray_by_size(tag_size - 8).rstrip('\x00').rsplit('\x00')
                v1 = decoder.get_uint32_le()

            elif tag_size >= 4:
                value_data = decoder.get_bytearray_by_size(tag_size - 4)

                import pymobius
                value = pymobius.dump(value_data)

            tags[tag_type] = value

        return tags

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Decode REDR record
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __decode_redr(self, decoder, r):
        dummy1 = decoder.get_uint32_le()
        dummy2 = decoder.get_uint32_le()

        r.location = decoder.get_c_string()
