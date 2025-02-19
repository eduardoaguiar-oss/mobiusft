#ifndef MOBIUS_REGISTRY_PSSP_DATA2_H
#define MOBIUS_REGISTRY_PSSP_DATA2_H

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
#include <mobius/os/win/registry/registry_key.h>
#include <memory>
#include <string>

namespace mobius
{
namespace os
{
namespace win
{
namespace registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Protected Storage System Provider Data2's keys container
//! \author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class pssp_data2
{
public:
  // constructors
  pssp_data2 ();
  pssp_data2 (pssp_data2&&) = default;
  pssp_data2 (const pssp_data2&) = default;

  // operators
  pssp_data2& operator= (const pssp_data2&) = default;
  pssp_data2& operator= (pssp_data2&&) = default;

  // function prototypes
  void set_key (const std::string&, mobius::bytearray&);
  mobius::bytearray get_key (const std::string&) const;

private:
  //! \brief implementation class forward declaration
  class impl;

  //! \brief implementation pointer
  std::shared_ptr <impl> impl_;
};

} // namespace registry
} // namespace win
} // namespace os
} // namespace mobius

#endif
