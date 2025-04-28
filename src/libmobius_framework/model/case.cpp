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
#include <mobius/framework/model/case.hpp>
#include <mobius/framework/model/item.hpp>
#include <mobius/exception.inc>
#include <mobius/exception_posix.inc>
#include <mobius/database/database.h>
#include <mobius/io/path.h>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <sys/stat.h>
#include <sys/types.h>

namespace mobius::framework::model
{
void case_schema (mobius::database::database);

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Datatypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
using uid_type = std::uint32_t;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Module data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief data mutex
static std::mutex mutex_;

// @brief next case UID
static uid_type next_uid_ = 1;

// @brief opened cases
static std::unordered_map <uid_type, Case> cases_;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr char DIR_SEPARATOR = '/';

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create directory recursively
// @param path directory path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
make_directory (const std::string& path)
{
  if (mkdir (path.c_str (), 0700) == -1)
    {
      if (errno == ENOENT)
        {
          auto idx = path.rfind (DIR_SEPARATOR);

          if (idx == std::string::npos)
            throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("Cannot create directory"));

          std::string head = path.substr (0, idx);
          make_directory (head);
          make_directory (path);
        }

      else if (errno != EEXIST)
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Join paths
// @param abs_path Absolute path
// @param rel_path Relative path
// @return Full path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
join_path (const std::string& abs_path, const std::string& rel_path)
{
  auto path = mobius::io::path (abs_path);
  return to_string (path.get_child_by_path (rel_path));
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Case implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Case::impl
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl (const impl&) = delete;
  impl (impl&&) = delete;
  impl (const std::string&, std::uint32_t);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // function prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string get_path (const std::string&) const;
  std::string create_path (const std::string&) const;
  mobius::database::connection new_connection ();
  mobius::database::transaction new_transaction ();
  mobius::database::database get_database () const;
  bool has_item_by_uid (std::int64_t) const;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get root item
  // @return root item
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::int64_t
  get_root_item_uid () const
  {
    return root_item_uid_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get uid
  // @return uid
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_uid () const
  {
    return uid_;
  }

private:
  // @brief case UID
  std::uint32_t uid_;

  // @brief root item UID
  std::int64_t root_item_uid_;

  // @brief base directory
  std::string base_dir_;

  // @brief database connection pool
  mobius::database::connection_pool pool_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param path case folder path
// @param uid case UID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Case::impl::impl (const std::string& path, std::uint32_t uid)
  : uid_ (uid),
    base_dir_ (path),
    pool_ (join_path (path, "case.sqlite"))

{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create tables
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = pool_.get_database ();
  db.execute ("pragma foreign_keys=ON");

  auto transaction = db.new_transaction ();
  case_schema (db);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // get root item UID, if any
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto stmt = db.new_statement (
           "SELECT uid "
             "FROM item "
            "WHERE parent_uid IS NULL");

  if (stmt.fetch_row ())
    root_item_uid_ = stmt.get_column_int64 (0);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // otherwise, populate case and root item
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  else
    {
      // create case row
      stmt = db.new_statement (
               "INSERT INTO 'case'"
                    "VALUES (1, DATETIME ('NOW'))");

      stmt.execute ();


      // create root item
      stmt = db.new_statement (
               "INSERT INTO item "
                    "VALUES (NULL, NULL, 1, 'case', DATETIME ('NOW'))");

      stmt.execute ();

      root_item_uid_ = db.get_last_insert_row_id ();
    }

  transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get path inside case folder
// @param rpath Relative path
// @return Full path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
Case::impl::get_path (const std::string& rpath) const
{
  return join_path (base_dir_, rpath);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create path inside case folder
// @param rpath relative path
// @return full path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
Case::impl::create_path (const std::string& rpath) const
{
  const std::string fullpath = join_path (base_dir_, rpath);

  mobius::io::path path (fullpath);
  make_directory (path.get_dirname ());

  return fullpath;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create new connection for case database
// @return new connection object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::database::connection
Case::impl::new_connection ()
{
  return pool_.acquire ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create new transaction
// @return new transaction object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::database::transaction
Case::impl::new_transaction ()
{
  auto db = pool_.get_database ();
  return db.new_transaction ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get database object
// @return database object for current thread
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::database::database
Case::impl::get_database () const
{
  return pool_.get_database ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get item by UID
// @param uid item UID
// @return item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
Case::impl::has_item_by_uid (std::int64_t uid) const
{
  auto db = pool_.get_database ();

  auto stmt = db.new_statement (
                "SELECT 1 "
                  "FROM item "
                 "WHERE uid = ?");

  stmt.bind (1, uid);

  return stmt.fetch_row ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param path case folder path
// @param uid case UID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Case::Case (const std::string& path, std::uint32_t uid)
  : impl_ (std::make_shared <impl> (path, uid))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get path inside case folder
// @param rpath relative path
// @return full path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
Case::get_path (const std::string& rpath) const
{
  return impl_->get_path (rpath);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create path inside case folder
// @param rpath relative path
// @return full path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
Case::create_path (const std::string& rpath) const
{
  return impl_->create_path (rpath);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create new connection for case database
// @return new connection object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::database::connection
Case::new_connection ()
{
  return impl_->new_connection ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create new transaction
// @return new transaction object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::database::transaction
Case::new_transaction ()
{
  return impl_->new_transaction ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get database object
// @return database object for current thread
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::database::database
Case::get_database () const
{
  return impl_->get_database ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get item by UID
// @param uid item UID
// @return item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
item
Case::get_item_by_uid (std::int64_t uid) const
{
  item it;

  if (impl_->has_item_by_uid (uid))
    it = item (*this, uid);

  return it;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get root item
// @return root item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
item
Case::get_root_item () const
{
  return item (*this, impl_->get_root_item_uid ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get uid
// @return uid
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
Case::get_uid () const
{
  return impl_->get_uid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get passwords
// @return Passwords
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <evidence>
Case::get_passwords () const
{
  auto db = impl_->get_database ();

  auto stmt = db.new_statement (
                "SELECT item_uid, uid "
                  "FROM evidence "
                 "WHERE type = 'password' "
              "ORDER BY item_uid");

  std::vector <evidence> passwords;
  item i (*this, -1);

  while (stmt.fetch_row ())
    {
      auto item_uid = stmt.get_column_int64 (0);
      auto uid = stmt.get_column_int64 (1);

      if (i.get_uid () != item_uid)
        i = item (*this, item_uid);

      passwords.emplace_back (i, uid, "password");
    }

  return passwords;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get password hashes
// @return Password hashes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <evidence>
Case::get_password_hashes () const
{
  auto db = impl_->get_database ();

  auto stmt = db.new_statement (
                "SELECT item_uid, uid "
                  "FROM evidence "
                 "WHERE type = 'password_hash' "
              "ORDER BY item_uid");

  std::vector <evidence> password_hashes;
  item i (*this, -1);

  while (stmt.fetch_row ())
    {
      auto item_uid = stmt.get_column_int64 (0);
      auto uid = stmt.get_column_int64 (1);

      if (i.get_uid () != item_uid)
        i = item (*this, item_uid);

      password_hashes.emplace_back (i, uid, "password_hash");
    }

  return password_hashes;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if two cases are equal
// @param a case a
// @param b case b
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator== (const Case& a, const Case& b)
{
  return a.get_uid () == b.get_uid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if two cases are different
// @param a case a
// @param b case b
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator!= (const Case& a, const Case& b)
{
  return a.get_uid () != b.get_uid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Operator<
// @param a case a
// @param b case b
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator< (const Case& a, const Case& b)
{
  return a.get_uid () < b.get_uid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Operator<=
// @param a case a
// @param b case b
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator<= (const Case& a, const Case& b)
{
  return a.get_uid () <= b.get_uid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Operator>
// @param a case a
// @param b case b
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator> (const Case& a, const Case& b)
{
  return a.get_uid () > b.get_uid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Operator>=
// @param a case a
// @param b case b
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator>= (const Case& a, const Case& b)
{
  return a.get_uid () >= b.get_uid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new case
// @param path case folder path
// @return new case
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Case
new_case (const std::string& path)
{
  std::lock_guard <std::mutex> lock (mutex_);

  Case c (path, next_uid_);
  cases_[c.get_uid ()] = c;
  ++next_uid_;

  return c;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Open case
// @param path case folder path
// @return opened case
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Case
open_case (const std::string& path)
{
  std::lock_guard <std::mutex> lock (mutex_);

  Case c (path, next_uid_);
  cases_[c.get_uid ()] = c;
  ++next_uid_;

  return c;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Close case
// @param c case object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
close_case (const Case& c)
{
  std::lock_guard <std::mutex> lock (mutex_);
  cases_.erase (c.get_uid ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get opened cases
// @return opened cases
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <Case>
get_cases ()
{
  std::vector <Case> cases;
  std::lock_guard <std::mutex> lock (mutex_);

  for (const auto& p : cases_)
    cases.push_back (p.second);

  return cases;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of cases opened
// @return number
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
get_case_count ()
{
  std::lock_guard <std::mutex> lock (mutex_);
  return cases_.size ();
}

} // namespace mobius::framework::model
