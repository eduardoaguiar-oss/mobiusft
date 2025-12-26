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
#include <iconv.h>
#include <memory>
#include <mobius/core/charset.hpp>
#include <mobius/core/exception_posix.inc>
#include <mobius/core/string_functions.hpp>
#include <stdexcept>

namespace mobius::core
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert bytearray from charset to UTF-8
// @param data bytearray
// @param charset charset
// @return string encoded as UTF-8
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
conv_charset_to_utf8 (const mobius::core::bytearray &data,
                      const std::string &charset)
{
    const std::string u_charset = mobius::core::string::toupper (charset);

    if (u_charset == "UTF-8" || u_charset == "UTF8")
        return data.to_string ();

    else
    {
        bytearray output = conv_charset (data, u_charset, "UTF-8");

        // strip trailings '\0'
        auto size = output.size ();
        while (size > 0 && output[size - 1] == '\0')
            --size;

        return std::string (reinterpret_cast<const char *> (output.data ()),
                            size);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert bytearray from charset to UTF-8, with partial convertion
// @param data bytearray
// @param charset charset
// @return string encoded as UTF-8 and remaining bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::pair<std::string, mobius::core::bytearray>
conv_charset_to_utf8_partial (const mobius::core::bytearray &data,
                              const std::string &charset)
{
    const std::string u_charset = mobius::core::string::toupper (charset);

    if (u_charset == "UTF-8" || u_charset == "UTF8")
        return std::make_pair (data.to_string (), mobius::core::bytearray ());

    else
    {
        auto p = conv_charset_partial (data, u_charset, "UTF-8");
        return std::make_pair (p.first.to_string (), p.second);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert a bytearray from one charset to other charset
// @param data bytearray
// @param f_charset from charset
// @param t_charset to charset
// @return new bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
conv_charset (const mobius::core::bytearray &data, const std::string &f_charset,
              const std::string &t_charset)
{
    // allocate descriptor
    const std::string uf_charset = mobius::core::string::toupper (f_charset);
    const std::string ut_charset = mobius::core::string::toupper (t_charset);
    iconv_t cd = iconv_open (ut_charset.c_str (), uf_charset.c_str ());

    if (cd == (iconv_t) -1)
        throw std::invalid_argument (MOBIUS_EXCEPTION_POSIX);

    // convert data
    mobius::core::bytearray out;
    size_t insize = data.size ();
    size_t outsize = insize * 6;

    char *inbuf = (char *) data.data ();
    auto outbuf = std::make_unique<char[]> (outsize);
    char *p_out = outbuf.get ();

    if (::iconv (cd, &inbuf, &insize, &p_out, &outsize) != (size_t) -1)
    {
        // flush out partially converted input
        iconv (cd, nullptr, nullptr, &p_out, &outsize);

        out = mobius::core::bytearray (
            reinterpret_cast<const uint8_t *> (outbuf.get ()),
            p_out - outbuf.get ());
    }

    // deallocate descriptor
    iconv_close (cd);

    return out;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert bytearray from one charset to other, with partial convertion
// @param data bytearray
// @param f_charset from charset
// @param t_charset to charset
// @return new bytearray and remaining bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::pair<mobius::core::bytearray, mobius::core::bytearray>
conv_charset_partial (const mobius::core::bytearray &data,
                      const std::string &f_charset,
                      const std::string &t_charset)
{
    // allocate descriptor
    const std::string uf_charset = mobius::core::string::toupper (f_charset);
    const std::string ut_charset = mobius::core::string::toupper (t_charset);
    iconv_t cd = iconv_open (ut_charset.c_str (), uf_charset.c_str ());

    if (cd == (iconv_t) -1)
        throw std::invalid_argument (MOBIUS_EXCEPTION_POSIX);

    // convert data
    mobius::core::bytearray out;
    mobius::core::bytearray rest;
    size_t insize = data.size ();
    size_t outsize = insize * 6;

    char *inbuf = (char *) data.data ();
    auto outbuf = std::make_unique<char[]> (outsize);
    char *p_out = outbuf.get ();

    size_t rc = ::iconv (cd, &inbuf, &insize, &p_out, &outsize);

    if (rc != (size_t) -1 || errno == EINVAL)
    {
        out = mobius::core::bytearray (
            reinterpret_cast<const uint8_t *> (outbuf.get ()),
            p_out - outbuf.get ());

        if (inbuf < (char *) data.end ())
            rest = mobius::core::bytearray (
                reinterpret_cast<const uint8_t *> (inbuf),
                (char *) data.end () - inbuf);
    }

    // deallocate descriptor
    iconv_close (cd);

    return std::make_pair (out, rest);
}

} // namespace mobius::core
