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
#include "category.h"
#include <mobius/core/application.h>
#include <mobius/core/thread_guard.h>
#include <mobius/database/database.h>

namespace
{
static const std::string RESOURCE_ID = "database.category";

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get database instance
// @return Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::database::database
_get_database ()
{
  mobius::database::database db;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // If DB instance is already opened for current thread, simply return it
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (mobius::core::has_thread_resource (RESOURCE_ID))
    db = mobius::core::get_thread_resource <mobius::database::database> (RESOURCE_ID);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Otherwise, create a new DB instance and associate to current thread
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  else
    {
      // create database instance
      mobius::core::application app;
      auto path = app.get_config_path ("category.sqlite");

      db = mobius::database::database (path);

      // create db tables, if necessary
      auto transaction = db.new_transaction ();

      db.execute ("PRAGMA foreign_keys = ON;");

      db.execute (
        "CREATE TABLE IF NOT EXISTS category"
        "(uid INTEGER PRIMARY KEY AUTOINCREMENT,"
        "id TEXT NOT NULL,"
        "name TEXT NULL,"
        "description TEXT NULL,"
        "icon_data BLOB NULL)"
      );

      db.execute (
        "CREATE UNIQUE INDEX IF NOT EXISTS idx_category "
        "ON category (id)");

      db.execute (
        "CREATE TABLE IF NOT EXISTS attribute"
        "(uid INTEGER PRIMARY KEY AUTOINCREMENT,"
        "category_uid INTEGER NOT NULL,"
        "id TEXT NOT NULL,"
        "name TEXT NULL,"
        "description TEXT NULL,"
        "datatype TEXT NULL,"
        "value_mask TEXT NULL,"
        "idx INTEGER NOT NULL,"
        "FOREIGN KEY (category_uid) REFERENCES category (uid) ON DELETE CASCADE)"
      );

      db.execute (
        "CREATE UNIQUE INDEX IF NOT EXISTS idx_attribute_1 "
        "ON attribute (category_uid, id)");

      transaction.commit ();

      // save DB instance for future reuse
      mobius::core::set_thread_resource (RESOURCE_ID, db);
    }

  return db;
}

} // namespace


