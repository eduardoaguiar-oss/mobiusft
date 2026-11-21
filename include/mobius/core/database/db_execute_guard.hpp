#ifndef MOBIUS_CORE_DATABASE_DB_EXECUTE_GUARD_HPP
#define MOBIUS_CORE_DATABASE_DB_EXECUTE_GUARD_HPP

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
#include <mobius/core/database/database.hpp>
#include <stack>
#include <string>

namespace mobius::core::database
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Executes registered SQL commands, in reverse order, on scope exit
// @note Declare before any transaction object, so commands run after the
//       transaction has committed or rolled back (DETACH and journal_mode
//       cannot run inside a transaction)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class db_execute_guard
{
  public:

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors and destructor
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit db_execute_guard (const mobius::core::database::database&);
    db_execute_guard (const db_execute_guard &) = delete;
    db_execute_guard &operator= (const db_execute_guard &) = delete;
    ~db_execute_guard ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void set_pragma (const std::string &, const std::string &, const std::string &);
    void attach (const std::string &, const std::string &);
    void add (const std::string &);

  private:
    // @brief Database object
    mobius::core::database::database db_;

    // @brief Stack of commands to execute on scope exit
    std::stack<std::string> commands_;
};

} // namespace mobius::core::database

#endif // MOBIUS_CORE_DATABASE_DB_EXECUTE_GUARD_HPP