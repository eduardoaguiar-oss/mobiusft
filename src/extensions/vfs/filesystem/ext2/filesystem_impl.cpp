// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "filesystem_impl.hpp"
#include <mobius/decoder/data_decoder.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if stream contains an instance of ext2/3/4 filesystem
// @param reader Reader object
// @param offset Offset from the beginning of the stream
// @return True/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
filesystem_impl::is_instance (
  mobius::io::reader reader,
  std::uint64_t offset
)
{
  constexpr int MAGIC_OFFSET = 0x0438;

  reader.seek (offset + MAGIC_OFFSET);
  auto data = reader.read (2);

  return data == "\x53\xef";
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param offset Offset from the beginning of volume
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
filesystem_impl::filesystem_impl (
  const mobius::io::reader& reader,
  size_type offset
)
 : reader_ (reader),
   offset_ (offset),
   tsk_adaptor_ (reader, offset)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get metadata item
// @param name Item name
// @return Data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::pod::data
filesystem_impl::get_metadata (const std::string& name) const
{
  _load_data ();
  return metadata_.get (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get root folder
// @return Root folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::folder
filesystem_impl::get_root_folder () const
{
  return tsk_adaptor_.get_root_folder ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load data on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
filesystem_impl::_load_data () const
{
  if (data_loaded_)
    return;

  // create decoder
  mobius::io::reader reader = reader_;
  reader.seek (offset_ + 1024);
  mobius::decoder::data_decoder decoder (reader);

  // decode superblock
  auto inodes_count = decoder.get_uint32_le ();
  auto blocks_count = decoder.get_uint32_le ();
  auto root_blocks_count = decoder.get_uint32_le ();
  auto free_blocks_count = decoder.get_uint32_le ();
  auto free_inodes_count = decoder.get_uint32_le ();
  auto first_data_block = decoder.get_uint32_le ();
  auto block_size = (1 << (10 + decoder.get_uint32_le ()));
  auto cluster_size = (1 << decoder.get_uint32_le ()) * block_size;
  auto blocks_per_group = decoder.get_uint32_le ();
  auto clusters_per_group = decoder.get_uint32_le ();
  auto inodes_per_group = decoder.get_uint32_le ();
  auto last_mount_time = decoder.get_unix_datetime ();
  auto last_write_time = decoder.get_unix_datetime ();
  auto mount_count = decoder.get_uint16_le ();
  auto max_mount_count = decoder.get_int16_le ();
  decoder.skip (2);             // s_magic
  auto state = decoder.get_uint16_le ();
  auto errors = decoder.get_uint16_le ();
  auto minor_revision_level = decoder.get_uint16_le ();
  auto last_check_time = decoder.get_unix_datetime ();
  auto check_interval = decoder.get_uint32_le ();
  auto creator_os = decoder.get_uint32_le ();
  auto revision_level = decoder.get_uint32_le ();
  auto def_resuid = decoder.get_uint16_le ();
  auto def_resgid = decoder.get_uint16_le ();
  auto first_inode = decoder.get_uint32_le ();
  auto inode_size = decoder.get_uint16_le ();
  auto block_group_number = decoder.get_uint16_le ();
  auto feature_compat = decoder.get_uint32_le ();
  auto feature_incompat = decoder.get_uint32_le ();
  auto feature_read_only = decoder.get_uint32_le ();
  auto volume_uuid = decoder.get_uuid ();
  auto volume_name = decoder.get_string_by_size (16);
  auto last_mount_point = decoder.get_string_by_size (64);
  decoder.skip (8);             // s_algorithm_usage_bitmap...s_reserved_gdt_blocks
  auto journal_uuid = decoder.get_uuid ();
  auto journal_inode = decoder.get_uint32_le ();
  auto journal_device = decoder.get_uint32_le ();
  decoder.skip (20);            // s_last_orphan...s_hash_seed[4]
  auto default_hash_version = decoder.get_uint8 ();
  decoder.skip (3);             // s_jnl_backup_type, s_desc_size
  auto default_mount_options = decoder.get_uint32_le ();
  decoder.skip (4);             // s_first_meta_bg
  auto creation_time = decoder.get_unix_datetime ();
  decoder.skip (17*4);          // s_jnl_blocks[17]
  blocks_count |= std::uint64_t (decoder.get_uint32_le ());
  root_blocks_count |= std::uint64_t (decoder.get_uint32_le ());
  free_blocks_count |= std::uint64_t (decoder.get_uint32_le ());
  decoder.skip (28);            // s_min_extra_isize...s_reserved_pad
  auto kib_written = decoder.get_uint64_le ();
  decoder.skip (20);            // s_snapshot_inum...s_snapshot_list
  auto error_count = decoder.get_uint32_le ();
  auto first_error_time = decoder.get_unix_datetime ();
  decoder.skip (48);            // s_first_error_ino...s_first_error_line
  auto last_error_time = decoder.get_unix_datetime ();
  decoder.skip (48);            // s_last_error_ino...s_last_error_func[32]
  auto mount_options = decoder.get_string_by_size (64);
  decoder.skip (20);            // s_usr_quota_inum...s_backup_bgs[2]
  auto encryption_algorithms = decoder.get_bytearray_by_size (4);
  decoder.skip (420);           // s_encrypt_pw_salt[16]..s_reserved[98]
  auto checksum = decoder.get_uint32_le ();

  // set metadata
  metadata_.set ("inodes_count", inodes_count);
  metadata_.set ("blocks_count", blocks_count);
  metadata_.set ("root_blocks_count", root_blocks_count);
  metadata_.set ("free_blocks_count", free_blocks_count);
  metadata_.set ("free_inodes_count", free_inodes_count);
  metadata_.set ("first_data_block", first_data_block);
  metadata_.set ("block_size", block_size);
  metadata_.set ("cluster_size", cluster_size);
  metadata_.set ("blocks_per_group", blocks_per_group);
  metadata_.set ("clusters_per_group", clusters_per_group);
  metadata_.set ("inodes_per_group", inodes_per_group);
  metadata_.set ("last_mount_time", last_mount_time);
  metadata_.set ("last_write_time", last_write_time);
  metadata_.set ("mount_count", mount_count);
  metadata_.set ("max_mount_count", max_mount_count);
  metadata_.set ("state", state);
  metadata_.set ("errors", errors);
  metadata_.set ("minor_revision_level", minor_revision_level);
  metadata_.set ("revision_level", revision_level);
  metadata_.set ("last_check_time", last_check_time);
  metadata_.set ("check_interval", check_interval);
  metadata_.set ("creator_os", creator_os);
  metadata_.set ("def_resuid", def_resuid);
  metadata_.set ("def_resgid", def_resgid);
  metadata_.set ("first_inode", first_inode);
  metadata_.set ("inode_size", inode_size);
  metadata_.set ("block_group_number", block_group_number);
  metadata_.set ("feature_compat", feature_compat);
  metadata_.set ("feature_incompat", feature_incompat);
  metadata_.set ("feature_read_only", feature_read_only);
  metadata_.set ("volume_uuid", volume_uuid);
  metadata_.set ("volume_name", volume_name);
  metadata_.set ("last_mount_point", last_mount_point);
  metadata_.set ("journal_uuid", journal_uuid);
  metadata_.set ("journal_inode", journal_inode);
  metadata_.set ("journal_device", journal_device);
  metadata_.set ("default_hash_version", default_hash_version);
  metadata_.set ("default_mount_options", default_mount_options);
  metadata_.set ("creation_time", creation_time);
  metadata_.set ("kib_written", kib_written);
  metadata_.set ("error_count", error_count);
  metadata_.set ("first_error_time", first_error_time);
  metadata_.set ("last_error_time", last_error_time);
  metadata_.set ("mount_options", mount_options);
  metadata_.set ("encryption_algorithms", encryption_algorithms);
  metadata_.set ("checksum", checksum);

  // derived information
  size_ = blocks_count * block_size;

  // type
  constexpr int EXT4_FEATURE_COMPAT_HAS_JOURNAL = 0x0004;
  constexpr int EXT4_FEATURE_INCOMPAT_EXTENTS = 0x0040;

  if (feature_incompat & EXT4_FEATURE_INCOMPAT_EXTENTS)
    type_ = "ext4";

  else if (feature_compat & EXT4_FEATURE_COMPAT_HAS_JOURNAL)
    type_ = "ext3";

  else
    type_ = "ext2";

  // name
  name_ = type_ + " (";

  if (!volume_name.empty ())
    name_ += volume_name;
  else
    name_ += "UUID: " + volume_uuid;
  name_ += ')';

  // Set data loaded
  data_loaded_ = true;
}


