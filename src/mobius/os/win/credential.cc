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
#include "credential.h"
#include <mobius/charset.h>
#include <mobius/core/log.h>
#include <mobius/decoder/data_decoder.h>

namespace mobius::os::win
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief credential implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class credential::impl
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl (const impl&) = delete;
  impl (impl&&) = delete;
  explicit impl (mobius::io::reader);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl& operator= (const impl&) = delete;
  impl& operator= (impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get domain
  // @return Domain
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_domain () const
  {
    return domain_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get username
  // @return Username
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_username () const
  {
    return username_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get password
  // @return Password
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_password () const
  {
    return password_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get password data
  // @return Password data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::bytearray
  get_password_data () const
  {
    return password_data_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get flags
  // @return Flags
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_flags () const
  {
    return flags_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get last update time
  // @return Last update time
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::datetime::datetime
  get_last_update_time () const
  {
    return last_update_time_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get data
  // @return data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <std::pair <std::string, mobius::bytearray>>
  get_data () const
  {
    return data_;
  }

private:
  // @brief Domain name
  std::string domain_;

  // @brief User name
  std::string username_;

  // @brief Password
  std::string password_;

  // @brief Password data
  mobius::bytearray password_data_;

  // @brief Flags
  std::uint32_t flags_;

  // @brief Last update date/time
  mobius::datetime::datetime last_update_time_;

  // @brief Credential data
  std::vector <std::pair <std::string, mobius::bytearray>> data_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
credential::impl::impl (mobius::io::reader reader)
{
  mobius::core::log log (__FILE__, __FUNCTION__);
  mobius::decoder::data_decoder decoder (reader);

  // check header size
  auto header_size = decoder.get_uint32_le ();

  if (header_size == 0)
    return;

  // decode header
  auto blob_size = decoder.get_uint32_le ();
  auto u1 = decoder.get_uint32_le ();
  auto u2 = decoder.get_uint32_le ();
  auto u3 = decoder.get_uint32_le ();
  last_update_time_ = decoder.get_nt_datetime ();
  auto u4 = decoder.get_uint32_le ();
  auto u5 = decoder.get_uint32_le ();
  flags_ = decoder.get_uint32_le ();
  auto u7 = decoder.get_uint32_le ();
  auto u8 = decoder.get_uint32_le ();

  log.development (__LINE__, "win::credential flags=" + std::to_string (flags_));
  log.development (__LINE__, "win::credential u1=" + std::to_string (u1));
  log.development (__LINE__, "win::credential u2=" + std::to_string (u2));
  log.development (__LINE__, "win::credential u3=" + std::to_string (u3));
  log.development (__LINE__, "win::credential u4=" + std::to_string (u4));
  log.development (__LINE__, "win::credential u5=" + std::to_string (u5));
  log.development (__LINE__, "win::credential u7=" + std::to_string (u7));
  log.development (__LINE__, "win::credential u8=" + std::to_string (u8));

  auto size = decoder.get_uint32_le ();
  domain_ = decoder.get_string_by_size (size, "utf-16le");

  size = decoder.get_uint32_le ();
  auto s1 = decoder.get_string_by_size (size, "utf-16le");

  size = decoder.get_uint32_le ();
  auto s2 = decoder.get_string_by_size (size, "utf-16le");

  size = decoder.get_uint32_le ();
  auto s3 = decoder.get_string_by_size (size, "utf-16le");

  size = decoder.get_uint32_le ();
  username_ = decoder.get_string_by_size (size, "utf-16le");

  size = decoder.get_uint32_le ();
  password_data_ = decoder.get_bytearray_by_size (size);

  log.development (__LINE__, "win::credential s1=" + s1);
  log.development (__LINE__, "win::credential s2=" + s2);
  log.development (__LINE__, "win::credential s3=" + s3);

  // try to decode password
  try
    {
      password_ = conv_charset_to_utf8 (password_data_, "utf-16le");
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, "win::credential " + std::string (e.what ()));
    }

  // decode data blocks
  while (decoder.tell () < blob_size)
    {
      auto unk1 = decoder.get_uint32_le ();

      if (unk1 != 0)
        log.development (__LINE__, "win::credential unk1=" + std::to_string (unk1));

      size = decoder.get_uint32_le ();
      auto name = decoder.get_string_by_size (size, "utf-16le");

      size = decoder.get_uint32_le ();
      auto value = decoder.get_bytearray_by_size (size);

      data_.emplace_back (std::make_pair (name, value));
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
credential::credential (mobius::io::reader reader)
  : impl_ (std::make_shared <impl> (reader))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get domain
// @return Domain
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
credential::get_domain () const
{
  return impl_->get_domain ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get username
// @return Username
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
credential::get_username () const
{
  return impl_->get_username ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get password
// @return Password
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
credential::get_password () const
{
  return impl_->get_password ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get password data
// @return Password data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
credential::get_password_data () const
{
  return impl_->get_password_data ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get flags
// @return Flags
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
credential::get_flags () const
{
  return impl_->get_flags ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last update time
// @return Last update time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
credential::get_last_update_time () const
{
  return impl_->get_last_update_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get data
// @return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <std::pair <std::string, mobius::bytearray>>
credential::get_data () const
{
  return impl_->get_data ();
}

} // namespace mobius::os::win


