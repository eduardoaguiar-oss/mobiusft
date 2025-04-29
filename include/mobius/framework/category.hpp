#ifndef MOBIUS_FRAMEWORK_CATEGORY_HPP
#define MOBIUS_FRAMEWORK_CATEGORY_HPP

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
#include <mobius/bytearray.h>
#include <mobius/core/database/database.hpp>
#include <mobius/core/database/transaction.hpp>
#include <mobius/framework/attribute.hpp>
#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace mobius::framework
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief General category class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class category
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  category ();
  explicit category (std::int64_t);
  category (category&&) noexcept = default;
  category (const category&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  category& operator= (const category&) noexcept = default;
  category& operator= (category&&) noexcept = default;
  operator bool () const;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // function prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string get_id () const;
  std::string get_name () const;
  std::string get_description () const;
  mobius::bytearray get_icon_data () const;

  void set_name (const std::string&);
  void set_description (const std::string&);
  void set_icon_data (const mobius::bytearray&);

  attribute get_attribute (const std::string&) const;
  attribute new_attribute (const std::string&);
  void remove_attribute (const std::string&);
  std::vector <attribute> get_attributes () const;

private:
  // @brief implementation class forward declaration
  class impl;

  // @brief implementation pointer
  std::shared_ptr <impl> impl_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Function prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
category get_category (const std::string&);
category new_category (const std::string&);
void remove_category (const std::string&);
std::vector <category> get_categories ();
void remove_categories ();
mobius::core::database::database get_category_database ();
mobius::core::database::transaction new_category_transaction ();

} // namespace mobius::framework

#endif


