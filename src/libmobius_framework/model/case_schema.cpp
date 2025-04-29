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
#include <mobius/core/database/database.hpp>
#include <mobius/core/database/meta_table.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/string_functions.h>

namespace mobius::framework::model
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr int SCHEMA_VERSION = 13;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// History
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
 * Version      Modifications
 * ---------------------------------------------------------------------------
 *       4      cookie.value modified from TEXT to BLOB
 *
 *       5      New column item.metadata (BLOB)
 *              New table text_search
 *
 *       6      New table evidence
 *              New table evidence_attribute
 *              Table text_search removed
 *              Table cookie removed
 *              Table password removed
 *              Table password_attribute removed
 *              Table password_hash removed
 *              Table password_hash_attribute removed
 *
 *      7       Remove column item.metadata
 *              attribute.value modified from TEXT to BLOB
 *
 *      8       Item attributes converted from bytes to string (Python 3)
 *
 *      9       Removed tables 'application' and 'profile'
 *
 *      10      New table evidence_tag
 *              New table datasource
 *
 *      11      Datasource.revision column
 *
 *      12      New table evidence_source
 *
 *      13      New table event
 *
 * Create new table                     Rename old table
 * Copy data                            Create new table
 * Drop old table                       Copy data
 * Rename new into old                  Drop old table
 * Correct ↑                            Incorrect ↑
 */

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Upgrade schema to v11
// @param db Case database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_case_schema_upgrade_v11 (mobius::core::database::database db)
{
  db.execute ("ALTER TABLE datasource "
               "ADD COLUMN revision INTEGER NOT NULL DEFAULT 1");
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create database tables and indexes
// @param db Case database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
case_schema (mobius::core::database::database db)
{
  db.execute ("PRAGMA foreign_keys = OFF;");
  auto transaction = db.new_transaction ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create table 'case'
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  db.execute (
    "CREATE TABLE IF NOT EXISTS 'case' ("
                "uid INTEGER PRIMARY KEY,"
      "creation_time DATETIME NOT NULL);"
    );

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create table 'item'
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  db.execute (
    "CREATE TABLE IF NOT EXISTS item ("
                "uid INTEGER PRIMARY KEY AUTOINCREMENT,"
         "parent_uid INTEGER,"
                "idx INTEGER NOT NULL,"
           "category TEXT NOT NULL,"
      "creation_time DATETIME NOT NULL,"
    "FOREIGN KEY (parent_uid) REFERENCES item (uid) ON DELETE CASCADE);"
    );

  db.execute (
    "CREATE INDEX IF NOT EXISTS idx_item "
           "ON item (parent_uid)");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create table 'attribute'
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  db.execute (
    "CREATE TABLE IF NOT EXISTS attribute ("
           "uid INTEGER PRIMARY KEY AUTOINCREMENT,"
      "item_uid INTEGER,"
            "id TEXT NOT NULL,"
         "value BLOB,"
    "FOREIGN KEY (item_uid) REFERENCES item (uid) ON DELETE CASCADE);"
    );

  db.execute (
    "CREATE UNIQUE INDEX IF NOT EXISTS idx_attribute "
           "ON attribute (item_uid, id)");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create table 'datasource'
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  db.execute (
    "CREATE TABLE IF NOT EXISTS datasource ("
      "item_uid INTEGER PRIMARY KEY NOT NULL,"
      "revision INTEGER NOT NULL,"
         "state BLOB NOT NULL,"
    "FOREIGN KEY (item_uid) REFERENCES item (uid) ON DELETE CASCADE);"
    );

  db.execute (
    "CREATE UNIQUE INDEX IF NOT EXISTS idx_datasource "
           "ON datasource (item_uid)");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create table 'ant'
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  db.execute (
    "CREATE TABLE IF NOT EXISTS ant ("
                      "uid INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "item_uid INTEGER,"
                       "id TEXT NOT NULL,"
                     "name TEXT,"
                  "version TEXT,"
      "last_execution_time DATETIME,"
    "FOREIGN KEY (item_uid) REFERENCES item (uid) ON DELETE CASCADE);"
    );

  db.execute (
    "CREATE UNIQUE INDEX IF NOT EXISTS idx_ant "
           "ON ant (item_uid, id)");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create table 'evidence'
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  db.execute (
    "CREATE TABLE IF NOT EXISTS evidence ("
                      "uid INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "item_uid INTEGER,"
                     "type TEXT NOT NULL,"
    "FOREIGN KEY (item_uid) REFERENCES item (uid) ON DELETE CASCADE);"
    );

  db.execute (
    "CREATE INDEX IF NOT EXISTS idx_evidence "
           "ON evidence (item_uid, type)");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create table 'evidence_attribute'
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  db.execute (
    "CREATE TABLE IF NOT EXISTS evidence_attribute ("
              "uid INTEGER PRIMARY KEY AUTOINCREMENT,"
     "evidence_uid INTEGER,"
               "id TEXT NOT NULL,"
            "value BLOB,"
       "FOREIGN KEY (evidence_uid) REFERENCES evidence (uid) ON DELETE CASCADE);"
    );

  db.execute (
    "CREATE UNIQUE INDEX IF NOT EXISTS idx_evidence_attribute "
           "ON evidence_attribute (evidence_uid, id)");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create table 'evidence_source'
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  db.execute (
    "CREATE TABLE IF NOT EXISTS evidence_source ("
              "uid INTEGER PRIMARY KEY AUTOINCREMENT,"
     "evidence_uid INTEGER NOT NULL,"
             "type INTEGER NOT NULL,"
       "source_uid INTEGER NOT NULL,"
      "description TEXT NOT NULL,"
       "FOREIGN KEY (evidence_uid) REFERENCES evidence (uid) ON DELETE CASCADE);"
    );

  db.execute (
    "CREATE INDEX IF NOT EXISTS idx_evidence_source "
           "ON evidence_source (evidence_uid)");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create table 'evidence_tag'
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  db.execute (
    "CREATE TABLE IF NOT EXISTS evidence_tag ("
              "uid INTEGER PRIMARY KEY AUTOINCREMENT,"
     "evidence_uid INTEGER,"
             "name TEXT NOT NULL,"
       "FOREIGN KEY (evidence_uid) REFERENCES evidence (uid) ON DELETE CASCADE);"
    );

  db.execute (
    "CREATE UNIQUE INDEX IF NOT EXISTS idx_evidence_tag "
           "ON evidence_tag (evidence_uid, name)");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create table 'event'
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  db.execute (
    "CREATE TABLE IF NOT EXISTS event ("
                      "uid INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "item_uid INTEGER,"
                "timestamp DATETIME NOT NULL,"
                     "text TEXT NOT NULL,"
    "FOREIGN KEY (item_uid) REFERENCES item (uid) ON DELETE CASCADE);"
    );

  db.execute (
    "CREATE INDEX IF NOT EXISTS idx_event "
           "ON event (item_uid)");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // upgrade database, if necessary
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::core::database::meta_table meta_table (db);
  int version = meta_table.get_version ();

  if (version == 0)
    ; // newly created database

  else if (version > 8 && version < 11)
    _case_schema_upgrade_v11 (db);

  if (version < SCHEMA_VERSION)
    meta_table.set_version (SCHEMA_VERSION);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // commit changes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  transaction.commit ();
  db.execute ("PRAGMA foreign_keys = ON;");
}

} // namespace mobius::framework::model
