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
#include "CXMLElement.hpp"
#include <algorithm>
#include <iostream>

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode CXMLElement structure
// @see CXMLElement::Serialize@XML.cpp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CXMLElement::decode (mobius::decoder::mfc& decoder)
{
  name_ = decoder.get_string ();
  value_ = decoder.get_string ();

  // attributes
  auto count = decoder.get_count ();

  for (std::uint32_t i = 0; i < count; i++)
    {
      auto name = decoder.get_string ();
      auto value = decoder.get_string ();
      attributes_[name] = value;
    }

  // child elements
  count = decoder.get_count ();

  for (std::uint32_t i = 0; i < count; i++)
    {
      CXMLElement child;
      child.decode (decoder);

      children_.push_back (child);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get metadata from element
// @return Metadata
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::map <std::string, std::string>
CXMLElement::get_metadata () const
{
  std::map <std::string, std::string> metadata;

  if (!name_.empty ())
    {
      const std::string name = name_;

      std::transform (
         attributes_.begin (),
         attributes_.end (),
         std::inserter (metadata, metadata.end ()),
         [name](const auto& p){ return std::make_pair (name + '.' + p.first, p.second); }
      );
    }

  return metadata;
}

} // namespace mobius::extension::app::shareaza




