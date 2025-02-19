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
#include "datasource_impl.h"
#include <mobius/io/file.h>

namespace mobius::datasource::ufdr
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
//! \param state Object state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource_impl::datasource_impl (const mobius::pod::map& state)
{
  auto type = state.get<std::string> ("type");

  if (type != get_type ())
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid datasource type: " + type));

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get UFDR state
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto ufdr_state = state.get<mobius::pod::map> ("state");
  url_ = ufdr_state.get<std::string> ("url");
  file_info_ = ufdr_state.get<mobius::pod::map> ("file_info");
  case_info_ = ufdr_state.get<mobius::pod::map> ("case_info");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get metadata
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto extractions = ufdr_state.get<std::vector<mobius::pod::data>>("extractions");

  for(const auto& extraction_state : extractions)
    extractions_.emplace_back(mobius::pod::map(extraction_state));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
//! \param url UFDR file URL
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource_impl::datasource_impl (const std::string& url)
  : url_ (url)
{
  auto f = mobius::io::new_file_by_url (url_);

  file_info_.set ("name", f.get_name ());
  file_info_.set ("path", f.get_path ());
  file_info_.set ("size", f.get_size ());
  file_info_.set ("creation_time", f.get_creation_time ());
  file_info_.set ("last_modification_time", f.get_modification_time ());
  file_info_.set ("last_metadata_time", f.get_metadata_time ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get object state
//! \return Object state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::pod::map
datasource_impl::get_state () const
{
  std::vector <mobius::pod::data> metadata;

  for (const auto& extraction : extractions_)
    metadata.push_back (extraction.get_state ());

  mobius::pod::map state = {
    {"url", url_},
    {"file_info", file_info_},
    {"case_info", case_info_},
    {"extractions", metadata},
  };

  return {
    {"type", get_type ()},
    {"state", state},
  };
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check if datasource is available
//! \return True/False
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
datasource_impl::is_available () const
{
  auto f = mobius::io::new_file_by_url (url_);
  return f.exists ();
}

} // namespace mobius::datasource::ufdr
