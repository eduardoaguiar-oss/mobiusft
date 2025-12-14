#ifndef MOBIUS_CORE_OS_WIN_DPAPI_CREDHIST_ENTRY_HPP
#define MOBIUS_CORE_OS_WIN_DPAPI_CREDHIST_ENTRY_HPP

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
#include <mobius/core/bytearray.hpp>
#include <mobius/core/io/reader.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace mobius::core::os::win::dpapi
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief credhist_entry class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class credhist_entry
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  explicit credhist_entry (mobius::core::io::reader, std::uint32_t);
  credhist_entry (credhist_entry&&) noexcept = default;
  credhist_entry (const credhist_entry&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  credhist_entry& operator= (const credhist_entry&) noexcept = default;
  credhist_entry& operator= (credhist_entry&&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Function prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t get_revision () const;
  std::string get_guid () const;
  std::uint32_t get_next_link_size () const;
  std::uint32_t get_type () const;
  std::uint32_t get_hash_id () const;
  std::uint32_t get_iterations () const;
  std::uint32_t get_cipher_id () const;
  mobius::core::bytearray get_salt () const;
  std::string get_sid () const;
  mobius::core::bytearray get_cipher_text () const;
  mobius::core::bytearray get_hash_sha1 () const;
  mobius::core::bytearray get_hash_ntlm () const;

  bool decrypt_with_key (const mobius::core::bytearray&);
  bool decrypt_with_password_hash (const mobius::core::bytearray&);
  bool decrypt_with_password (const std::string&);
  bool is_decrypted () const;

private:
  // @brief Implementation class forward declaration
  class impl;

  // @brief Implementation pointer
  std::shared_ptr <impl> impl_;
};

} // namespace mobius::core::os::win::dpapi

#endif


