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
#include "tokenizer.h"

namespace mobius
{
namespace decoder
{
namespace sgml
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Create tokenizer
//! \param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
tokenizer::tokenizer (const mobius::io::reader& reader)
 : sourcecode_ (reader)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get token
//! \return Token type and token text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::pair <tokenizer::token_type, std::string>
tokenizer::get_token ()
{
  token_type type = token_type::end;
  std::string text;

  auto c = sourcecode_.peek ();

  // entity
  if (c == '&')
    {
      type = token_type::entity;
      text = _get_entity_token ();
    }

  // tag
  else if (c == '<')
    {
      text = _get_tag_token ();

      if (text.size () > 1 && text[1] == '/')
        type = token_type::end_tag;

      else if (text.size () > 1 && text[text.size () - 2] == '/')
       type = token_type::empty_tag;

      else
        type = token_type::start_tag;
    }

  // general text
  else if (c != 0)
    {
      type = token_type::text;
      text = _get_text_token ();
    }

  return std::make_pair (type, text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get entity token
//! \return Entity token ("&entity;")
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
tokenizer::_get_entity_token ()
{
  std::string text (1, sourcecode_.get ());

  auto c = sourcecode_.get ();

  while (c && c != ';')
    {
      text.push_back (c);
      c = sourcecode_.get ();
    }

  if (c == ';')
    text.push_back (c);

  return text;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get tag token
//! \return Tag token ("<xxx yyy=...>")
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
tokenizer::_get_tag_token ()
{
  std::string text (1, sourcecode_.get ());

  auto c = sourcecode_.get ();
  while (c && c != '>')
    {
      text.push_back (c);
      c = sourcecode_.get ();
    }

  if (c == '>')
    text.push_back (c);

  return text;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get text token
//! \return Text token
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
tokenizer::_get_text_token ()
{
  std::string text (1, sourcecode_.get ());

  auto c = sourcecode_.peek ();

  while (c && c != '<' && c != '&')
    {
      text.push_back (sourcecode_.get ());
      c = sourcecode_.peek ();
    }

  return text;
}

} // namespace sgml
} // namespace decoder
} // namespace mobius