namespace mobius::framework
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Category implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class category::impl
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl () = default;
  explicit impl (std::int64_t);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief check if object is valid
  // @return true/false
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  operator bool () const
  {
    return uid_ != -1;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get id
  // @return id
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_id () const
  {
    _load_data ();
    return id_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get name
  // @return name
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_name () const
  {
    _load_data ();
    return name_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get description
  // @return description
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_description () const
  {
    _load_data ();
    return description_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief get icon data
  // @return icon data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::bytearray
  get_icon_data () const
  {
    _load_data ();
    return icon_data_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void set_name (const std::string&);
  void set_description (const std::string&);
  void set_icon_data (const mobius::bytearray&);
  attribute get_attribute (const std::string&) const;
  attribute new_attribute (const std::string&);
  void remove_attribute (const std::string&);
  std::vector <attribute> get_attributes () const;

private:
  // @brief unique ID
  std::int64_t uid_ = -1;

  // @brief category ID
  mutable std::string id_;

  // @brief name
  mutable std::string name_;

  // @brief description
  mutable std::string description_;

  // @brief icon data
  mutable mobius::bytearray icon_data_;

  // @brief data loaded flag
  mutable bool data_loaded_ = false;

  // helper functions
  void _load_data () const;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param uid unique ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
category::impl::impl (std::int64_t uid)
  : uid_ (uid)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set name
// @param name new category name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
category::impl::set_name (const std::string& name)
{
  _load_data ();

  auto db = _get_database ();

  auto stmt = db.new_statement (
                "UPDATE category "
                "SET name = ? "
                "WHERE uid = ?");

  stmt.bind (1, name);
  stmt.bind (2, uid_);
  stmt.execute ();

  name_ = name;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set description
// @param description description
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
category::impl::set_description (const std::string& description)
{
  _load_data ();

  auto db = _get_database ();

  auto stmt = db.new_statement (
                "UPDATE category "
                   "SET description = ? "
                 "WHERE uid = ?");

  stmt.bind (1, description);
  stmt.bind (2, uid_);
  stmt.execute ();

  description_ = description;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set icon data
// @param icon_data icon data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
category::impl::set_icon_data (const mobius::bytearray& icon_data)
{
  _load_data ();

  auto db = _get_database ();

  auto stmt = db.new_statement (
                "UPDATE category "
                   "SET icon_data = ? "
                 "WHERE uid = ?");

  stmt.bind (1, icon_data);
  stmt.bind (2, uid_);
  stmt.execute ();

  icon_data_ = icon_data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get attribute
// @param id attribute ID
// @return attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
attribute
category::impl::get_attribute (const std::string& id) const
{
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "SELECT uid "
                  "FROM attribute "
                 "WHERE category_uid = ? "
                   "AND id = ?");

  stmt.bind (1, uid_);
  stmt.bind (2, id);

  attribute attr;

  if (stmt.fetch_row ())
    {
      std::int64_t uid = stmt.get_column_int64 (0);
      attr = attribute (uid);
    }

  return attr;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief New attribute
// @param id attribute ID
// @return attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
attribute
category::impl::new_attribute (const std::string& id)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // check if attribute already exists
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto attr = get_attribute (id);
  if (attr)
    return attr;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // get next attribute index
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "SELECT MAX (idx) "
                  "FROM attribute "
                 "WHERE category_uid = ?");

  stmt.bind (1, uid_);
  stmt.fetch_row ();
  int index = stmt.get_column_int (0) + 1;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create attribute
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  stmt = db.new_statement (
           "INSERT INTO attribute "
                "VALUES (NULL, ?, ?, NULL, NULL, ?, NULL, ?)");

  stmt.bind (1, uid_);
  stmt.bind (2, id);
  stmt.bind (3, "string");              // default datatype
  stmt.bind (4, index);
  stmt.execute ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // return attribute
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto uid = db.get_last_insert_row_id ();
  return attribute (uid);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove attribute
// @param id attribute ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
category::impl::remove_attribute (const std::string& id)
{
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "DELETE FROM attribute "
                      "WHERE category_uid = ? "
                        "AND id = ?");

  stmt.bind (1, uid_);
  stmt.bind (2, id);
  stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attributes
// @return attributes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <attribute>
category::impl::get_attributes () const
{
  std::vector <attribute> attributes;

  auto db = _get_database ();

  auto stmt = db.new_statement (
                "SELECT uid "
                  "FROM attribute "
                 "WHERE category_uid = ? "
              "ORDER BY idx");

  stmt.bind (1, uid_);

  while (stmt.fetch_row ())
    {
      attributes.emplace_back (stmt.get_column_int64 (0));
    }

  return attributes;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load data on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
category::impl::_load_data () const
{
  if (data_loaded_)
    return;

  // load data
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "SELECT * "
                  "FROM category "
                 "WHERE uid = ?");

  stmt.bind (1, uid_);

  if (stmt.fetch_row ())
    {
      id_ = stmt.get_column_string (1);
      name_ = stmt.get_column_string (2);
      description_ = stmt.get_column_string (3);
      icon_data_ = stmt.get_column_bytearray (4);
    }

  // set data loaded
  data_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param uid row ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
category::category (std::int64_t uid)
  : impl_ (std::make_shared <impl> (uid))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
category::category ()
  : impl_ (std::make_shared <impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if object is valid
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
category::operator bool () const
{
  return impl_->operator bool ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get id
// @return id
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
category::get_id () const
{
  return impl_->get_id ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get name
// @return name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
category::get_name () const
{
  return impl_->get_name ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set name
// @param name name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
category::set_name (const std::string& name)
{
  impl_->set_name (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get description
// @return description
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
category::get_description () const
{
  return impl_->get_description ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set description
// @param description description
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
category::set_description (const std::string& description)
{
  impl_->set_description (description);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get icon data
// @return icon data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
category::get_icon_data () const
{
  return impl_->get_icon_data ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set icon data
// @param icon_data icon data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
category::set_icon_data (const mobius::bytearray& icon_data)
{
  impl_->set_icon_data (icon_data);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get attribute
// @param id attribute ID
// @return attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
attribute
category::get_attribute (const std::string& id) const
{
  return impl_->get_attribute (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create new attribute
// @param id attribute ID
// @return attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
attribute
category::new_attribute (const std::string& id)
{
  return impl_->new_attribute (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief remove attribute
// @param id attribute ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
category::remove_attribute (const std::string& id)
{
  impl_->remove_attribute (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get attributes
// @return attributes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <attribute>
category::get_attributes () const
{
  return impl_->get_attributes ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get category database
// @return Database
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::database::database
get_category_database ()
{
  return _get_database ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new transaction to category database
// @return New database transaction
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::database::transaction
new_category_transaction ()
{
  auto db = _get_database ();
  return db.new_transaction ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get category
// @param id category ID
// @return category
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
category
get_category (const std::string& id)
{
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "SELECT uid "
                  "FROM category "
                 "WHERE id = ?");

  stmt.bind (1, id);

  category cat;

  if (stmt.fetch_row ())
    {
      std::int64_t uid = stmt.get_column_int64 (0);
      cat = category (uid);
    }

  return cat;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief New category
// @param id category ID
// @return category
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
category
new_category (const std::string& id)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // check if category already exists
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto cat = get_category (id);
  if (cat)
    return cat;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create category
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "INSERT INTO category "
                     "VALUES (NULL, ?, NULL, NULL, NULL)");

  stmt.bind (1, id);
  stmt.execute ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // return category
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto uid = db.get_last_insert_row_id ();
  return category (uid);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove category
// @param id category ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
remove_category (const std::string& id)
{
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "DELETE FROM category "
                      "WHERE id = ?");

  stmt.bind (1, id);
  stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get categories
// @return categories
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <category>
get_categories ()
{
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "SELECT uid "
                  "FROM category");

  std::vector <category> categories;

  while (stmt.fetch_row ())
    {
      categories.emplace_back (stmt.get_column_int64 (0));
    }

  return categories;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove all categories
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
remove_categories ()
{
  auto db = _get_database ();
  auto stmt = db.new_statement ("DELETE FROM category");

  stmt.execute ();
}

} // namespace mobius::framework


