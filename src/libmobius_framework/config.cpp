// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008-2026 Eduardo Aguiar
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
#include <mobius/core/application.hpp>
#include <mobius/core/database/database.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/framework/config.hpp>
#include <mutex>

namespace
{
// Mutex to protect write access across threads
static std::mutex mutex_;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get database instance
// @return Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::database::database
_get_database ()
{
    static mobius::core::database::database db;

    if (!db)
    {
        // create database instance
        mobius::core::application app;
        auto path = app.get_config_path ("mobiusft.sqlite");

        db = mobius::core::database::database (path);
        db.execute ("PRAGMA journal_mode=WAL");
        db.execute ("PRAGMA synchronous=FULL");

        // create db tables, if necessary
        db.execute (
            "CREATE TABLE IF NOT EXISTS configuration"
            "(var TEXT PRIMARY KEY,"
            "value BLOB NULL);"
        );
    }

    return db;
}

} // namespace

namespace mobius::framework
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new transaction to config database
// @return New database transaction
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::database::transaction
new_config_transaction ()
{
    auto db = _get_database ();
    return db.new_transaction ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if config value exists
// @param name Value name
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
has_config (const std::string &name)
{
    auto db = _get_database ();

    auto statement = db.new_statement (
        "SELECT 1 "
        "FROM configuration "
        "WHERE var = ?"
    );

    statement.bind (1, name);

    return statement.fetch_row ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get config value
// @param name Value name
// @return Value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
get_config (const std::string &name)
{
    auto db = _get_database ();

    auto statement = db.new_statement (
        "SELECT value "
        "FROM configuration "
        "WHERE var = ?"
    );

    statement.bind (1, name);

    mobius::core::pod::data value;

    if (statement.fetch_row ())
        value = statement.get_column_pod (0);

    return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set config value
// @param name Value name
// @param value Value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_config (const std::string &name, const mobius::core::pod::data &value)
{
    {
        std::lock_guard<std::mutex> lock (mutex_);
        auto db = _get_database ();

        auto statement = db.new_statement (
            "INSERT OR REPLACE INTO configuration "
            "VALUES (?, ?)"
        );

        statement.bind (1, name);
        statement.bind (2, value);
        statement.execute ();
    }

    mobius::core::emit ("config-set", name, value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove config value
// @param name Value name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
remove_config (const std::string &name)
{
    {
        std::lock_guard<std::mutex> lock (mutex_);
        auto db = _get_database ();

        auto statement = db.new_statement (
            "DELETE FROM configuration "
            "WHERE var = ?"
        );

        statement.bind (1, name);
        statement.execute ();
    }

    mobius::core::emit ("config-remove", name);
}

} // namespace mobius::framework
