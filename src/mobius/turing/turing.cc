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
#include "turing.h"
#include <mobius/core/application.h>
#include <mobius/database/connection_pool.h>
#include <mobius/string_functions.h>
#include <string>
#include <tuple>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Usage:
//
// mobius::turing::turing turing;                // one object for each thread!
// auto transaction = turing.new_transaction ()  // only when modifying data
// ...
// operations (set_hash, remove_hashes, get_hash_password, get_hashes, ...)
// ...
// transaction.commit ();                        // only when have modified data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string LM_NULL = "aad3b435b51404ee";
static const std::string NT_NULL = "31d6cfe0d16ae931b73c59d7e0c089c0";
static constexpr int SCHEMA_VERSION = 2;

} // namespace


namespace mobius::turing
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Implementation class for turing class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class mobius::turing::turing::impl
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl () = default;
  impl (impl&&) = delete;
  impl (const impl&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl& operator=(const impl&) = delete;
  impl& operator=(impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Check if object is valid
  // @return true/false
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  operator bool() const noexcept
  {
    return is_database_loaded_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::database::transaction new_transaction ();
  bool has_hash (const std::string&, const std::string&);
  void set_hash (const std::string&, const std::string&, const std::string&);
  std::pair <pwd_status, std::string> get_hash_password (const std::string&, const std::string&) const;
  hashlist_type get_hashes () const;
  void remove_hashes ();

private:
  // @brief Flag is database loaded?
  mutable bool is_database_loaded_ = false;

  // @brief database object
  mutable mobius::database::database db_;

  // Helper functions
  void _load_database () const;
};


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new transaction for Turing database
// @return New database transaction
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::database::transaction
turing::impl::new_transaction ()
{
  _load_database();

  return db_.new_transaction ();
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if hash is set
// @param hash_type Hash type
// @param hash_value Hash value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
turing::impl::has_hash (
    const std::string& hash_type,
    const std::string& hash_value)
{
  _load_database();

  auto stmt = db_.new_statement (
                "SELECT 1 "
                  "FROM hash "
                 "WHERE type = ? "
                   "AND value = ?");

  stmt.bind (1, hash_type);
  stmt.bind (2, hash_value);

  return bool (stmt.fetch_row ());
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set hash
// @param hash_type Hash type
// @param hash_value Hash value
// @param password Password as UTF-8 string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
turing::impl::set_hash (
    const std::string& hash_type,
    const std::string& hash_value,
    const std::string& password)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // handle LM hashes as two separated halves
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (hash_type == "lm" && hash_value.length () > 16)
    {
      set_hash (hash_type,
                hash_value.substr (0, 16),
                mobius::string::toupper (password.substr (0, 7)));

      if (password.length () > 7)
        set_hash (hash_type,
                  hash_value.substr (16),
                  mobius::string::toupper (password.substr (7)));
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // if hash already exists then return
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (has_hash (hash_type, hash_value))
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // insert hash
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  _load_database();

  auto stmt = db_.new_statement (
                "INSERT INTO hash "
                     "VALUES (?, ?, ?)");

  stmt.bind (1, hash_type);
  stmt.bind (2, hash_value);
  stmt.bind (3, password);
  stmt.execute ();
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash password
// @param hash_type Hash type
// @param hash_value Hash value
// @return status, password
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::pair <turing::pwd_status, std::string>
turing::impl::get_hash_password (
    const std::string& hash_type,
    const std::string& hash_value) const
{
  std::string password;
  pwd_status status = pwd_status::not_found;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // test empty values
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (hash_type == "lm" && hash_value == LM_NULL)
    status = pwd_status::found;

  else if (hash_type == "nt" && hash_value == NT_NULL)
    status = pwd_status::found;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // handle LM hashes as two separated halves
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  else if (hash_type == "lm" && hash_value.length () > 16)
    {
      auto lm_1 = get_hash_password (hash_type, hash_value.substr (0, 16));
      auto lm_2 = get_hash_password (hash_type, hash_value.substr (16));

      if (lm_1.first == pwd_status::found && lm_2.first == pwd_status::found)
        {
          password = lm_1.second + lm_2.second;
          status = pwd_status::found;
        }

      else if (lm_1.first == pwd_status::found)
        {
          password = lm_1.second + "???????";
          status = pwd_status::lm_1_found;
        }

      else if (lm_2.first == pwd_status::found)
        {
          password = "???????" + lm_2.second;
          status = pwd_status::lm_2_found;
        }
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // select hash from table
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  else
    {
      _load_database();

      auto stmt = db_.new_statement (
                "SELECT password "
                  "FROM hash "
                 "WHERE type = ? "
                   "AND value = ?");

      stmt.bind (1, hash_type);
      stmt.bind (2, hash_value);

      if (stmt.fetch_row ())
        {
          password = stmt.get_column_string (0);
          status = pwd_status::found;
        }
    }

  return std::make_pair (status, password);
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hashes
// @return Tuples <type, value, password>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
turing::hashlist_type
turing::impl::get_hashes () const
{
  _load_database();

  auto stmt = db_.new_statement (
                "SELECT type, value, password "
                  "FROM hash "
              "ORDER BY type, value");

  hashlist_type hashes;

  while (stmt.fetch_row ())
    {
      const std::string hash_type = stmt.get_column_string (0);
      const std::string hash_value = stmt.get_column_string (1);
      const std::string password = stmt.get_column_string (2);
      hashes.emplace_back (hash_type, hash_value, password);
    }

  return hashes;
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove hashes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
turing::impl::remove_hashes ()
{
  _load_database();

  auto stmt = db_.new_statement ("DELETE FROM hash");
  stmt.execute ();
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load database, if necessary
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
turing::impl::_load_database () const
{
  if (is_database_loaded_)
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // open database
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::core::application app;
  const auto path = app.get_config_path ("turing.sqlite");
  db_ = mobius::database::database (path);
  db_.execute ("PRAGMA foreign_keys = OFF;");

  auto transaction = db_.new_transaction ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create table 'meta'
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  db_.execute (
    "CREATE TABLE IF NOT EXISTS meta ("
                 "key TEXT PRIMARY KEY,"
               "value TEXT NOT NULL"
    ");");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // set schema version
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto stmt = db_.new_statement (
                "SELECT value "
                  "FROM meta "
                 "WHERE key = 'version'");

  if (stmt.fetch_row ())
    {
      int current_version = stmt.get_column_int (0);

      if (current_version < SCHEMA_VERSION)
        {
          // update schema version
          stmt = db_.new_statement (
                   "UPDATE meta "
                      "SET value = ? "
                    "WHERE key = 'version'");

          stmt.bind (1, SCHEMA_VERSION);
          stmt.execute ();
        }
    }

  else
    {
      // insert 'version' metadata
      stmt = db_.new_statement (
               "INSERT INTO meta "
                    "VALUES ('version', ?)");

      stmt.bind (1, SCHEMA_VERSION);
      stmt.execute ();
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create 'hash' table
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  db_.execute (
     "CREATE TABLE IF NOT EXISTS hash"
              "(type TEXT NOT NULL,"
              "value TEXT NOT NULL,"
           "password TEXT NOT NULL,"
            "PRIMARY KEY (type, value))");

  transaction.commit ();
  db_.execute ("PRAGMA foreign_keys = ON;");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // set database loaded
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  is_database_loaded_ = true;
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
turing::turing ()
  : impl_ (std::make_shared<impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new transaction for Turing database
// @return new database transaction
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::database::transaction
turing::new_transaction ()
{
  return impl_->new_transaction ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if hash is set
// @param hash_type Hash type
// @param hash_value Hash value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
turing::has_hash (
    const std::string& hash_type,
    const std::string& hash_value)
{
  return impl_->has_hash (hash_type, hash_value);
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set hash
// @param hash_type Hash type
// @param hash_value Hash value
// @param password Password as UTF-8 string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
turing::set_hash (
    const std::string& hash_type,
    const std::string& hash_value,
    const std::string& password)
{
  impl_->set_hash (hash_type, hash_value, password);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash password
// @param hash_type Hash type
// @param hash_value Hash value
// @return status, password
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::pair <turing::pwd_status, std::string>
turing::get_hash_password (
    const std::string& hash_type,
    const std::string& hash_value) const
{
  return impl_->get_hash_password (hash_type, hash_value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hashes
// @return Tuples <type, value, password>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
turing::hashlist_type
turing::get_hashes () const
{
  return impl_->get_hashes ();
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove hashes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
turing::remove_hashes ()
{
  impl_->remove_hashes ();
}

} // namespace mobius::turing


