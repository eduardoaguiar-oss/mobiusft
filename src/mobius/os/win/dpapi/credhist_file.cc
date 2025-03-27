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
#include "credhist_file.h"
#include <mobius/charset.h>
#include <mobius/crypt/hash.h>
#include <mobius/exception.inc>
#include <stdexcept>
#include <set>

namespace mobius::os::win::dpapi
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Try to decrypt all CREDHIST entries in sequence
// @param entries Reference to entries
// @return true if any entry has been decrypted, false if none
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static bool
decrypt_sequence (std::vector <credhist_entry>& entries)
{
  bool rc = false;

  // get hashes from decrypted entries
  std::set <mobius::bytearray> hashes;

  for (auto entry : entries)
    {
      if (entry.is_decrypted ())
        hashes.insert (entry.get_hash_sha1 ());
    }

  // test those hashes on non-decrypted entries
  while (!hashes.empty ())
    {
      auto iter_first = hashes.begin ();
      auto h = *iter_first;
      hashes.erase (iter_first);

      for (auto entry : entries)
        {
          if (!entry.is_decrypted () && entry.decrypt_with_password_hash (h))
            {
              hashes.insert (entry.get_hash_sha1 ());
              rc = true;
            }
        }
    }

  return rc;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
//
// 1. CREDHIST files contain a single linked list of entries, starting from the
//    end of the file, up to the beginning of it.
// 2. Each entry has an entry footer (24 bytes) and optionally an entry data
//    block.
// 3. entry.size determines how many bytes to jump from current entry end
//    position to the next entry end position.
// 4. if entry.size > 0 then entry has data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
credhist_file::credhist_file (mobius::io::reader reader)
{
  // Read first entry (control block only)
  constexpr int FOOTER_SIZE = 24;

  if (reader.get_size () < FOOTER_SIZE)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("Not enough bytes to read"));

  reader.seek (-FOOTER_SIZE, mobius::io::reader::whence_type::end);
  auto pos = reader.tell ();
  auto entry = credhist_entry (reader, FOOTER_SIZE);
  auto link_size = entry.get_next_link_size ();

  // Iterate through entries
  while (link_size > 0)
    {
      pos = pos - link_size;
      reader.seek (pos);
      entry = credhist_entry (reader, link_size);
      link_size = entry.get_next_link_size ();
      entries_.push_back (entry);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt CREDHIST entries
// @param key Decryption key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
credhist_file::decrypt_with_key (const mobius::bytearray& key)
{
  bool rc = false;

  if (entries_.size ())
    {
      rc = entries_[0].decrypt_with_key (key);
      rc |= decrypt_sequence (entries_);
    }

  return rc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt CREDHIST entries with password hash
// @param h Password hash
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
credhist_file::decrypt_with_password_hash (const mobius::bytearray& h)
{
  bool rc = false;

  for (auto entry : entries_)
    {
      if (entry.decrypt_with_password_hash (h))
        rc = true;
    }

  rc |= decrypt_sequence (entries_);

  return rc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt CREDHIST entries with password
// @param password Password
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
credhist_file::decrypt_with_password (const std::string& password)
{
  mobius::crypt::hash h ("sha1");
  h.update (conv_charset (password, "UTF-8", "UTF-16LE"));

  return decrypt_with_password_hash (h.get_digest ());
}

} // namespace mobius::os::win::dpapi


