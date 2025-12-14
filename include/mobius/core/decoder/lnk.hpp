#ifndef MOBIUS_CORE_DECODER_LNK_HPP
#define MOBIUS_CORE_DECODER_LNK_HPP

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008-2026 Eduardo Aguiar
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
#include <mobius/core/io/reader.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace mobius::core::decoder
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief .LNK decoder class
// @author Eduardo Aguiar
// @see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-shllink/16cb4ca1-9339-4d0c-a68d-bf1d6cc0f943
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class lnk
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  explicit lnk (const mobius::core::io::reader&);
  lnk (lnk&&) noexcept = default;
  lnk (const lnk&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  lnk& operator= (const lnk&) noexcept = default;
  lnk& operator= (lnk&&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Function prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bool is_target_readonly () const;
  bool is_target_hidden () const;
  bool is_target_system () const;
  bool is_target_directory () const;
  bool is_target_archive () const;
  bool is_target_temporary () const;
  bool is_target_sparse () const;
  bool is_target_reparse_point () const;
  bool is_target_compressed () const;
  bool is_target_offline () const;
  bool is_target_content_indexed () const;
  bool is_target_encrypted () const;
  bool is_run_in_separate_process () const;
  bool is_run_as_user () const;
  std::uint32_t get_header_size () const;
  std::string get_clsid () const;
  std::uint32_t get_flags () const;
  std::uint32_t get_file_attributes () const;
  mobius::core::datetime::datetime get_creation_time () const;
  mobius::core::datetime::datetime get_access_time () const;
  mobius::core::datetime::datetime get_write_time () const;
  std::uint32_t get_file_size () const;
  std::uint32_t get_icon_index () const;
  std::uint32_t get_show_command () const;
  std::uint16_t get_hotkeys () const;
  std::string get_volume_label () const;
  std::uint32_t get_drive_type () const;
  std::uint32_t get_drive_serial_number () const;
  std::string get_local_base_path () const;
  std::string get_common_path_suffix () const;
  std::string get_net_name () const;
  std::string get_device_name () const;
  std::uint32_t get_network_provider_type () const;
  std::string get_name () const;
  std::string get_relative_path () const;
  std::string get_working_dir () const;
  std::string get_arguments () const;
  std::string get_icon_location () const;
  std::string get_netbios_name () const;
  std::uint64_t get_header_offset () const;
  std::uint64_t get_link_target_id_list_offset () const;
  std::uint64_t get_link_info_offset () const;
  std::uint64_t get_string_data_offset () const;
  std::uint64_t get_extra_data_offset () const;

private:
  // @brief Implementation class forward declaration
  class impl;

  // @brief Implementation pointer
  std::shared_ptr <impl> impl_;
};

} // namespace mobius::core::decoder

#endif


