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
#include <mobius/core/kff/kff.hpp>
#include <mobius/core/application.hpp>
#include <mobius/exception.inc>
#include <mobius/io/file.h>
#include <mobius/io/folder.h>
#include <mobius/io/path.h>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace mobius::core::kff
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// KFF data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Hashsets
static std::unordered_map <std::string, hashset> hashsets_;

// @brief Hashsets mutex
static std::mutex mutex_;

// @brief Is data loaded
static std::once_flag is_loaded_;

// @brief Database schema version
static constexpr int SCHEMA_VERSION = 1;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize kff data
// Read .sqlite files from kff config dir
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_init ()
{
  mobius::core::application app;
  auto path = app.get_config_path ("kff");
  auto folder = mobius::io::new_folder_by_path (path);

  if (!folder.exists ())
    folder.create ();

  else
    {
      for (const auto& c : folder.get_children ())
        {
          if (c.is_file () && c.get_extension () == "sqlite")
            {
              mobius::io::path p (path + '/' + c.get_name ());
              auto id = p.get_filename_prefix ();
              hashsets_.emplace (id, hashset (p.get_value ()));
            }
        }
    }
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
kff::kff ()
{
  std::call_once (is_loaded_, _init);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create connection_set object to hashset databases
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::database::connection_set
kff::new_connection ()
{
  mobius::database::connection_set cs;

  const std::lock_guard <std::mutex> lock (mutex_);

  for (auto& p : hashsets_)
    cs.add (p.second.new_connection ());

  return cs;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create hash set
// @param id Hash set ID
// @param is_alert If hash set is alert type
// @return hash set object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hashset
kff::new_hashset (const std::string& id, bool is_alert)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // check if hashset already exists
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  {
    const std::lock_guard <std::mutex> lock (mutex_);

    if (hashsets_.find (id) != hashsets_.end ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("hashset '" + id + "' already exists"));
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create database
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::core::application app;
  auto path = app.get_config_path ("kff/" + id + ".sqlite");

  hashset h (path);
  h.create (is_alert);

  {
    const std::lock_guard <std::mutex> lock (mutex_);
    hashsets_.emplace (id, h);
  }

  return h;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove hash set
// @param id Hash set ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
kff::remove_hashset (const std::string& id)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // remove from hashset_
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  {
    const std::lock_guard <std::mutex> lock (mutex_);
    hashsets_.erase (id);
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // delete database file
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::core::application app;
  auto path = app.get_config_path ("kff/" + id + ".sqlite");
  auto f = mobius::io::new_file_by_path (path);
  f.remove ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hashsets
// @return hashsets
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <std::pair <std::string, hashset>>
kff::get_hashsets () const
{
  const std::lock_guard <std::mutex> lock (mutex_);

  return std::vector <std::pair <std::string, hashset>> (hashsets_.begin (), hashsets_.end ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Lookup hash in alert hashset
// @param type Hash type
// @param value Hash value
// @return Hash sets
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <std::string>
kff::alert_lookup (const std::string& type, const std::string& value) const
{
  std::vector <std::string> hashset_ids;

  for (const auto& p : get_hashsets ())
    {
      auto id = p.first;
      auto hashset = p.second;

      if (hashset.is_alert () && hashset.lookup (type, value))
        hashset_ids.push_back (id);
    }

  return hashset_ids;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Lookup hash in all hash sets
// @param type Hash type
// @param value Hash value
// @return 'A' alert, 'I' ignored, 'N' not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
char
kff::lookup (const std::string& type, const std::string& value) const
{
  auto hashsets = get_hashsets ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // search first into is_alert hashsets
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& p : get_hashsets ())
    {
      auto id = p.first;
      auto hashset = p.second;

      if (hashset.is_alert () && hashset.lookup (type, value))
        return 'A';
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // then, search into non is_alert hashsets
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& p : get_hashsets ())
    {
      auto id = p.first;
      auto hashset = p.second;

      if (!hashset.is_alert () && hashset.lookup (type, value))
        return 'I';
    }

  return 'N';
}

} // namespace mobius::core::kff
