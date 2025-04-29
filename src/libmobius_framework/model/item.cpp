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
#include <mobius/core/mediator.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/exception.inc>
#include <mobius/framework/category.hpp>
#include <mobius/framework/model/item.hpp>
#include <mobius/framework/model/case.hpp>
#include <mobius/framework/model/event.hpp>
#include <mobius/framework/model/evidence.hpp>
#include <mobius/io/file.h>
#include <mobius/io/folder.h>
#include <cstddef>
#include <stdexcept>
#include <unordered_map>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Rationale:
// 1. Every item attribute is stored as serialized mobius::core::pod::data
// 2. SQLite limits BLOB size to 1 GB (see https://www.sqlite.org/limits.html)
// 3. If serialized data size is greater than ATTRIBUTE_FILE_THRESHOLD,
//    this data is saved in an attribute file at item/data/attrs/<id>.pod and
//    the attribute value is filled with ATTRIBUTE_FILE_ID.
// 3. Conversely, when reading attributes, if an attribute has value equal to
//    ATTRIBUTE_FILE_ID, then its value is read from attribute file.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace mobius::framework::model
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr std::uint64_t ATTRIBUTE_FILE_THRESHOLD = 33554432; // 32 MiB
const mobius::bytearray ATTRIBUTE_FILE_ID = {0xde, 0xea, 0xbe, 0xef, 0xc0, 0xc0, 0xa0, 'M', 'O', 'B', 'I', 'U', 'S', 'P', 'O', 'D'};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Expand value mask, using item attributes
// @param value_mask value mask
// @param item case item
// @return expanded string value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
expand_value_mask (const std::string& value_mask, const mobius::framework::model::item& item)
{
  std::size_t len = value_mask.length ();
  std::size_t pos = 0;
  std::string out;

  while (pos < len)
    {
      auto start_pos = value_mask.find ("${", pos);

      if (start_pos == std::string::npos)
        {
          out += value_mask.substr (pos);
          pos = len;
        }

      else
        {
          out += value_mask.substr (pos, start_pos - pos);
          pos = start_pos;

          auto end_pos = value_mask.find ("}", start_pos + 2);

          if (end_pos == std::string::npos)
            {
              out += value_mask.substr (pos);
              pos = len;
            }

          else
            {
              std::string var = value_mask.substr (start_pos + 2, end_pos - start_pos - 2);
              auto value = item.get_attribute (var);

              if (!value.is_null ())
                out += item.get_attribute (var).to_string ();

              pos = end_pos + 1;
            }
        }
    }

  return out;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief item implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class item::impl
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl (const impl&) = delete;
  impl (impl&&) = delete;
  explicit impl (const Case&);
  impl (const Case&, uid_type);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl& operator= (const impl&) = delete;
  impl& operator= (impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // function prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bool has_attribute (const std::string&) const;
  mobius::core::pod::data get_attribute (const std::string&) const;
  void set_attribute (const std::string&, const mobius::core::pod::data&);
  void remove_attribute (const std::string&);
  std::unordered_map <std::string, mobius::core::pod::data> get_attributes () const;

  bool has_datasource () const;
  mobius::core::datasource::datasource get_datasource () const;
  void set_datasource (const mobius::core::datasource::datasource&);
  void remove_datasource ();

  std::vector <event> get_events () const;
  void add_event (const std::string&);

  item new_child (const std::string&, int);
  void remove ();
  void move (int, const item&);

  std::string get_data_path (const std::string&) const;
  std::string create_data_path (const std::string&) const;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get uid
  // @return uid
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  uid_type
  get_uid () const
  {
    return uid_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get category
  // @return category
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_category () const
  {
    _load_data ();
    return category_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get case
  // @return case
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  Case
  get_case () const
  {
    return case_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get database
  // @return database
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::core::database::database
  get_database () const
  {
    return case_.get_database ();
  }

private:
  // @brief Case object
  Case case_;

  // @brief Unique ID
  uid_type uid_ = -1;

  // @brief Category
  mutable std::string category_;

  // @brief Datasource revision
  mutable std::int64_t datasource_revision_ = 0;

  // @brief Datasource object
  mutable mobius::core::datasource::datasource datasource_;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Helper functions
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void _load_data () const;
  int _reserve_index (int) const;
  void _save_attribute_file (const std::string&, const mobius::bytearray&);
  mobius::bytearray _load_attribute_file (const std::string&) const;
  void _remove_attribute_file (const std::string&);
  mobius::io::file _get_attribute_file (const std::string&) const;
  mobius::io::file _create_attribute_file (const std::string&) const;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param c case object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
item::impl::impl (const Case& c)
  : case_ (c)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param c case object
// @param uid Unique ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
item::impl::impl (const Case& c, uid_type uid)
  : case_ (c),
    uid_ (uid)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get data path
// @param rpath Relative path
// @return Fullpath
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
item::impl::get_data_path (const std::string& rpath) const
{
  if (uid_ == -1)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  char s_uid[32];
  sprintf (s_uid, "%04ld", uid_);

  return case_.get_path ("data/" + std::string (s_uid) + '/' + rpath);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create data path
// @param rpath Relative path
// @return Fullpath
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
item::impl::create_data_path (const std::string& rpath) const
{
  char s_uid[32];
  sprintf (s_uid, "%04ld", get_uid ());

  return case_.create_path ("data/" + std::string (s_uid) + '/' + rpath);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if item has attribute
// @param id Attribute ID
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
item::impl::has_attribute (const std::string& id) const
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Build query
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = get_database ();
  auto stmt = db.new_statement (
                "SELECT 1 "
                  "FROM attribute "
                 "WHERE item_uid = ? "
                   "AND id = ?");

  stmt.bind (1, uid_);
  stmt.bind (2, id);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Return result
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  return bool (stmt.fetch_row ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attribute
// @param id Attribute ID
// @return Attribute value or pod::null if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
item::impl::get_attribute (const std::string& id) const
{
  mobius::core::pod::data value;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Build query
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = get_database ();
  auto stmt = db.new_statement (
                "SELECT value "
                  "FROM attribute "
                 "WHERE item_uid = ? "
                   "AND id = ?");

  stmt.bind (1, uid_);
  stmt.bind (2, id);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Return result
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (stmt.fetch_row ())
    {
      auto bytes = stmt.get_column_bytearray (0);

      if (bytes == ATTRIBUTE_FILE_ID)
        bytes = _load_attribute_file (id);

      value = mobius::core::pod::unserialize (bytes);
    }

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set attribute
// @param id Attribute ID
// @param value Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::impl::set_attribute (const std::string& id, const mobius::core::pod::data& value)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Remove old attribute file, if any
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto f = _get_attribute_file (id);

  if (f.exists ())
    f.remove ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Serialize value and check if size is greater than threshold
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::bytearray bytes = mobius::core::pod::serialize (value);

  if (bytes.size () > ATTRIBUTE_FILE_THRESHOLD)
    {
      _save_attribute_file (id, bytes);
      bytes = ATTRIBUTE_FILE_ID;
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Add to database
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = get_database ();
  mobius::core::database::statement stmt;

  if (has_attribute (id))
    {
      stmt = db.new_statement (
               "UPDATE attribute "
                  "SET value = ? "
                "WHERE item_uid = ? "
                  "AND id = ?");

      stmt.bind (1, bytes);
      stmt.bind (2, uid_);
      stmt.bind (3, id);
    }

  else
    {
      stmt = db.new_statement (
               "INSERT INTO attribute "
                    "VALUES (NULL, ?, ?, ?)");

      stmt.bind (1, uid_);
      stmt.bind (2, id);
      stmt.bind (3, bytes);
    }

  stmt.execute ();

  add_event ("attribute '" + id + "' set");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove attribute
// @param id Attribute ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::impl::remove_attribute (const std::string& id)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Remove from database
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = get_database ();
  auto stmt = db.new_statement (
                "DELETE FROM attribute "
                      "WHERE item_uid = ? "
                        "AND id = ?");

  stmt.bind (1, get_uid ());
  stmt.bind (2, id);
  stmt.execute ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Remove attribute file, if any
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  _remove_attribute_file (id);

  add_event ("attribute '" + id + "' removed");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load attributes on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::unordered_map <std::string, mobius::core::pod::data>
item::impl::get_attributes () const
{
  std::unordered_map <std::string, mobius::core::pod::data> attributes;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Build query
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = get_database ();
  auto stmt = db.new_statement (
                "SELECT id, value "
                  "FROM attribute "
                 "WHERE item_uid = ?");

  stmt.bind (1, uid_);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Fetch rows
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  while (stmt.fetch_row ())
    {
      auto id = stmt.get_column_string (0);
      auto bytes = stmt.get_column_bytearray (1);

      if (bytes == ATTRIBUTE_FILE_ID)
        bytes = _load_attribute_file (id);

      auto value = mobius::core::pod::unserialize (bytes);
      attributes[id] = value;
    }

  return attributes;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if item has datasource
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
item::impl::has_datasource () const
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Build query
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = get_database ();

  auto stmt = db.new_statement (
                "SELECT 1 "
                  "FROM datasource "
                 "WHERE item_uid = ?");

  stmt.bind (1, uid_);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Return result
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  return stmt.fetch_row ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get datasource
// @return Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datasource::datasource
item::impl::get_datasource () const
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Build query
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = get_database ();

  auto stmt = db.new_statement (
                "SELECT revision, state "
                  "FROM datasource "
                 "WHERE item_uid = ?");

  stmt.bind (1, uid_);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Return result
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (stmt.fetch_row ())
    {
      auto revision = stmt.get_column_int64 (0);

      if (revision != datasource_revision_)
        {
          mobius::core::pod::map state (stmt.get_column_pod (1));
          datasource_revision_ = revision;
          datasource_ = mobius::core::datasource::datasource (state);
        }
    }

  return datasource_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set datasource
// @param datasource Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::impl::set_datasource (const mobius::core::datasource::datasource& datasource)
{
  auto db = get_database ();
  auto state = datasource.get_state ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Try to update VFS
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto stmt = db.new_statement (
               "UPDATE datasource "
                  "SET state = ?, "
                      "revision = revision + 1 "
                "WHERE item_uid = ?");

  stmt.bind (1, state);
  stmt.bind (2, uid_);
  stmt.execute ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // If there is no row affected by UPDATE, INSERT data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (!db.get_changes ())
    {
      stmt = db.new_statement (
               "INSERT INTO datasource "
                    "VALUES (?, ?, 1)");

      stmt.bind (1, uid_);
      stmt.bind (2, state);
      stmt.execute ();
    }

  datasource_ = datasource;
  datasource_revision_ = 1;

  add_event ("datasource set");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove datasource
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::impl::remove_datasource ()
{
  auto db = get_database ();

  auto stmt = db.new_statement (
                "DELETE FROM datasource "
                      "WHERE item_uid = ?");

  stmt.bind (1, get_uid ());
  stmt.execute ();

  datasource_ = mobius::core::datasource::datasource ();
  datasource_revision_ = 0;

  add_event ("datasource removed");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new child item
// @param category item's category
// @param idx child position, starting in 1 or -1 for last position
// @return new item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
item
item::impl::new_child (const std::string& category, int idx)
{
  if (uid_ == -1)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  auto db = case_.get_database ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // get item index
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  idx = _reserve_index (idx);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create item
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto stmt = db.new_statement (
                "INSERT INTO item "
                     "VALUES (NULL, ?, ?, ?, DATETIME ('NOW'))");

  stmt.bind (1, uid_);
  stmt.bind (2, idx);
  stmt.bind (3, category);
  stmt.execute ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // return item
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto uid = db.get_last_insert_row_id ();
  auto i = item (case_, uid);

  i.add_event ("item created");

  return i;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::impl::remove ()
{
  if (uid_ == -1)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  if (uid_ == 1)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot remove root item"));

  auto db = case_.get_database ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // get item index
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto stmt = db.new_statement (
                "SELECT idx, parent_uid "
                  "FROM item "
                 "WHERE uid = ?");

  stmt.bind (1, uid_);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // if item exists, delete it
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (stmt.fetch_row ())
    {
      auto idx = stmt.get_column_int (0);
      auto parent_uid = stmt.get_column_int64 (1);

      // delete item
      auto stmt = db.new_statement (
                    "DELETE FROM item "
                          "WHERE uid = ?");

      stmt.bind (1, uid_);
      stmt.execute ();

      // update idx for remaining items
      stmt = db.new_statement (
               "UPDATE item "
                  "SET idx = idx - 1 "
                "WHERE parent_uid = ? "
                  "AND idx > ?");

      stmt.bind (1, parent_uid);
      stmt.bind (2, idx);
      stmt.execute ();
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // remove data folder
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto path = get_data_path ("");
  auto folder = mobius::io::new_folder_by_path (path);

  if (folder.exists ())
    folder.remove ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // reset attributes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  uid_ = -1;
  category_.clear ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Move item
// @param idx position before which the item will be inserted
// @param parent parent item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::impl::move (int idx, const item& parent)
{
  if (uid_ == -1)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  if (uid_ == 1)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot move root item"));

  if (!parent)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("new parent cannot be null"));

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // get current idx and parent
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = case_.get_database ();

  auto stmt = db.new_statement (
                "SELECT idx, parent_uid "
                "FROM item "
                "WHERE uid = ?");

  stmt.bind (1, uid_);

  if (!stmt.fetch_row ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("Item not found"));

  auto old_idx = stmt.get_column_int (0);
  auto old_parent_uid = stmt.get_column_int64 (1);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // open slot into new parent for item
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  idx = parent.impl_->_reserve_index (idx);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // set item's idx and parent
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  stmt = db.new_statement (
           "UPDATE item "
              "SET parent_uid = ?, "
                  "idx = ? "
            "WHERE uid = ?");

  stmt.bind (1, parent.get_uid ());
  stmt.bind (2, idx);
  stmt.bind (3, uid_);
  stmt.execute ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // close slot on old parent
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  stmt = db.new_statement (
           "UPDATE item "
              "SET idx = idx - 1 "
            "WHERE parent_uid = ? "
              "AND idx > ?");

  stmt.bind (1, old_parent_uid);
  stmt.bind (2, old_idx);
  stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add event
// @param text Event text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::impl::add_event (const std::string& text)
{
  auto db = get_database ();

  mobius::core::database::statement stmt = db.new_statement (
               "INSERT INTO event "
                    "VALUES (NULL, ?, ?, ?)");

  stmt.bind (1, uid_);
  stmt.bind (2, mobius::core::datetime::now ());
  stmt.bind (3, text);

  stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get events
// @return Events
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <event>
item::impl::get_events () const
{
  std::vector <event> events;

  auto db = get_database ();

  auto stmt = db.new_statement (
                "SELECT timestamp, text "
                  "FROM event "
                 "WHERE item_uid = ?");

  stmt.bind (1, uid_);

  while (stmt.fetch_row ())
    {
      const auto timestamp = stmt.get_column_datetime (0);
      const auto text = stmt.get_column_string (1);
      events.emplace_back (timestamp, text);
    }

  return events;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load data on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::impl::_load_data () const
{
  auto db = case_.get_database ();

  auto stmt = db.new_statement (
                "SELECT category "
                  "FROM item "
                 "WHERE uid = ?");

  stmt.bind (1, uid_);

  if (stmt.fetch_row ())
    category_ = stmt.get_column_string (0);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reserve one slot for a child at position idx
// @param idx index, starting in 1 or -1 for last position
// @return index reserved
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
item::impl::_reserve_index (int idx) const
{
  if (uid_ == -1)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get last idx
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = case_.get_database ();

  auto stmt = db.new_statement (
                "SELECT MAX (idx) "
                  "FROM item "
                 "WHERE parent_uid = ?");

  stmt.bind (1, uid_);

  int max_idx = 0;

  if (stmt.fetch_row ())
    max_idx = stmt.get_column_int (0);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Reserve slot, if necessary
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (idx == -1)
    idx = max_idx + 1;

  else if (idx < 1 || idx > max_idx + 1)
    throw std::out_of_range (MOBIUS_EXCEPTION_MSG ("Index out of range"));

  else
    {
      auto stmt = db.new_statement (
                    "UPDATE item "
                       "SET idx = idx + 1 "
                     "WHERE parent_uid = ? "
                       "AND idx >= ?");

      stmt.bind (1, uid_);
      stmt.bind (2, idx);
      stmt.execute ();
    }

  return idx;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attribute file
// @param id Attribute ID
// @return File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::file
item::impl::_get_attribute_file (const std::string& id) const
{
  auto path = get_data_path ("attrs/" + id + ".pod");
  return mobius::io::new_file_by_path (path);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create attribute file
// @param id Attribute ID
// @return File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::file
item::impl::_create_attribute_file (const std::string& id) const
{
  auto path = create_data_path ("attrs/" + id + ".pod");
  return mobius::io::new_file_by_path (path);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save attribute file
// @param id Attribute ID
// @param bytes Attribute data serialized
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::impl::_save_attribute_file (
  const std::string& id,
  const mobius::bytearray& bytes)
{
  auto f = _create_attribute_file (id);
  auto writer = f.new_writer ();
  writer.write (bytes);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load attribute file
// @param id Attribute ID
// @return Attribute data serialized
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
item::impl::_load_attribute_file (const std::string& id) const
{
  auto f = _get_attribute_file (id);
  auto reader = f.new_reader ();
  return reader.read (reader.get_size ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove attribute file, if any
// @param id Attribute ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::impl::_remove_attribute_file (const std::string& id)
{
  auto f = _get_attribute_file (id);

  if (f.exists ())
    f.remove ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param c case object
// @param uid Unique ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
item::item (const Case& c, uid_type uid)
  : impl_ (std::make_shared <impl> (c, uid))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get database object
// @return Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::database::database
item::get_database () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  return impl_->get_database ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get uid
// @return uid
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
item::uid_type
item::get_uid () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  return impl_->get_uid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get category
// @return category
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
item::get_category () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  return impl_->get_category ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get case
// @return case
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Case
item::get_case () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  return impl_->get_case ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if attribute exists
// @param id attribute ID
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
item::has_attribute (const std::string& id) const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  return impl_->has_attribute (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attribute value
// @param id attribute ID
// @return attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
item::get_attribute (const std::string& id) const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  return impl_->get_attribute (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set attribute value
// @param id attribute ID
// @param value attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::set_attribute (const std::string& id, const mobius::core::pod::data& value)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  auto old_value = impl_->get_attribute (id);

  if (value != old_value)
    {
      impl_->set_attribute (id, value);
      mobius::core::emit ("attribute-modified", *this, id, old_value, value);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove attribute
// @param id attribute ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::remove_attribute (const std::string& id)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  auto old_value = impl_->get_attribute (id);
  impl_->remove_attribute (id);

  mobius::core::emit ("attribute-removed", *this, id, old_value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attributes
// @return Map containing attributes' IDs and values
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::unordered_map <std::string, mobius::core::pod::data>
item::get_attributes () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  return impl_->get_attributes ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if item has datasource
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
item::has_datasource () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  return impl_->has_datasource ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get datasource
// @return Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datasource::datasource
item::get_datasource () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  return impl_->get_datasource ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set datasource
// @param datasource Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::set_datasource (const mobius::core::datasource::datasource& datasource)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  impl_->set_datasource (datasource);

  remove_ants ();

  mobius::core::emit ("datasource-modified", *this, datasource);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove datasource
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::remove_datasource ()
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  impl_->remove_datasource ();

  remove_ants ();

  mobius::core::emit ("datasource-removed", *this);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of children
// @return Number of children
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
item::get_child_count () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // run query
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = impl_->get_database ();

  auto stmt = db.new_statement (
                "SELECT count (*) "
                  "FROM item "
                 "WHERE parent_uid = ?");

  stmt.bind (1, impl_->get_uid ());

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // get result
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  int count = 0;

  if (stmt.fetch_row ())
    count = stmt.get_column_int (0);

  return count;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get children items
// @return children
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <item>
item::get_children () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // run query
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = impl_->get_database ();

  auto stmt = db.new_statement (
                "SELECT uid "
                  "FROM item "
                 "WHERE parent_uid = ? "
              "ORDER BY idx");

  stmt.bind (1, impl_->get_uid ());

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // fill vector
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto c = impl_->get_case ();
  std::vector <item> items;

  while (stmt.fetch_row ())
    {
      auto uid = stmt.get_column_int64 (0);
      items.emplace_back (c, uid);
    }

  return items;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get parent item
// @return parent, if exists
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
item
item::get_parent () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // get parent_uid
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto db = impl_->get_database ();

  auto stmt = db.new_statement (
                "SELECT parent_uid "
                  "FROM item "
                 "WHERE uid = ?");

  stmt.bind (1, impl_->get_uid ());

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // if item exists, get parent
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  item parent;

  if (stmt.fetch_row ())
    {
      if (!stmt.is_column_null (0))
        {
          auto c = impl_->get_case ();
          parent = c.get_item_by_uid (stmt.get_column_int64 (0));
        }
    }

  return parent;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new child item
// @param category item's category
// @param idx child position, starting in 1 or -1 for last position
// @return new item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
item
item::new_child (const std::string& category, int idx)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  return impl_->new_child (category, idx);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::remove ()
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  impl_->remove ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Move item
// @param idx new index
// @param parent parent item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::move (int idx, const item& parent)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  impl_->move (idx, parent);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Expand ${} masks into attribute values
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::expand_masks ()
{
  auto category = mobius::framework::get_category (get_category ());

  if (!category)
    return;

  for (const auto& attr : category.get_attributes ())
    {
      const std::string value_mask = attr.get_value_mask ();

      if (!value_mask.empty ())
        {
          auto old_value = get_attribute (attr.get_id ());
          std::string value = expand_value_mask (value_mask, *this);
          set_attribute (attr.get_id (), value);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get data path
// @param rpath Relative path
// @return Fullpath
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
item::get_data_path (const std::string& rpath) const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  return impl_->get_data_path (rpath);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create data path
// @param rpath relative path
// @return fullpath
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
item::create_data_path (const std::string& rpath) const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  return impl_->create_data_path (rpath);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new evidence
// @param type Evidence type
// @return Evidence
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
evidence
item::new_evidence (const std::string& type)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  auto db = get_database ();

  auto stmt = db.new_statement (
                "INSERT INTO evidence "
                     "VALUES (NULL, ?, ?)");

  stmt.bind (1, get_uid ());
  stmt.bind (2, type);
  stmt.execute ();

  auto uid = db.get_last_insert_row_id ();
  return evidence (*this, uid, type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add evidence
// @param e Evidence object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
evidence
item::add_evidence (const evidence& e)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  auto evidence = new_evidence (e.get_type ());
  evidence.set_attributes (e.get_attributes ());
  evidence.set_tags (e.get_tags ());

  return evidence;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get evidences for a given type
// @param type Evidence type
// @return Vector of evidences of that type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <evidence>
item::get_evidences (const std::string& type) const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  auto db = get_database ();

  auto stmt = db.new_statement (
                  "SELECT uid "
                    "FROM evidence "
                   "WHERE item_uid = ? "
                     "AND type = ?");

  stmt.bind (1, get_uid ());
  stmt.bind (2, type);

  std::vector <evidence> evidences;

  while (stmt.fetch_row ())
    {
      auto uid = stmt.get_column_int64 (0);
      evidences.emplace_back (*this, uid, type);
    }

  return evidences;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove evidences
// @param type Evidence type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::remove_evidences (const std::string& type)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  auto db = get_database ();

  auto stmt = db.new_statement (
                "DELETE FROM evidence "
                      "WHERE item_uid = ? "
                        "AND type = ?");

  stmt.bind (1, get_uid ());
  stmt.bind (2, type);
  stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove all evidences
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::remove_evidences ()
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  auto db = get_database ();

  auto stmt = db.new_statement (
                "DELETE FROM evidence "
                      "WHERE item_uid = ?");

  stmt.bind (1, get_uid ());
  stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Count evidences
// @param type Evidence type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
item::count_evidences (const std::string& type) const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  auto db = get_database ();

  auto stmt = db.new_statement (
                "SELECT count (*) "
                  "FROM evidence "
                 "WHERE item_uid = ? "
                   "AND type = ?");

  stmt.bind (1, get_uid ());
  stmt.bind (2, type);

  std::int64_t count = 0;

  if (stmt.fetch_row ())
    count = stmt.get_column_int64 (0);

  return count;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Count all evidences
// @return map of type -> count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::unordered_map <std::string, std::int64_t>
item::count_evidences () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  auto db = get_database ();

  auto stmt = db.new_statement (
                "SELECT type, count (*) "
                  "FROM evidence "
                 "WHERE item_uid = ? "
              "GROUP BY type "
              "ORDER BY type");

  stmt.bind (1, get_uid ());

  std::unordered_map <std::string, std::int64_t> counters;

  while (stmt.fetch_row ())
    {
      auto type = stmt.get_column_string (0);
      auto count = stmt.get_column_int64 (1);
      counters[type] = count;
    }

  return counters;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add event
// @param text Event text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::add_event (const std::string& text)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  impl_->add_event (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get events
// @return Events
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <event>
item::get_events () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid item"));

  return impl_->get_events ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new connection
// @return New connection object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::database::connection
item::new_connection ()
{
  auto c = get_case ();
  return c.new_connection ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new transaction
// @return New transaction object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::database::transaction
item::new_transaction ()
{
  auto c = get_case ();
  return c.new_transaction ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief check if two items are equal
// @param a item a
// @param b item b
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator== (const item& a, const item& b)
{
  return a.get_uid () == b.get_uid () && a.get_case () == b.get_case ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief check if two items are different
// @param a item a
// @param b item b
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator!= (const item& a, const item& b)
{
  return a.get_uid () != b.get_uid () || a.get_case () != b.get_case ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Operator <
// @param a item a
// @param b item b
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator< (const item& a, const item& b)
{
  return a.get_case () == b.get_case () && a.get_uid () < b.get_uid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Operator <=
// @param a item a
// @param b item b
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator<= (const item& a, const item& b)
{
  return a.get_case () == b.get_case () && a.get_uid () <= b.get_uid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Operator >
// @param a item a
// @param b item b
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator> (const item& a, const item& b)
{
  return a.get_case () == b.get_case () && a.get_uid () > b.get_uid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Operator >=
// @param a item a
// @param b item b
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator>= (const item& a, const item& b)
{
  return a.get_case () == b.get_case () && a.get_uid () >= b.get_uid ();
}

} // namespace mobius::framework::model

namespace std
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// std::hash specialization for mobius::model::item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
size_t
std::hash <mobius::framework::model::item>::operator () (const mobius::framework::model::item& item) const
{
  constexpr std::uint64_t MUL = 0x9e3779b97f4a7c15ull; // 2^64/phi
  auto item_uid = item.get_uid ();
  auto case_uid = item.get_case ().get_uid ();

  // Combine item UID and case UID
  std::uint64_t seed = case_uid * MUL;
  seed ^= (item_uid >> 32) * MUL + (seed << 6) + (seed >> 2);
  seed ^= (item_uid & 0xffffffff) * MUL + (seed << 6) + (seed >> 2);

  return seed;
}

} // namespace std
