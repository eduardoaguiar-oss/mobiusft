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
#include "registry.h"
#include "registry_key_impl_container.h"
#include "registry_key_impl_link.h"
#include "registry_file.h"
#include <mobius/io/file.h>
#include <mobius/io/reader.h>
#include <mobius/string_functions.h>
#include <mobius/bytearray.h>
#include <algorithm>
#include <map>

namespace mobius
{
namespace os
{
namespace win
{
namespace registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// helper functions prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void registry_set_msdcc_key (registry_key);
void registry_set_lsa_keys (registry_key, const mobius::bytearray&);
void registry_set_pssp_keys (registry_key);
void registry_set_user_assist_keys (registry_key);

namespace               // local namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief create a new container key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static registry_key
new_container_key (const std::string& name = std::string ())
{
  auto key = registry_key (std::make_shared <registry_key_impl_container> ());

  if (!name.empty ())
    key.set_name (name);

  return key;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief create a new link to key
//! \param key registry key
//! \param name new name of the key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static registry_key
new_link (registry_key key, const std::string& name = std::string ())
{
  auto lkey = registry_key (std::make_shared <registry_key_impl_link> (key));

  if (!name.empty ())
    lkey.set_name (name);

  return lkey;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief expand abbreviated path (HKLM, HKU, ...)
//! \param path key, value or data path
//! \return expanded path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
expand_path (const std::string& path)
{
  std::string ret_path;
  std::string::size_type p1 = 0;
  std::string::size_type p2 = 0;

  // get first name starting pos
  while (p1 < path.size () && path[p1] == '\\')
    ++p1;

  ret_path = path.substr (0, p1);

  // get first name ending pos
  p2 = path.find ('\\', p1);

  if (p2 == std::string::npos)
    p2 = path.length ();

  // replace abbreviated name
  const std::string root_name = path.substr (p1, p2 - p1);
  const std::string root_lname = mobius::string::tolower (root_name);

  if (root_lname == "hklm")
    ret_path += "HKEY_LOCAL_MACHINE";

  else if (root_lname == "hkcu")
    ret_path +=  "HKEY_CURRENT_USER";

  else if (root_lname == "hku")
    ret_path += "HKEY_USERS";

  else if (root_lname == "hkcr")
    ret_path += "HKEY_CLASSES_ROOT";

  else if (root_lname == "hkcc")
    ret_path += "HKEY_CURRENT_CONFIG";

  else if (root_lname == "hkpd")
    ret_path += "HKEY_PERFORMANCE_DATA";

  else
    ret_path += root_name;

  // add the remaining string
  ret_path += path.substr (p2);

  return ret_path;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief registry implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class registry::impl
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  //! \brief check if object is valid
  //! \return true/false
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  operator bool () const
  {
    return files_.size () > 0;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  //! \brief get files
  //! \return files
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <registry_file>
  get_files () const
  {
    return files_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  //! \brief get keys
  //! \return keys
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <registry_key>
  get_keys () const
  {
    _load_keys ();
    return std::vector <registry_key> (root_.begin (), root_.end ());
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  //! \brief get syskey
  //! \return registry syskey
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::bytearray
  get_syskey () const
  {
    return syskey_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // function prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  registry_file add_file_by_path (const std::string&, const std::string&, const std::string&);
  registry_file add_file_by_url (const std::string&, const std::string&, const std::string&);
  registry_file add_file_by_reader (const std::string&, const std::string&, mobius::io::reader);
  void remove_file (std::uint32_t);

  registry_key get_key_by_path (const std::string&) const;
  std::vector <registry_key> get_key_by_mask (const std::string&) const;

  registry_value get_value_by_path (const std::string&) const;
  std::vector <registry_value> get_value_by_mask (const std::string&) const;

  registry_data get_data_by_path (const std::string&) const;
  std::vector <registry_data> get_data_by_mask (const std::string&) const;

private:
  //! \brief registry files
  std::vector <registry_file> files_;

  //! \brief next UID for registry file
  std::uint32_t next_uid_ = 1;

  //! \brief registry root keys container
  mutable registry_key root_;

  //! \brief keys loaded flag
  mutable bool keys_loaded_ = false;

  //! \brief syskey
  mutable mobius::bytearray syskey_;

  // helper functions
  void _load_keys () const;
  void _set_syskey () const;
  void _set_hkey_local_machine () const;
  void _set_hkey_users () const;
  void _set_hkey_classes_root () const;
  void _set_hkey_current_config () const;
  void _set_hkey_current_user () const;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry::impl::impl ()
  : root_ (new_container_key ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief add registry file by local path
//! \param role file role
//! \param path original path
//! \param localpath local path
//! \return new registry file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_file
registry::impl::add_file_by_path (const std::string& role, const std::string& path, const std::string& localpath)
{
  auto f = mobius::io::new_file_by_path (localpath);
  
  return add_file_by_reader (role, path, f.new_reader ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief add registry file by URL
//! \param role file role
//! \param path original path
//! \param url URL
//! \return new registry file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_file
registry::impl::add_file_by_url (const std::string& role, const std::string& path, const std::string& url)
{
  auto f = mobius::io::new_file_by_url (url);
  auto reader = f.new_reader ();

  return add_file_by_reader (role, path, reader);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief add registry file by reader
//! \param role file role
//! \param path original path
//! \param reader generic reader
//! \return new registry file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_file
registry::impl::add_file_by_reader (const std::string& role, const std::string& path, mobius::io::reader reader)
{
  registry_file r (next_uid_, role, path, reader);
  files_.push_back (r);

  // set state
  next_uid_++;
  keys_loaded_ = false;

  // return registry_file
  return r;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief remove registry file from registry
//! \param uid unique identifier
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry::impl::remove_file (std::uint32_t uid)
{
  auto lambda = [uid] (registry_file f)
  {
    return f.get_uid () == uid;
  };

  files_.erase (
    std::remove_if (files_.begin (), files_.end (), lambda),
    files_.end ()
  );

  keys_loaded_ = false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get key by path
//! \param name key path
//! \return key or empty key, if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_key
registry::impl::get_key_by_path (const std::string& path) const
{
  _load_keys ();
  return root_.get_key_by_path (expand_path (path));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get key by mask
//! \param mask fnmatch mask
//! \return subkeys or empty vector if none found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <registry_key>
registry::impl::get_key_by_mask (const std::string& mask) const
{
  _load_keys ();
  return root_.get_key_by_mask (expand_path (mask));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get value by path
//! \param name value path
//! \return value or empty value, if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_value
registry::impl::get_value_by_path (const std::string& path) const
{
  _load_keys ();
  return root_.get_value_by_path (expand_path (path));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get values by mask
//! \param mask value mask
//! \return values or empty vector, if no values were found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <registry_value>
registry::impl::get_value_by_mask (const std::string& mask) const
{
  _load_keys ();
  return root_.get_value_by_mask (expand_path (mask));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get data by path
//! \param name value path
//! \return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_data
registry::impl::get_data_by_path (const std::string& path) const
{
  _load_keys ();
  return root_.get_data_by_path (expand_path (path));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get data by mask
//! \param mask value mask
//! \return data or empty vector, if no values were found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <registry_data>
registry::impl::get_data_by_mask (const std::string& mask) const
{
  _load_keys ();
  return root_.get_data_by_mask (expand_path (mask));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief build registry keys according to the files added
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry::impl::_load_keys () const
{
  // if keys loaded, return
  if (keys_loaded_)
    return;

  // set root keys
  root_.clear_keys ();

  _set_hkey_local_machine ();
  _set_hkey_users ();
  _set_hkey_classes_root ();
  _set_hkey_current_config ();
  _set_hkey_current_user ();

  // set data
  _set_syskey ();

  // set special keys
  registry_set_lsa_keys (root_, syskey_);
  registry_set_pssp_keys (root_);
  registry_set_user_assist_keys (root_);
  registry_set_msdcc_key (root_);	// uses LSA keys

  // set keys loaded
  keys_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief set syskey
//! \see https://github.com/Neohapsis/creddump7/blob/master/framework/win32/lsasecrets.py
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry::impl::_set_syskey () const
{
  // load syskey
  auto lsa_key = root_.get_key_by_path ("\\HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Lsa");

  if (lsa_key)
    {
      auto jd_key = lsa_key.get_key_by_name ("JD");
      auto skew1_key = lsa_key.get_key_by_name ("Skew1");
      auto gbg_key = lsa_key.get_key_by_name ("GBG");
      auto data_key = lsa_key.get_key_by_name ("Data");

      if (jd_key && skew1_key && gbg_key && data_key)
        {
          mobius::bytearray tmp;
          tmp.from_hexstring (jd_key.get_classname () + skew1_key.get_classname () + gbg_key.get_classname () + data_key.get_classname ());
          syskey_ = mobius::bytearray ({tmp[8], tmp[5], tmp[4], tmp[2], tmp[11], tmp[9], tmp[13], tmp[3], tmp[0], tmp[6], tmp[1], tmp[12], tmp[14], tmp[10], tmp[15], tmp[7]});
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief set HKEY_LOCAL_MACHINE root key
//! \see https://msdn.microsoft.com/en-us/library/windows/desktop/ms724877(v=vs.85).aspx
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry::impl::_set_hkey_local_machine () const
{
  auto hklm = new_container_key ("HKEY_LOCAL_MACHINE");
  root_.add_key (hklm);

  // add HKLM subkeys
  for (const auto& f : files_)
    {
      if (f.get_role () != "NTUSER" && f.get_role () != "DEFAULT")
        {
          auto key = f.get_root_key ();
          key.set_name (f.get_role ());
          hklm.add_key (key);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief set HKEY_USERS root key
//! \see https://msdn.microsoft.com/en-us/library/windows/desktop/ms724877(v=vs.85).aspx
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry::impl::_set_hkey_users () const
{
  auto hku = new_container_key ("HKEY_USERS");
  root_.add_key (hku);

  // build profile map
  std::map <std::string, std::string> profile_map;

  for (auto k : root_.get_key_by_mask ("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\*"))
    {
      auto data = k.get_data_by_name ("ProfileImagePath");

      if (data)
        {
          const std::string sid = k.get_name ();

          std::string p_path = mobius::string::tolower (data.get_data_as_string ("utf-16"));
          p_path = mobius::string::replace (p_path, "%systemdrive%", "");
          p_path = mobius::string::replace (p_path, "%systemroot%", "\\windows");
          p_path = mobius::string::word (p_path, -1, ":");
          p_path += "\\ntuser.dat";

          profile_map[p_path] = sid;
        }
    }

  // process NTUSER.dat registry files
  std::uint64_t idx = 1;

  for (const auto& f : files_)
    {
      if (f.get_role () == "DEFAULT")
        {
          auto link = new_link (f.get_root_key (), ".DEFAULT");
          hku.add_key (link);
        }

      else if (f.get_role () == "NTUSER")
        {
          std::string key_name;
          auto key = f.get_root_key ();

          // try to get SID from profile_map
          if (key_name.empty ())
            {
              const std::string f_path = mobius::string::tolower (mobius::string::word (f.get_path (), -1, ":"));

              auto iter = profile_map.find (f_path);
              if (iter != profile_map.end ())
                key_name = iter->second;
            }

          // try to get PSSP SID as key name
          if (key_name.empty ())
            {
              auto pssp_key = key.get_key_by_path ("Software\\Microsoft\\Protected Storage System Provider");

              if (pssp_key && (pssp_key.begin () != pssp_key.end ()))
                key_name = pssp_key.begin ()->get_name ();
            }

          // no SID, create one
          if (key_name.empty ())
            key_name = "UNKNOWN-SID-" + mobius::string::to_string (idx++,  8);

          // add key to HKU
          key.set_name (key_name);
          hku.add_key (key);
        }
    }

  // create .Default as link to S-1-5-18
  //! \see https://blogs.msdn.microsoft.com/oldnewthing/20070302-00/?p=27783
  auto ls_key = hku.get_key_by_name ("S-1-5-18");

  //if (ls_key)
  //  {
  //    auto l = new_link (ls_key, ".Default");
  //    hku.add_key (l);
  //  }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief set HKEY_CLASSES_ROOT root key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry::impl::_set_hkey_classes_root () const
{
  registry_key hkcr;
  auto sk = root_.get_key_by_path ("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes");

  if (sk)
    hkcr = new_link (sk, "HKEY_CLASSES_ROOT");

  else
    hkcr = new_container_key ("HKEY_CLASSES_ROOT");

  root_.add_key (hkcr);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief set HKEY_CURRENT_CONFIG root key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry::impl::_set_hkey_current_config () const
{
  auto hkcc = new_container_key ("HKEY_CURRENT_CONFIG");

  // get HKLM\SYSTEM key
  auto system_key = root_.get_key_by_path ("HKEY_LOCAL_MACHINE\\SYSTEM");

  if (system_key)
    {
      // get current control set number
      std::uint32_t cc_set = 1;

      auto cc_set_data = system_key.get_data_by_path ("Select\\Current");
      if (cc_set_data)
        cc_set = cc_set_data.get_data_as_dword ();

      //set key HKLM\SYSTEM\CurrentControlSet
      const std::string name = "ControlSet" + mobius::string::to_string (cc_set, 3);
      auto ccs_key = system_key.get_key_by_name (name);

      if (ccs_key)
        {
          auto lccs = new_link (ccs_key, "CurrentControlSet");
          system_key.add_key (lccs);
        }

      // set key HKCC (HKEY_CURRENT_CONFIG)
      auto sk = system_key.get_key_by_path ("CurrentControlSet\\Hardware Profiles\\0001");

      if (sk)
        {
          hkcc = new_link (sk, "HKEY_CURRENT_CONFIG");

          auto hp_key = system_key.get_key_by_path ("CurrentControlSet\\Hardware Profiles");

          if (hp_key)
            hp_key.add_key (new_link (sk, "Current"));
        }
    }

  // set key HKCC (HKEY_CURRENT_CONFIG)
  root_.add_key (hkcc);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief set HKEY_CURRENT_USER root key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry::impl::_set_hkey_current_user () const
{
  auto hkcu = new_container_key ("HKEY_CURRENT_USER");
  //! \todo link to user with RID = 1000 or RID = 500.
  root_.add_key (hkcu);
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry::registry ()
  : impl_ (std::make_shared <impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief check if object is valid
//! \return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry::operator bool () const
{
  return impl_->operator bool ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief add registry file by local path
//! \param role file role
//! \param path original path
//! \param localpath local path
//! \return new registry file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_file
registry::add_file_by_path (const std::string& role, const std::string& path, const std::string& localpath)
{
  return impl_->add_file_by_path (role, path, localpath);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief add registry file URL
//! \param role file role
//! \param path original path
//! \param url URL
//! \return new registry file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_file
registry::add_file_by_url (const std::string& role, const std::string& path, const std::string& url)
{
  return impl_->add_file_by_url (role, path, url);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief add registry file by reader
//! \param role file role
//! \param path original path
//! \param reader generic reader
//! \return new registry file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_file
registry::add_file_by_reader (const std::string& role, const std::string& path, mobius::io::reader reader)
{
  return impl_->add_file_by_reader (role, path, reader);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief remove registry file from registry
//! \param uid unique identifier
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry::remove_file (std::uint32_t uid)
{
  impl_->remove_file (uid);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get files
//! \return files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <registry_file>
registry::get_files () const
{
  return impl_->get_files ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get keys
//! \return keys
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <registry_key>
registry::get_keys () const
{
  return impl_->get_keys ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get key by path
//! \param name key path
//! \return key or empty key, if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_key
registry::get_key_by_path (const std::string& path) const
{
  return impl_->get_key_by_path (path);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get key by mask
//! \param mask fnmatch mask
//! \return subkeys or empty vector if none found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <registry_key>
registry::get_key_by_mask (const std::string& mask) const
{
  return impl_->get_key_by_mask (mask);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get value by path
//! \param name value path
//! \return value or empty value, if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_value
registry::get_value_by_path (const std::string& path) const
{
  return impl_->get_value_by_path (path);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get values by mask
//! \param mask value mask
//! \return values or empty vector, if no values were found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <registry_value>
registry::get_value_by_mask (const std::string& mask) const
{
  return impl_->get_value_by_mask (mask);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get data by path
//! \param name value path
//! \return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_data
registry::get_data_by_path (const std::string& path) const
{
  return impl_->get_data_by_path (path);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get data by mask
//! \param mask value mask
//! \return data or empty vector, if no values were found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <registry_data>
registry::get_data_by_mask (const std::string& mask) const
{
  return impl_->get_data_by_mask (mask);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get syskey
//! \return registry syskey
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
registry::get_syskey () const
{
  return impl_->get_syskey ();
}

} // namespace registry
} // namespace win
} // namespace os
} // namespace mobius
