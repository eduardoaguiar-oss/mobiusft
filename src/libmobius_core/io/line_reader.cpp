// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
// Eduardo Aguiar
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
#include <mobius/core/io/line_reader.hpp>

namespace mobius::core::io
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Detect line separator
// @param s String
// @return Separator string or "" if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
_detect_separator (const std::string &s)
{
    char last_c = 0;

    for (const auto &c : s)
    {
        if (c == '\n' && last_c == '\r')
            return "\r\n";

        else if (c == '\n')
            return "\n";

        else if (last_c == '\r')
            return "\r";

        last_c = c;
    }

    return {};
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param text_reader Text reader object
// @param separator Line separator
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
line_reader::line_reader (const mobius::core::io::text_reader &text_reader,
                          const std::string &separator)
    : text_reader_ (text_reader),
      separator_ (separator)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param encoding Charset encoding
// @param separator Line separator
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
line_reader::line_reader (const mobius::core::io::reader &reader,
                          const std::string &encoding,
                          const std::string &separator)
    : text_reader_ (text_reader (reader, encoding)),
      separator_ (separator)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Read line
// @param line Reference to line
// @return <i>true</i> if line has been read, <i>false</i> otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
line_reader::read (std::string &line)
{
    // detect separator, if not set
    while (separator_.empty ())
    {
        auto data = text_reader_.read (65536);

        if (data.empty ())
        {
            if (buffer_.empty ())
                return false;

            line = buffer_;
            buffer_.clear ();
            return true;
        }

        buffer_ += data;
        separator_ = _detect_separator (buffer_);
    }

    // search for separator
    auto pos = buffer_.find (separator_);

    while (pos == std::string::npos)
    {
        auto data = text_reader_.read (65536);

        if (data.empty ())
        {
            if (buffer_.empty ())
                return false;

            line = buffer_;
            buffer_.clear ();
            return true;
        }

        buffer_ += data;
        pos = buffer_.find (separator_);
    }

    // if separator found, return line up to separator
    if (pos != std::string::npos)
    {
        line = buffer_.substr (0, pos);
        buffer_ = buffer_.erase (0, pos + separator_.size ());
        return true;
    }

    return false;
}

} // namespace mobius::core::io
