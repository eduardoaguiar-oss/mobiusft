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
#include "cipher_impl_ige.h"
#include <mobius/exception.inc>

namespace mobius::crypt
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
//! \param cipher_id Cipher ID
//! \param key Cipher key
//! \param iv Initialization vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher_impl_ige::cipher_impl_ige (
  const std::string& cipher_id,
  const mobius::bytearray& key,
  const mobius::bytearray& iv)
: cipher_ (cipher_id, "ecb"),
  iv_ (iv),
  v1_ (iv.slice (0, iv.size () / 2 - 1)),
  v2_ (iv.slice (iv.size () / 2, iv.size () - 1))
{
  if (cipher_.is_stream ())
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("cannot use cipher mode with stream cipher"));

  cipher_.set_key (key);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Encrypt data
//! \param data Plaintext data
//! \return Encrypted data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
cipher_impl_ige::encrypt (const mobius::bytearray& data)
{
  mobius::bytearray out;
  const auto block_size = cipher_.get_block_size ();

  for (std::size_t i = 0;i < data.size ();i += block_size)
    {
      const auto block_data = data.slice (i, i + block_size - 1);
      v1_ = cipher_.encrypt (block_data ^ v1_) ^ v2_;
      v2_ = block_data;
      out += v1_;
    }

  return out;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Decrypt data
//! \param data Ciphertext data
//! \return Decrypted data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
cipher_impl_ige::decrypt (const mobius::bytearray& data)
{
  mobius::bytearray out;
  const auto block_size = cipher_.get_block_size ();

  for (std::size_t i = 0;i < data.size ();i += block_size)
    {
      const auto block_data = data.slice (i, i + block_size - 1);
      v2_ = cipher_.decrypt (block_data ^ v2_) ^ v1_;
      v1_ = block_data;
      out += v2_;
    }

  return out;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Reset cipher mode
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
cipher_impl_ige::reset ()
{
  v1_ = iv_.slice (0, iv_.size () / 2 - 1);
  v2_ = iv_.slice (iv_.size () / 2, iv_.size () - 1);
  cipher_.reset ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set final flag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
cipher_impl_ige::final ()
{
  cipher_.final ();
}

} // namespace mobius::crypt