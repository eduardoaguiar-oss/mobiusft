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
#include "hive_key.h"
#include "hive_decoder.h"
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/string_functions.h>

namespace mobius::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::uint16_t KEY_HIVE_ENTRY = 0x0004;
static constexpr std::uint16_t KEY_NO_DELETE = 0x0008;
static constexpr std::uint16_t KEY_SYM_LINK = 0x0010;
static constexpr std::uint32_t INVALID_OFFSET = 0xffffffff;
static constexpr std::uint32_t HIVE_BASE_OFFSET = 4096;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get absolute offset
// @param offset relative offset
// @return absolute offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::uint32_t
decode_offset (mobius::core::decoder::data_decoder& decoder)
{
  auto offset = decoder.get_uint32_le ();

  if (offset != INVALID_OFFSET)
    offset += HIVE_BASE_OFFSET;

  return offset;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class hive_key::impl
{
public:
  // constructors
  impl () = default;
  impl (const mobius::io::reader&, offset_type);
  impl (const impl&) = delete;
  impl (impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief check if object is valid
  // @return true/false
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  operator bool () const
  {
    return offset_ != INVALID_OFFSET;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get offset
  // @return offset
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  offset_type
  get_offset () const
  {
    return offset_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief check if object is readonly
  // @return true/false
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bool
  is_readonly () const
  {
    _load_metadata ();
    return is_readonly_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief check if object is symlink
  // @return true/false
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bool
  is_symlink () const
  {
    _load_metadata ();
    return is_symlink_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief check if object is root key
  // @return true/false
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bool
  is_root_key () const
  {
    _load_metadata ();
    return is_root_key_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get signature
  // @return signature
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_signature () const
  {
    _load_metadata ();
    return signature_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get flags
  // @return flags
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint16_t
  get_flags () const
  {
    _load_metadata ();
    return flags_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get last modification time
  // @return last modification time
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::core::datetime::datetime
  get_last_modification_time () const
  {
    _load_metadata ();
    return last_modification_time_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get access bits
  // @return access bits
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_access_bits () const
  {
    _load_metadata ();
    return access_bits_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get number of subkeys
  // @return number of subkeys
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_subkeys_count () const
  {
    _load_metadata ();
    return subkeys_count_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get number of volatile subkeys
  // @return number of volatile subkeys
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_volatile_subkeys_count () const
  {
    _load_metadata ();
    return volatile_subkeys_count_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get number of values
  // @return number of values
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_values_count () const
  {
    _load_metadata ();
    return values_count_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get name
  // @return name
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_name () const
  {
    _load_metadata ();
    return name_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get classname
  // @return classname
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_classname () const
  {
    _load_metadata ();
    return classname_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get subkeys offset
  // @return subkeys offset
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_subkeys_offset () const
  {
    _load_metadata ();
    return subkeys_offset_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get volatile subkeys offset
  // @return volatile subkeys offset
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_volatile_subkeys_offset () const
  {
    _load_metadata ();
    return volatile_subkeys_offset_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get parent offset
  // @return parent offset
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_parent_offset () const
  {
    _load_metadata ();
    return parent_offset_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get security offset
  // @return security offset
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_security_offset () const
  {
    _load_metadata ();
    return security_offset_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get values offset
  // @return values offset
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_values_offset () const
  {
    _load_metadata ();
    return values_offset_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get parent key
  // @return key
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  hive_key
  get_parent () const
  {
    _load_metadata ();
    return hive_key (reader_, parent_offset_);
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief return iterator to the first item
  // @return iterator
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  const_iterator_type
  begin () const
  {
    _load_subkeys ();
    return subkeys_.begin ();
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief return iterator after the last item
  // @return iterator
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  const_iterator_type
  end () const
  {
    _load_subkeys ();
    return subkeys_.end ();
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get values
  // @return values
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <hive_value>
  get_values () const
  {
    _load_values ();
    return values_;
  }

private:
  // @brief generic reader
  mobius::io::reader reader_;

  // @brief offset in bytes
  offset_type offset_ = INVALID_OFFSET;

  // @brief set if key is readonly
  mutable bool is_readonly_;

  // @brief set if key is symlink
  mutable bool is_symlink_;

  // @brief set if key is the root key
  mutable bool is_root_key_;

  // @brief signature (nk)
  mutable std::string signature_;

  // @brief flags (bit mask)
  mutable std::uint16_t flags_ = 0;

  // @brief last written timestamp
  mutable mobius::core::datetime::datetime last_modification_time_;

  // @brief access bits (bit mask)
  mutable std::uint32_t access_bits_ = 0;

  // @brief number of subkeys
  mutable std::uint32_t subkeys_count_;

  // @brief number of volatile subkeys
  mutable std::uint32_t volatile_subkeys_count_;

  // @brief number of values
  mutable std::uint32_t values_count_;

  // @brief key name
  mutable std::string name_;

  // @brief class name
  mutable std::string classname_;

  // @brief offset to the subkeys list
  mutable std::uint32_t subkeys_offset_;

  // @brief offset to the volatile subkeys list
  mutable std::uint32_t volatile_subkeys_offset_;

  // @brief offset to the parent key
  mutable std::uint32_t parent_offset_;

  // @brief offset to the security data
  mutable std::uint32_t security_offset_;

  // @brief offset to the values list
  mutable std::uint32_t values_offset_;

  // @brief subkeys
  mutable std::vector <hive_key> subkeys_;

  // @brief values
  mutable std::vector <hive_value> values_;

  // @brief metadata loaded flag
  mutable bool metadata_loaded_ = false;

  // @brief subkeys loaded flag
  mutable bool subkeys_loaded_ = false;

  // @brief values loaded flag
  mutable bool values_loaded_ = false;

  // helper functions
  void _load_metadata () const;
  void _load_subkeys () const;
  void _load_values () const;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader generic reader
// @param offset offset in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_key::impl::impl (const mobius::io::reader& reader, offset_type offset)
  : reader_ (reader),
    offset_ (offset)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load metadata on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hive_key::impl::_load_metadata () const
{
  // check if metadata is already loaded
  if (metadata_loaded_)
    return;

  // set metadata loaded
  metadata_loaded_ = true;

  // check if it can retrieve data
  if (offset_ == INVALID_OFFSET || offset_ >= reader_.get_size ())
    return;

  // create decoder
  mobius::core::decoder::data_decoder decoder (reader_);
  decoder.seek (offset_);

  // get cell size
  std::int32_t cellsize = decoder.get_int32_le ();

  if (cellsize >= 0)
    return;

  // retrieve metadata
  signature_ = decoder.get_string_by_size (2);
  flags_ = decoder.get_uint16_le ();
  last_modification_time_ = decoder.get_nt_datetime ();
  access_bits_ = decoder.get_uint32_le ();
  parent_offset_ = decode_offset (decoder);
  subkeys_count_ = decoder.get_uint32_le ();
  volatile_subkeys_count_ = decoder.get_uint32_le ();
  subkeys_offset_ = decode_offset (decoder);
  volatile_subkeys_offset_ = decode_offset (decoder);
  values_count_ = decoder.get_uint32_le ();
  values_offset_ = decode_offset (decoder);
  security_offset_ = decode_offset (decoder);

  // class name offset
  std::uint32_t classname_offset = decode_offset (decoder);

  // largest subkey name length, largest subkey class name length,
  // largest value name length, largest value data length, work_var
  decoder.skip (20);

  // key name length in bytes
  std::uint16_t key_name_length = decoder.get_uint16_le ();
  std::uint16_t classname_length = decoder.get_uint16_le ();

  // set name
  constexpr std::uint16_t KEY_COMP_NAME = 0x0020;
  std::string encoding = (flags_ & KEY_COMP_NAME) ? "CP1252" : "UTF-16LE";
  name_ = decoder.get_string_by_size (key_name_length, encoding);

  // set classname
  if (classname_offset != INVALID_OFFSET)
    {
      decoder.seek (classname_offset);
      std::int32_t cellsize = decoder.get_int32_le ();

      if (cellsize <= 0)
        classname_ = decoder.get_string_by_size (classname_length, "UTF-16LE");
    }

  // flags
  is_readonly_ = flags_ & KEY_NO_DELETE;
  is_symlink_ = flags_ & KEY_SYM_LINK;
  is_root_key_ = flags_ & KEY_HIVE_ENTRY;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load subkeys on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hive_key::impl::_load_subkeys () const
{
  // check if subkeys are already loaded
  if (subkeys_loaded_)
    return;

  _load_metadata ();

  // set subkeys loaded
  subkeys_loaded_ = true;

  // check if it can retrieve data
  if (offset_ == INVALID_OFFSET || subkeys_offset_ == INVALID_OFFSET || subkeys_offset_ >= reader_.get_size ())
    return;

  // create decoder
  hive_decoder decoder (reader_);
  auto offsets = decoder.decode_subkeys_list (subkeys_offset_);

  // build subkeys_
  subkeys_.reserve (offsets.size ());
  for (auto offset : offsets)
    subkeys_.push_back (hive_key (reader_, offset));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load values on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hive_key::impl::_load_values () const
{
  // check if values are already loaded
  if (values_loaded_)
    return;

  _load_metadata ();

  // set values loaded
  values_loaded_ = true;

  // check if it can retrieve data
  if (offset_ == INVALID_OFFSET || values_offset_ == INVALID_OFFSET || values_offset_ >= reader_.get_size ())
    return;

  // create decoder
  hive_decoder decoder (reader_);
  auto offsets = decoder.decode_values_list (values_offset_, values_count_);

  // populate values_
  values_.reserve (offsets.size ());
  for (auto offset : offsets)
    values_.push_back (hive_value (reader_, offset));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_key::hive_key ()
  : impl_ (std::make_shared <impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Generic reader
// @param offset Offset in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_key::hive_key (const mobius::io::reader& reader, offset_type offset)
  : impl_ (std::make_shared <impl> (reader, offset))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief check if object is valid
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_key::operator bool () const
{
  return impl_->operator bool ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get offset
// @return offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_key::offset_type
hive_key::get_offset () const
{
  return impl_->get_offset ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief check if object is readonly
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
hive_key::is_readonly () const
{
  return impl_->is_readonly ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief check if object is symlink
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
hive_key::is_symlink () const
{
  return impl_->is_symlink ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief check if object is root key
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
hive_key::is_root_key () const
{
  return impl_->is_root_key ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get signature
// @return signature
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
hive_key::get_signature () const
{
  return impl_->get_signature ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get flags
// @return flags
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint16_t
hive_key::get_flags () const
{
  return impl_->get_flags ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get last modification time
// @return last modification time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
hive_key::get_last_modification_time () const
{
  return impl_->get_last_modification_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get access bits
// @return access bits
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
hive_key::get_access_bits () const
{
  return impl_->get_access_bits ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get subkeys count
// @return subkeys count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
hive_key::get_subkeys_count () const
{
  return impl_->get_subkeys_count ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get volatile subkeys count
// @return volatile subkeys count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
hive_key::get_volatile_subkeys_count () const
{
  return impl_->get_volatile_subkeys_count ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get values count
// @return values count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
hive_key::get_values_count () const
{
  return impl_->get_values_count ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get name
// @return name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
hive_key::get_name () const
{
  return impl_->get_name ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get classname
// @return classname
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
hive_key::get_classname () const
{
  return impl_->get_classname ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get subkeys offset
// @return subkeys offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
hive_key::get_subkeys_offset () const
{
  return impl_->get_subkeys_offset ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get volatile subkeys offset
// @return volatile subkeys offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
hive_key::get_volatile_subkeys_offset () const
{
  return impl_->get_volatile_subkeys_offset ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get parent offset
// @return parent offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
hive_key::get_parent_offset () const
{
  return impl_->get_parent_offset ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get security offset
// @return security offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
hive_key::get_security_offset () const
{
  return impl_->get_security_offset ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get values offset
// @return values offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
hive_key::get_values_offset () const
{
  return impl_->get_values_offset ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get parent
// @return parent
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_key
hive_key::get_parent () const
{
  return impl_->get_parent ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief return iterator to the first subkey
// @return iterator
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_key::const_iterator_type
hive_key::begin () const
{
  return impl_->begin ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief return iterator after the last subkey
// @return iterator
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_key::const_iterator_type
hive_key::end () const
{
  return impl_->end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get values
// @return values
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <hive_value>
hive_key::get_values () const
{
  return impl_->get_values ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief check if hive key has subkeys
// @return true if hive key has at least one subkey, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
hive_key::has_subkeys () const
{
  return impl_->get_subkeys_count () > 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get subkey by name
// @param name subkey name
// @return subkey or empty key, if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_key
hive_key::get_key_by_name (const std::string& name) const
{
  const std::string lname = mobius::string::tolower (name);

  for (const auto& sk : *this)
    if (mobius::string::tolower (sk.get_name ()) == lname)
      return sk;

  return hive_key ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get subkeys by mask
// @param a_mask fnmatch mask
// @return Subkeys or empty vector, if no keys were found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <hive_key>
hive_key::get_keys_by_mask (const std::string& a_mask) const
{
  const std::string mask = mobius::string::tolower (a_mask);
  std::string::size_type pos = 0;

  // skip heading '\'
  while (pos < mask.size () && mask[pos] == '\\')
    ++pos;

  // seek each subkey which name is limited by '\\'
  std::vector <hive_key> subkeys = {*this};
  std::string::size_type end = mask.find ('\\', pos);

  while (pos != std::string::npos && !subkeys.empty ())
    {
      // get sub mask
      std::string submask;

      if (end == std::string::npos)
        {
          submask = mask.substr (pos);
          pos = std::string::npos;
        }

      else
        {
          submask = mask.substr (pos, end - pos);
          pos = end + 1;
          end = mask.find ('\\', pos);
        }

      // create new vector with all subkeys whose name matches the sub mask
      std::vector <hive_key> tmp_keys;

      for (auto key : subkeys)
        {
          for (auto sk : key)
            {
              const std::string name = mobius::string::tolower (sk.get_name ());

              if (mobius::string::fnmatch (submask, name))
                tmp_keys.push_back (sk);
            }
        }

      // overwrite the candidate vector
      subkeys = tmp_keys;
    }

  // return subkeys
  return subkeys;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get subkey by path
// @param path subkey path
// @return subkey or empty key, if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_key
hive_key::get_key_by_path (const std::string& path) const
{
  std::string::size_type pos = 0;

  // skip heading '\'
  while (pos < path.size () && path[pos] == '\\')
    ++pos;

  // seek each subkey which name is limited by '\\'
  hive_key key = *this;
  std::string::size_type end = path.find ('\\', pos);

  while (pos != std::string::npos && key)
    {
      std::string name;

      if (end == std::string::npos)
        {
          name = path.substr (pos);
          pos = std::string::npos;
        }

      else
        {
          name = path.substr (pos, end - pos);
          pos = end + 1;
          end = path.find ('\\', pos);
        }

      key = key.get_key_by_name (name);
    }

  // return key
  return key;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get value by name
// @param name value name
// @return value or empty value, if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_value
hive_key::get_value_by_name (const std::string& name) const
{
  const std::string lname = mobius::string::tolower (name);

  for (const auto& v : get_values ())
    if (mobius::string::tolower (v.get_name ()) == lname)
      return v;

  return hive_value ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get values by mask
// @param a_mask value mask
// @return values or empty vector, if no values were found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <hive_value>
hive_key::get_values_by_mask (const std::string& a_mask) const
{
  // parse registry_key.mask + '\' + value.mask
  const std::string mask = mobius::string::tolower (a_mask);
  std::string key_mask;
  std::string value_mask;

  auto pos = mask.rfind ('\\');

  if (pos == std::string::npos)
    value_mask = mask;

  else
    {
      key_mask = mask.substr (0, pos);
      value_mask = mask.substr (pos + 1);
    }

  // search keys
  std::vector <hive_key> keys;

  if (key_mask.empty ())
    keys.push_back (*this);

  else
    keys = get_keys_by_mask (key_mask);

  // search for values
  std::vector <hive_value> values;

  for (const auto& key : keys)
    {
      for (auto v : key.get_values ())
        {
          const std::string name = mobius::string::tolower (v.get_name ());

          if (mobius::string::fnmatch (value_mask, name))
            values.push_back (v);
        }
    }

  return values;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get data by value name
// @param name value name
// @return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_data
hive_key::get_data_by_name (const std::string& name) const
{
  hive_data data;

  auto v = get_value_by_name (name);
  if (v)
    data = v.get_data ();

  return data;
}

} // namespace mobius::os::win::registry


