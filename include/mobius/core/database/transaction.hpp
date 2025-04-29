#ifndef MOBIUS_CORE_DATABASE_TRANSACTION_HPP
#define MOBIUS_CORE_DATABASE_TRANSACTION_HPP

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
#include <memory>

namespace mobius::core::database
{
class database;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Nested transaction control class
// @author Eduardo Aguiar
//
// This class is implemented with the following features:
//
// 1. Transactions can be nested:
//    auto transaction1 = db.new_transaction ();
//    db.execute (cmd1);
//    ...
//    auto transaction2 = db.new_transaction ();
//    db.execute (cmd2);
//    transaction2.commit (); // does not commit. Only ends transaction2
//
//    db.execute (cmd3);
//    transaction1.commit (); // It commits cmd1, cmd2 and cmd3
//
// 2. Consecutive calls to commit or rollback for the same transaction are ignored:
//
//    auto transaction = db.transaction ();
//    db.execute (cmd1);
//
//    db.commit ();
//
//    db.rollback ();  // ignored
//    db.commit ();    // ignored
//
// 3. Non-commited transactions are rolled-back when transaction is destroyed:
//
//    {
//      auto transaction = db.transaction ();
//      db.execute (cmd1);
//
//      // transaction is rolled-back
//    }
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class transaction
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  explicit transaction (const database&, bool);
  transaction (const transaction&) noexcept = default;
  transaction (transaction&&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  transaction& operator= (const transaction&) noexcept = default;
  transaction& operator= (transaction&&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void commit ();
  void rollback ();

private:
  // @brief Forward declaration
  class impl;

  // @brief Pointer to implementation class
  std::shared_ptr <impl> impl_;
};

} // namespace mobius::core::database

#endif


