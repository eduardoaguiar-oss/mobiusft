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
#include <cstdint>
#include <string>
#include <mobius/core/mediator.hpp>
#include <mobius/datetime/datetime.h>
#include <mobius/datetime/timedelta.h>
#include <mobius/framework/model/item.hpp>
#include <mobius/pod/data.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Extension data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C"
{
const char *EXTENSION_ID = "date-code";
const char *EXTENSION_NAME = "Date Code";
const char *EXTENSION_VERSION = "1.0";
const char *EXTENSION_AUTHORS = "Eduardo Aguiar";
const char *EXTENSION_DESCRIPTION = "Seagate date-code automatic decoding";
} // extern "C"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Internal data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
namespace
{

//! \brief Subscription ID to event "attribute-modified"
static std::uint64_t subscription_id_ = -1;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Callback to event "attribute-modified"
//! \param item Case item
//! \param attr_id Attribute ID
//! \param old_value Old value
//! \param new_value New value
//! \see https://www.digital-detective.net/data-recovery-documents/SeagateDateCode_NoteTechnique03-v1.01.pdf
//! \see https://www.os2museum.com/wp/decoding-seagate-date-codes/
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_callback
(
  mobius::framework::model::item item,
  const std::string& attr_id,
  const mobius::pod::data&,
  const mobius::pod::data& new_value
)
{
  // check if attribute being modified is "manufacturing_date"
  if (attr_id != "manufacturing_date" || !new_value.is_string ())
    return;

  // check if value has length 4 or 5
  std::string value = std::string (new_value);

  if (value.length () < 4 || value.length () > 5)
    return;

  // decode year, week and day  
  int Y = std::stoi (value.substr (0, 2));
  int W, D;

  if (value.length () == 4)
    {
      W = std::stoi (value.substr (2, 1));
      D = std::stoi (value.substr (3, 1));
    }
  else
    {
      W = std::stoi (value.substr (2, 2));
      D = std::stoi (value.substr (4, 1));
    }

  // calculate full year
  if (Y > 80)
    Y += 1899;
  
  else
    Y += 1999;

  // fiscal year begins at first saturday of July of the previous year
  mobius::datetime::date d (Y, 7, 1);

  int days = (W - 1) * 7 + D - 1;

  if (d.get_weekday () < 6)
    days += 5 - d.get_weekday ();
  
  else
    days += 6;

  // add days to first day of the fiscal year
  mobius::datetime::timedelta td (0, days);
  d = d + td;

  // set manufacturing date
  item.set_attribute ("manufacturing_date", to_string (d));
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Start extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
start ()
{
  subscription_id_ = mobius::core::subscribe ("attribute-modified", _callback);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Stop extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
stop ()
{
  mobius::core::unsubscribe (subscription_id_);
}
