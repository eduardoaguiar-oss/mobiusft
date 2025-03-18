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
#include "evidence.h"
#include <mobius/model/case.h>
#include <mobius/exception.inc>
#include <algorithm>
#include <stdexcept>

namespace mobius::model
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief evidence implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class evidence::impl
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl (const impl&) = delete;
  impl (impl&&) = delete;
  impl (item, std::int64_t, const std::string&);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl& operator= (const impl&) = delete;
  impl& operator= (impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  //! \brief Get item
  //! \return Item
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  item
  get_item () const
  {
    return item_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  //! \brief Get uid
  //! \return Uid
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::int64_t
  get_uid () const
  {
    return uid_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  //! \brief Get type
  //! \return Type
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_type () const
  {
    return type_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bool has_attribute (const std::string&) const;
  mobius::pod::data get_attribute (const std::string&) const;
  void set_attribute (const std::string&, const mobius::pod::data&);
  void set_attributes (const std::map <std::string, mobius::pod::data>&);
  void remove_attribute (const std::string&);
  std::map <std::string, mobius::pod::data> get_attributes () const;

  bool has_tag (const std::string&) const;
  void set_tag (const std::string&);
  void set_tags (const std::set <std::string>&);
  void reset_tag (const std::string&);
  std::set <std::string> get_tags () const;

  void add_source (evidence::source_type, std::uint64_t, const std::string&);
  std::vector <evidence::source> get_sources () const;

private:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  //! \brief Get database
  //! \return database
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::database::database
  _get_database () const
  {
    return case_.get_database ();
  }

private:
  //! \brief Case object
  Case case_;

  //! \brief Case item
  item item_;

  //! \brief UID
  const std::int64_t uid_ = -1;

  //! \brief Evidence type
  const std::string type_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
//! \param i Item object
//! \param uid Evidence UID
//! \param type Evidence type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
evidence::impl::impl (item i, std::int64_t uid, const std::string& type)
 : case_ (i.get_case ()),
   item_ (i),
   uid_ (uid),
   type_ (type)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check if attribute exists
//! \param id Attribute ID
//! \return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
evidence::impl::has_attribute (const std::string& id) const
{
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "SELECT * "
                  "FROM evidence_attribute "
                 "WHERE evidence_uid = ? "
                   "AND id = ?");

  stmt.bind (1, get_uid ());
  stmt.bind (2, id);

  return bool (stmt.fetch_row ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get attribute value
//! \param id Attribute ID
//! \return Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::pod::data
evidence::impl::get_attribute (const std::string& id) const
{
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "SELECT value "
                  "FROM evidence_attribute "
                 "WHERE evidence_uid = ? "
                   "AND id = ?");

  stmt.bind (1, get_uid ());
  stmt.bind (2, id);

  if (!stmt.fetch_row ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("attribute '" + id + "' not found"));

  return stmt.get_column_pod (0);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set attribute value
//! \param id Attribute ID
//! \param value Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence::impl::set_attribute (const std::string& id, const mobius::pod::data& value)
{
  auto db = _get_database ();
  mobius::database::statement stmt;

  if (has_attribute (id))
    {
      stmt = db.new_statement (
               "UPDATE evidence_attribute "
                  "SET value = ? "
                "WHERE evidence_uid = ? "
                  "AND id = ?");

      stmt.bind (1, value);
      stmt.bind (2, get_uid ());
      stmt.bind (3, id);
    }

  else
    {
      stmt = db.new_statement (
               "INSERT INTO evidence_attribute "
                    "VALUES (NULL, ?, ?, ?)");

      stmt.bind (1, get_uid ());
      stmt.bind (2, id);
      stmt.bind (3, value);
    }

  stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Remove attribute
//! \param id Attribute ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence::impl::remove_attribute (const std::string& id)
{
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "DELETE FROM evidence_attribute "
                      "WHERE evidence_uid = ? "
                        "AND id = ?");

  stmt.bind (1, get_uid ());
  stmt.bind (2, id);
  stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get attributes
//! \return Map with ID -> value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::map <std::string, mobius::pod::data>
evidence::impl::get_attributes () const
{
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "SELECT id, value "
                  "FROM evidence_attribute "
                 "WHERE evidence_uid = ?");

  stmt.bind (1, get_uid ());
  std::map <std::string, mobius::pod::data> values;

  while (stmt.fetch_row ())
    {
      auto id = stmt.get_column_string (0);
      auto value = stmt.get_column_pod (1);
      values[id] = value;
    }

  return values;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check if evidence has a given tag
//! \param name Tag name
//! \return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
evidence::impl::has_tag (const std::string& name) const
{
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "SELECT * "
                  "FROM evidence_tag "
                 "WHERE evidence_uid = ? "
                   "AND name = ?");

  stmt.bind (1, get_uid ());
  stmt.bind (2, name);

  return bool (stmt.fetch_row ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set tag
//! \param name Tag name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence::impl::set_tag (const std::string& name)
{
  auto db = _get_database ();

  mobius::database::statement stmt = db.new_statement (
           "INSERT OR IGNORE INTO evidence_tag "
                          "VALUES (NULL, ?, ?)");

  stmt.bind (1, get_uid ());
  stmt.bind (2, name);
  stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set tags
//! \param tags Tags
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence::impl::set_tags (const std::set<std::string>& tags)
{
  std::for_each (
     tags.cbegin (),
     tags.end (),
     [this](const std::string& tag){ set_tag (tag); }
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Reset tag
//! \param name Tag name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence::impl::reset_tag (const std::string& name)
{
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "DELETE FROM evidence_tag "
                      "WHERE evidence_uid = ? "
                        "AND name = ?");

  stmt.bind (1, get_uid ());
  stmt.bind (2, name);
  stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get tags
//! \return Set of evidence tags
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::set <std::string>
evidence::impl::get_tags () const
{
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "SELECT name "
                  "FROM evidence_tag "
                 "WHERE evidence_uid = ?");

  stmt.bind (1, get_uid ());

  std::set <std::string> tags;

  while (stmt.fetch_row ())
    {
      auto name = stmt.get_column_string (0);
      tags.insert (name);
    }

  return tags;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Add evidence source
//! \param type Source type
//! \param uid Source UID
//! \param description Source description
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence::impl::add_source (
  evidence::source_type type,
  std::uint64_t uid,
  const std::string& description)
{
  auto db = _get_database ();

  mobius::database::statement stmt = db.new_statement (
           "INSERT INTO evidence_source "
                "VALUES (NULL, ?, ?, ?, ?)");

  stmt.bind (1, get_uid ());
  stmt.bind (2, static_cast <int64_t> (type));
  stmt.bind (3, static_cast <int64_t> (uid));
  stmt.bind (4, description);

  stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get sources
//! \return List of sources
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <evidence::source>
evidence::impl::get_sources () const
{
  auto db = _get_database ();

  auto stmt = db.new_statement (
                "SELECT type, source_uid, description "
                  "FROM evidence_source "
                 "WHERE evidence_uid = ?");

  stmt.bind (1, get_uid ());

  std::vector <evidence::source> sources;

  while (stmt.fetch_row ())
    {
      source s;
      s.type = static_cast <evidence::source_type> (stmt.get_column_int64 (0));
      s.source_uid = stmt.get_column_int64 (1);
      s.description = stmt.get_column_string (2);

      sources.push_back (s);
    }

  return sources;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
//! \param i Item object
//! \param uid Evidence UID
//! \param type Evidence type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
evidence::evidence (item i, std::int64_t uid, const std::string& type)
  : impl_ (std::make_shared <impl> (i, uid, type))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get item
//! \return Item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
item
evidence::get_item () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  return impl_->get_item ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get uid
//! \return Uid
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
evidence::get_uid () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  return impl_->get_uid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get type
//! \return Type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
evidence::get_type () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  return impl_->get_type ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check if attribute exists
//! \param id Attribute ID
//! \return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
evidence::has_attribute (const std::string& id) const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  return impl_->has_attribute (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get attribute value
//! \param id Attribute ID
//! \return Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::pod::data
evidence::get_attribute (const std::string& id) const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  return impl_->get_attribute (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set attribute value
//! \param id Attribute ID
//! \param value Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence::set_attribute (const std::string& id, const mobius::pod::data& value)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  impl_->set_attribute (id, value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set attributes
//! \param attributes Map of attr_id -> attr_value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence::set_attributes (const std::map <std::string, mobius::pod::data>& attributes)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  std::for_each (
      attributes.cbegin (),
      attributes.cend (),
      [this](const auto& it){ set_attribute (it.first, it.second); }
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Remove attribute
//! \param id Attribute ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence::remove_attribute (const std::string& id)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  impl_->remove_attribute (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get attributes
//! \return Map with ID -> value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::map <std::string, mobius::pod::data>
evidence::get_attributes () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  return impl_->get_attributes ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check if evidence has a given tag
//! \param name Tag name
//! \return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
evidence::has_tag (const std::string& name) const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  return impl_->has_tag (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set tag
//! \param name Tag name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence::set_tag (const std::string& name)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  impl_->set_tag (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set tags
//! \param tags Tags
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence::set_tags (const std::set<std::string>& tags)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  impl_->set_tags (tags);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Reset tag
//! \param name Tag name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence::reset_tag (const std::string& name)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  impl_->reset_tag (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get tags
//! \return Set of evidence tags
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::set <std::string>
evidence::get_tags () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  return impl_->get_tags ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Add evidence source
//! \param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence::add_source (const mobius::io::file& f)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  if (f)
    {
      impl_->add_source (
         source_type::file,
         0,
         f.get_path () + " (i-node: " + std::to_string (f.get_inode ()) + ')'
      );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Add evidence source
//! \param e Evidence object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence::add_source (const evidence& e)
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  impl_->add_source (
     source_type::evidence,
     e.get_uid (),
     e.get_type () + " evidence"
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get sources
//! \return List of sources
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <evidence::source>
evidence::get_sources () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("evidence object is null"));

  return impl_->get_sources ();
}

} // namespace mobius::model
