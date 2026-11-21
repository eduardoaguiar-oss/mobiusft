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
#include <mobius/core/database/db_execute_guard.hpp>
#include <mobius/core/log.hpp>

namespace mobius::core::database
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param db Reference to the database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
db_execute_guard::db_execute_guard (const mobius::core::database::database &db)
    : db_ (db)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// Executes all registered commands in reverse order
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
db_execute_guard::~db_execute_guard ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    while (!commands_.empty ())
    {
        try
        {
            db_.execute (commands_.top ());
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, e.what ());
        }
        catch (...)
        {
        }

        commands_.pop ();
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add a command to be executed on scope exit
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
db_execute_guard::add (const std::string &command)
{
    commands_.push (command);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set pragma now; restore its previous value on scope exit
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
db_execute_guard::set_pragma (const std::string &schema, const std::string &name, const std::string &value)
{
    auto pragma = "PRAGMA " + schema + "." + name;

    auto stmt = db_.new_statement (pragma);
    auto old_value = stmt.fetch_row () ? stmt.get_column_string (0) : std::string ();

    db_.execute (pragma + " = " + value);

    if (!old_value.empty ())
        commands_.push (pragma + " = " + old_value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Attach database now; detach on scope exit
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
db_execute_guard::attach (const std::string &path, const std::string &alias)
{
    auto stmt = db_.new_statement ("ATTACH DATABASE ? AS " + alias);
    stmt.bind (1, path);
    stmt.execute ();

    commands_.push ("DETACH DATABASE " + alias);
}

} // namespace mobius::core::database