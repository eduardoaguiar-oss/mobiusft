#ifndef MOBIUS_FRAMEWORK_MODEL_CASE_HPP
#define MOBIUS_FRAMEWORK_MODEL_CASE_HPP

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
#include <mobius/core/database/connection_pool.hpp>
#include <mobius/core/database/transaction.hpp>
#include <mobius/framework/model/item.hpp>
#include <mobius/framework/model/evidence.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace mobius::framework::model
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Case class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Case
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  Case () noexcept = default;
  Case (const std::string&, std::uint32_t);
  Case (Case&&) noexcept = default;
  Case (const Case&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  Case& operator= (const Case&) noexcept = default;
  Case& operator= (Case&&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Function prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string get_path (const std::string&) const;
  std::string create_path (const std::string&) const;

  item get_item_by_uid (std::int64_t) const;
  item get_root_item () const;
  std::uint32_t get_uid () const;

  mobius::core::database::connection new_connection ();
  mobius::core::database::transaction new_transaction ();
  mobius::core::database::database get_database () const;

  std::vector <evidence> get_passwords () const;
  std::vector <evidence> get_password_hashes () const;

private:
  // @brief implementation class forward declaration
  class impl;

  // @brief implementation pointer
  std::shared_ptr <impl> impl_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool operator== (const Case&, const Case&);
bool operator!= (const Case&, const Case&);
bool operator< (const Case&, const Case&);
bool operator<= (const Case&, const Case&);
bool operator> (const Case&, const Case&);
bool operator>= (const Case&, const Case&);

Case new_case (const std::string& path);
Case open_case (const std::string& path);
void close_case (const Case&);
std::vector <Case> get_cases ();
int get_case_count ();

} // namespace mobius::framework::model

#endif


