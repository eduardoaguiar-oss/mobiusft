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
#include "lnk.h"
#include <mobius/core/log.h>
#include <mobius/decoder/data_decoder.h>
#include <mobius/exception.inc>
#include <mobius/string_functions.h>
#include <stdexcept>

namespace mobius::decoder
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief lnk implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class lnk::impl
{
public:

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl (const impl&) = delete;
  impl (impl&&) = delete;
  explicit impl (const mobius::io::reader&);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl& operator= (const impl&) = delete;
  impl& operator= (impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get header size
  // @return Header size
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_header_size () const
  {
    _load_data ();
    return header_size_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get clsid
  // @return Clsid
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_clsid () const
  {
    _load_data ();
    return clsid_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get flags
  // @return Flags
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_flags () const
  {
    _load_data ();
    return flags_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get file attributes
  // @return File attributes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_file_attributes () const
  {
    _load_data ();
    return file_attributes_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get creation time
  // @return Creation time
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::datetime::datetime
  get_creation_time () const
  {
    _load_data ();
    return creation_time_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get access time
  // @return Access time
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::datetime::datetime
  get_access_time () const
  {
    _load_data ();
    return access_time_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get write time
  // @return Write time
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::datetime::datetime
  get_write_time () const
  {
    _load_data ();
    return write_time_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get file size
  // @return File size
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_file_size () const
  {
    _load_data ();
    return file_size_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get icon index
  // @return Icon index
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_icon_index () const
  {
    _load_data ();
    return icon_index_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get show command
  // @return Show command
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_show_command () const
  {
    _load_data ();
    return show_command_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get hotkeys
  // @return Hotkeys
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint16_t
  get_hotkeys () const
  {
    _load_data ();
    return hotkeys_;
  }

 // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get volume label
  // @return Volume label
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_volume_label () const
  {
    _load_data ();
    return volume_label_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get drive type
  // @return Drive type
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_drive_type () const
  {
    _load_data ();
    return drive_type_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get drive serial number
  // @return Drive serial number
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_drive_serial_number () const
  {
    _load_data ();
    return drive_serial_number_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get local base path
  // @return Local base path
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_local_base_path () const
  {
    _load_data ();
    return local_base_path_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get common path suffix
  // @return Common path suffix
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_common_path_suffix () const
  {
    _load_data ();
    return common_path_suffix_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get net name
  // @return Net name
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_net_name () const
  {
    _load_data ();
    return net_name_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get device name
  // @return Device name
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_device_name () const
  {
    _load_data ();
    return device_name_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get network provider type
  // @return Network provider type
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_network_provider_type () const
  {
    _load_data ();
    return network_provider_type_;
  }
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get name
  // @return Name
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_name () const
  {
    _load_data ();
    return name_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get relative path
  // @return Relative path
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_relative_path () const
  {
    _load_data ();
    return relative_path_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get working dir
  // @return Working dir
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_working_dir () const
  {
    _load_data ();
    return working_dir_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get arguments
  // @return Arguments
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_arguments () const
  {
    _load_data ();
    return arguments_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get icon location
  // @return Icon location
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_icon_location () const
  {
    _load_data ();
    return icon_location_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get NetBIOS name
  // @return NetBIOS name
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_netbios_name () const
  {
    _load_data ();
    return netbios_name_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get header section offset
  // @return Header offset
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint64_t
  get_header_offset () const
  {
    _load_data ();
    return header_offset_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get link target id list section offset
  // @return Link target id list offset
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint64_t
  get_link_target_id_list_offset () const
  {
    _load_data ();
    return link_target_id_list_offset_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get link info section offset
  // @return Link info offset
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint64_t
  get_link_info_offset () const
  {
    _load_data ();
    return link_info_offset_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get string data section offset
  // @return String data offset
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint64_t
  get_string_data_offset () const
  {
    _load_data ();
    return string_data_offset_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get extra data section offset
  // @return Extra data offset
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint64_t
  get_extra_data_offset () const
  {
    _load_data ();
    return extra_data_offset_;
  }

private:
  // @brief Reader object
  mobius::io::reader reader_;

  // @brief Header size in bytes
  mutable std::uint32_t header_size_ = 0;

  // @brief CLSID
  mutable std::string clsid_;

  // @brief Flags
  mutable std::uint32_t flags_ = 0;

  // @brief Link target attributes
  mutable std::uint32_t file_attributes_ = 0;

  // @brief Link target creation time
  mutable mobius::datetime::datetime creation_time_;

  // @brief Link target access time
  mutable mobius::datetime::datetime access_time_;

  // @brief Link target write time
  mutable mobius::datetime::datetime write_time_;

  // @brief Link target file size (32 bits)
  mutable std::uint32_t file_size_ = 0;

  // @brief Icon index within icon location
  mutable std::uint32_t icon_index_ = 0;

  // @brief Show command
  mutable std::uint32_t show_command_ = 0;

  // @brief Hot keys
  mutable std::uint16_t hotkeys_ = 0;

  // @brief Volume label
  mutable std::string volume_label_;

  // @brief Drive type
  mutable std::uint32_t drive_type_ = 0;

  // @brief Drive serial number
  mutable std::uint32_t drive_serial_number_ = 0;

  // @brief Local base path
  mutable std::string local_base_path_;

  // @brief Common path suffix
  mutable std::string common_path_suffix_;

  // @brief Net name
  mutable std::string net_name_;

  // @brief Device name
  mutable std::string device_name_;

  // @brief Network provider type
  mutable std::uint32_t network_provider_type_ = 0;

  // @brief Name
  mutable std::string name_;

  // @brief Relative path
  mutable std::string relative_path_;

  // @brief Working directory
  mutable std::string working_dir_;

  // @brief Command line arguments
  mutable std::string arguments_;

  // @brief Icon location
  mutable std::string icon_location_;

  // @brief NetBIOS name
  mutable std::string netbios_name_;

  // @brief Header section offset
  mutable mobius::io::reader::offset_type header_offset_ = 0;

  // @brief LinkTargetIDList section offset
  mutable mobius::io::reader::offset_type link_target_id_list_offset_ = 0;

  // @brief LinkInfo section offset
  mutable mobius::io::reader::offset_type link_info_offset_ = 0;

  // @brief StringData section offset
  mutable mobius::io::reader::offset_type string_data_offset_ = 0;

  // @brief ExtraData section offset
  mutable mobius::io::reader::offset_type extra_data_offset_ = 0;

  // @brief Data loaded flag
  mutable bool data_loaded_ = false;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Helper functions
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void _load_data () const;
  void _load_header (mobius::decoder::data_decoder&) const;
  void _load_link_target_id_list (mobius::decoder::data_decoder&) const;
  void _load_link_info (mobius::decoder::data_decoder&) const;
  void _load_volume_id (mobius::decoder::data_decoder&) const;
  void _load_common_network_relative_link (mobius::decoder::data_decoder&) const;
  void _load_string_data (mobius::decoder::data_decoder&) const;
  void _load_extra_data (mobius::decoder::data_decoder&) const;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
lnk::impl::impl (const mobius::io::reader& reader)
 : reader_ (reader)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load data on demand
// @see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-shllink/16cb4ca1-9339-4d0c-a68d-bf1d6cc0f943 [MS-SHLLINK]
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
lnk::impl::_load_data () const
{
  if (data_loaded_)
    return;

  // Load file sections according to flags
  mobius::decoder::data_decoder decoder (reader_);

  _load_header (decoder);
  _load_link_target_id_list (decoder);
  _load_link_info (decoder);
  _load_string_data (decoder);
  _load_extra_data (decoder);

  // Set header loaded
  data_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode header structure
// @see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-shllink/16cb4ca1-9339-4d0c-a68d-bf1d6cc0f943 (section 2.1)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
lnk::impl::_load_header (mobius::decoder::data_decoder& decoder) const
{
  header_offset_ = decoder.tell ();
  header_size_ = decoder.get_uint32_le ();

  if (header_size_ != 0x0000004c)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid header size"));

  // Clsid (section 2.1)
  clsid_ = decoder.get_guid ();
  if (clsid_ != "00021401-0000-0000-C000-000000000046")
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid CLSID"));

  // Decode header
  flags_ = decoder.get_uint32_le ();
  file_attributes_ = decoder.get_uint32_le ();
  creation_time_ = decoder.get_nt_datetime ();
  access_time_ = decoder.get_nt_datetime ();
  write_time_ = decoder.get_nt_datetime ();
  file_size_ = decoder.get_uint32_le ();
  icon_index_ = decoder.get_uint32_le ();
  show_command_ = decoder.get_uint32_le ();
  hotkeys_ = decoder.get_uint16_le ();
  decoder.skip (10); // reserved1, 2 and 3
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode LinkTargetIDList structure
// @see [MS-SHLLINK] (section 2.2)
// @see https://helgeklein.com/blog/dissecting-a-shortcut/
//! \todo Decode items
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
lnk::impl::_load_link_target_id_list (mobius::decoder::data_decoder& decoder) const
{
  // Check if file has LinkTargetIDList section
  if (!(flags_ & 0x0001))
    return;

  link_target_id_list_offset_ = decoder.tell ();
  auto size = decoder.get_uint16_le () + 2;

  // Decode IDList structure (section 2.2.1)
  auto item_id_size = decoder.get_uint16_le ();

  while (item_id_size > 0)
    {
      mobius::bytearray data = decoder.get_bytearray_by_size (item_id_size - 2);
      item_id_size = decoder.get_uint16_le ();
    }

  // Set position at the end of section
  decoder.seek (link_target_id_list_offset_ + size);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode LinkInfo structure
// @see [MS-SHLLINK] (section 2.3)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
lnk::impl::_load_link_info (mobius::decoder::data_decoder& decoder) const
{
  // Check if file has LinkInfo section
  if (!(flags_ & 0x0002))
    return;

  // LinkInfo header info
  link_info_offset_ = decoder.tell ();
  auto size = decoder.get_uint32_le ();
  auto header_size = decoder.get_uint32_le ();
  auto flags = decoder.get_uint32_le ();
  auto volume_id_offset = decoder.get_uint32_le ();
  auto local_base_path_offset = decoder.get_uint32_le ();
  auto common_network_relative_link_offset = decoder.get_uint32_le ();
  auto common_path_suffix_offset = decoder.get_uint32_le ();
  std::string encoding = "CP1252";

  if (header_size >= 0x0024)
    {
      encoding = "UTF-16LE";
      local_base_path_offset = decoder.get_uint32_le ();
      common_path_suffix_offset = decoder.get_uint32_le ();
    }

  // VolumeID and LocalBasePath
  if (flags & 0x0001)
    {
      decoder.seek (link_info_offset_ + volume_id_offset);
      _load_volume_id (decoder);
      decoder.seek (link_info_offset_ + local_base_path_offset);
      local_base_path_ = decoder.get_c_string (encoding);
    }

  // CommonNetworkRelativeLink and CommonPathSuffix
  if (flags & 0x0002)
    {
      decoder.seek (link_info_offset_ + common_network_relative_link_offset);
      _load_common_network_relative_link (decoder);
      decoder.seek (link_info_offset_ + common_path_suffix_offset);
      common_path_suffix_ = decoder.get_c_string (encoding);
    }

  // Set position at the end of section
  decoder.seek (link_info_offset_ + size);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode VolumeID structure
// @see [MS-SHLLINK] (section 2.3.1)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
lnk::impl::_load_volume_id (mobius::decoder::data_decoder& decoder) const
{
  auto offset = decoder.tell ();

  // Read data
  auto size = decoder.get_uint32_le ();
  drive_type_ = decoder.get_uint32_le ();
  drive_serial_number_ = decoder.get_uint32_le ();
  auto volume_label_offset = decoder.get_uint32_le ();

  // Define volume label offset and encoding
  std::string encoding = "CP1252";

  if (volume_label_offset == 0x00000014)
    {
      volume_label_offset = decoder.get_uint32_le ();
      encoding = "UTF-16LE";
    }

  // Read volume label
  decoder.seek (offset + volume_label_offset);
  volume_label_ = decoder.get_c_string (encoding);

  // Set position at the end of section
  decoder.seek (offset + size);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode CommonNetworkRelativeLink structure
// @see [MS-SHLLINK] (section 2.3.2)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
lnk::impl::_load_common_network_relative_link (mobius::decoder::data_decoder& decoder) const
{
  auto offset = decoder.tell ();

  // Read data
  auto size = decoder.get_uint32_le ();
  auto flags = decoder.get_uint32_le ();
  auto net_name_offset = decoder.get_uint32_le ();
  auto device_name_offset = decoder.get_uint32_le ();
  network_provider_type_ = decoder.get_uint32_le ();
  std::string encoding = "CP1252";

  // Define net and device names offsets
  if (net_name_offset > 0x00000014)
    {
      encoding = "UTF-16LE";
      net_name_offset = decoder.get_uint32_le ();
      device_name_offset = decoder.get_uint32_le ();
    }

  // Read net name
  decoder.seek (offset + net_name_offset);
  net_name_ = decoder.get_c_string (encoding);

  // Read device name
  if (flags & 0x00000001)
    {
      decoder.seek (offset + device_name_offset);
      device_name_ = decoder.get_c_string (encoding);
    }

  // Set position at the end of section
  decoder.seek (offset + size);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode string data section
// @see [MS-SHLLINK] (section 2.4)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
lnk::impl::_load_string_data (mobius::decoder::data_decoder& decoder) const
{
  string_data_offset_ = decoder.tell ();
  std::string encoding = (flags_ & 0x00000080) ? "UTF-16LE" : "CP1252";
  std::uint32_t dsize = (flags_ & 0x00000080) ? 2 : 1;

  // Name_string
  if (flags_ & 0x00000004)
    {
      auto size = decoder.get_uint16_le () * dsize;
      name_ = decoder.get_string_by_size (size, encoding);
    }

  // Relative path
  if (flags_ & 0x00000008)
    {
      auto size = decoder.get_uint16_le () * dsize;
      relative_path_ = decoder.get_string_by_size (size, encoding);
    }

  // Working directory
  if (flags_ & 0x00000010)
    {
      auto size = decoder.get_uint16_le () * dsize;
      working_dir_ = decoder.get_string_by_size (size, encoding);
    }

  // Command-line arguments
  if (flags_ & 0x00000020)
    {
      auto size = decoder.get_uint16_le () * dsize;
      arguments_ = decoder.get_string_by_size (size, encoding);
    }

  // Icon location
  if (flags_ & 0x00000040)
    {
      auto size = decoder.get_uint16_le () * dsize;
      icon_location_ = decoder.get_string_by_size (size, encoding);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode extra data section
// @see [MS-SHLLINK] (section 2.5)
//! \todo Other signatures
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
lnk::impl::_load_extra_data (mobius::decoder::data_decoder& decoder) const
{
  extra_data_offset_ = decoder.tell ();
  auto size = decoder.get_uint32_le ();

  while (size > 3)
    {
      auto signature = decoder.get_uint32_le ();

      if (signature == 0xa0000003)		// section 2.5.10
        {
          decoder.skip (8);	// length, version
          netbios_name_ = decoder.get_string_by_size (16);
          decoder.skip (64);	// Droid, DroidBirth
        }

      else
        {
          mobius::core::log log (__FILE__, __FUNCTION__);
          log.development (__LINE__, "unhandled extra data section (0x" + mobius::string::to_hex (signature, 8) + ")");
          decoder.skip (size - 8);
        }

      size = decoder.get_uint32_le ();
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
lnk::lnk (const mobius::io::reader& reader)
  : impl_ (std::make_shared <impl> (reader))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if link target is readonly
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
lnk::is_target_readonly () const
{
  return get_file_attributes () & 0x0001;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if link target is hidden
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
lnk::is_target_hidden () const
{
  return get_file_attributes () & 0x0002;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if link target is system file
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
lnk::is_target_system () const
{
  return get_file_attributes () & 0x0004;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if link target is directory
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
lnk::is_target_directory () const
{
  return get_file_attributes () & 0x0010;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if link target is archive
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
lnk::is_target_archive () const
{
  return get_file_attributes () & 0x0020;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if link target is temporary
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
lnk::is_target_temporary () const
{
  return get_file_attributes () & 0x0100;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if link target is sparse file
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
lnk::is_target_sparse () const
{
  return get_file_attributes () & 0x0200;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if link target is reparse point
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
lnk::is_target_reparse_point () const
{
  return get_file_attributes () & 0x0400;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if link target is compressed
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
lnk::is_target_compressed () const
{
  return get_file_attributes () & 0x0800;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if link target is offline
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
lnk::is_target_offline () const
{
  return get_file_attributes () & 0x1000;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if link target is content indexed
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
lnk::is_target_content_indexed () const
{
  return ! (get_file_attributes () & 0x2000);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if link target is encrypted
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
lnk::is_target_encrypted () const
{
  return get_file_attributes () & 0x4000;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Target run in separate process if set
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
lnk::is_run_in_separate_process () const
{
  return get_flags () & 0x00000400;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Target run as a different user if set
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
lnk::is_run_as_user () const
{
  return get_flags () & 0x00002000;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get header size
// @return Header size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
lnk::get_header_size () const
{
  return impl_->get_header_size ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get clsid
// @return Clsid
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
lnk::get_clsid () const
{
  return impl_->get_clsid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get flags
// @return Flags
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
lnk::get_flags () const
{
  return impl_->get_flags ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file attributes
// @return File attributes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
lnk::get_file_attributes () const
{
  return impl_->get_file_attributes ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get creation time
// @return Creation time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
lnk::get_creation_time () const
{
  return impl_->get_creation_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get access time
// @return Access time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
lnk::get_access_time () const
{
  return impl_->get_access_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get write time
// @return Write time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
lnk::get_write_time () const
{
  return impl_->get_write_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file size
// @return File size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
lnk::get_file_size () const
{
  return impl_->get_file_size ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get icon index
// @return Icon index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
lnk::get_icon_index () const
{
  return impl_->get_icon_index ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get show command
// @return Show command
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
lnk::get_show_command () const
{
  return impl_->get_show_command ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hotkeys
// @return Hotkeys
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint16_t
lnk::get_hotkeys () const
{
  return impl_->get_hotkeys ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get volume label
// @return Volume label
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
lnk::get_volume_label () const
{
  return impl_->get_volume_label ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get drive type
// @return Drive type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
lnk::get_drive_type () const
{
  return impl_->get_drive_type ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get drive serial number
// @return Drive serial number
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
lnk::get_drive_serial_number () const
{
  return impl_->get_drive_serial_number ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get local base path
// @return Local base path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
lnk::get_local_base_path () const
{
  return impl_->get_local_base_path ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get common path suffix
// @return Common path suffix
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
lnk::get_common_path_suffix () const
{
  return impl_->get_common_path_suffix ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get net name
// @return Net name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
lnk::get_net_name () const
{
  return impl_->get_net_name ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get device name
// @return Device name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
lnk::get_device_name () const
{
  return impl_->get_device_name ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get network provider type
// @return Network provider type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
lnk::get_network_provider_type () const
{
  return impl_->get_network_provider_type ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get name
// @return Name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
lnk::get_name () const
{
  return impl_->get_name ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get relative path
// @return Relative path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
lnk::get_relative_path () const
{
  return impl_->get_relative_path ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get working dir
// @return Working dir
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
lnk::get_working_dir () const
{
  return impl_->get_working_dir ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get arguments
// @return Arguments
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
lnk::get_arguments () const
{
  return impl_->get_arguments ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get icon location
// @return Icon location
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
lnk::get_icon_location () const
{
  return impl_->get_icon_location ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get NetBIOS name
// @return NetBIOS name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
lnk::get_netbios_name () const
{
  return impl_->get_netbios_name ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get header offset
// @return Header offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
lnk::get_header_offset () const
{
  return impl_->get_header_offset ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get link target id list offset
// @return Link target id list offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
lnk::get_link_target_id_list_offset () const
{
  return impl_->get_link_target_id_list_offset ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get link info section offset
// @return Link info offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
lnk::get_link_info_offset () const
{
  return impl_->get_link_info_offset ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get string data section offset
// @return String data offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
lnk::get_string_data_offset () const
{
  return impl_->get_string_data_offset ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get extra data section offset
// @return Extra data offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
lnk::get_extra_data_offset () const
{
  return impl_->get_extra_data_offset ();
}

} // namespace mobius::decoder


