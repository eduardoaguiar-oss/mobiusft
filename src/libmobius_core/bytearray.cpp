// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
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
// along with this program. If not, see <https://www.gnu.org/licenses/>.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <mobius/core/bytearray.hpp>
#include <mobius/core/charset.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <array>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert a uint64_t value to a hex string
// @param value Value to convert
// @param siz Size of the hex string
// @return Hex string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
to_hex (std::uint64_t value, unsigned int siz)
{
    char buffer[32] = {0};

    snprintf (
        buffer,
        sizeof (buffer),
        "%0*llx",
        static_cast<int>(siz),
        static_cast<unsigned long long> (value)
    );

    return std::string (buffer);
}

} // namespace

namespace mobius::core
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create bytearray from C string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray::bytearray (const char *str)
    : data_ (str, str + strlen (str))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create bytearray from C++ string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray::bytearray (const std::string &str)
    : data_ (str.begin (), str.end ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create bytearray from C array
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray::bytearray (const uint8_t *data, size_type size)
    : data_ (data, data + size)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create bytearray with size copies of byte b
// @param b Byte
// @param size Bytearray size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray::bytearray (const uint8_t b, size_type size)
    : data_ (size)
{
    fill (b);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create bytearray with given size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray::bytearray (bytearray::size_type size)
    : data_ (size)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create bytearray from initializer list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray::bytearray (const std::initializer_list<uint8_t> &list)
    : data_ (list.begin (), list.end ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create bytearray from two const iterators
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray::bytearray (const_iterator b, const_iterator e)
    : data_ (b, e)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief operator[]
// @return element of bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray::const_reference
bytearray::operator[] (bytearray::size_type idx) const
{
    return data_.at (idx);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief operator[]
// @return Reference to an element of bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray::reference
bytearray::operator[] (bytearray::size_type idx)
{
    return data_.at (idx);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief operator^= apply XOR operator on two bytearrays
// @param o another bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray &
bytearray::operator^= (const bytearray &o) noexcept
{
    for (size_type i = 0; i < std::min (size (), o.size ()); i++)
    {
        data_[i] ^= o.data_[i];
    }

    return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief operator+= concatenate another bytearray
// @param o another bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray &
bytearray::operator+= (const bytearray &o)
{
    data_.insert (data_.end (), o.data_.begin (), o.data_.end ());
    return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief operator<<= Left shift with carrying
// @param n Number of bits
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray &
bytearray::operator<<= (int n) noexcept
{
    if (n <= 0)
        return *this;

    // if n > size in bits, simply fill with 0
    if (static_cast<size_type> (n) >= size () * 8)
    {
        fill (0);
        return *this;
    }

    // shift data
    int bytes = (n >> 3);
    int bits = (n & 7);
    size_type siz = size () - bytes;

    for (size_type i = 0; i < siz - 1; i++)
        data_[i] =
            (data_[i + bytes] << bits) | (data_[i + bytes + 1] >> (8 - bits));

    data_[siz - 1] = data_[size () - 1] << bits;

    // reset bytes at end
    std::fill (data_.end () - bytes, data_.end (), 0);

    return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief operator<<= Right shift with carrying
// @param n Number of bits
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray &
bytearray::operator>>= (int n) noexcept
{
    if (n <= 0)
        return *this;

    // if n > size in bits, simply fill with 0
    if (static_cast<size_type> (n) >= size () * 8)
    {
        fill (0);
        return *this;
    }

    // shift data
    size_type bytes = (n >> 3);
    int bits = (n & 7);

    for (size_type i = size () - 1; i > bytes; i--)
        data_[i] =
            (data_[i - bytes] >> bits) | (data_[i - bytes - 1] << (8 - bits));

    data_[bytes] = (data_[0] >> bits);

    // reset bytes at beginning
    std::fill (data_.begin (), data_.begin () + bytes, 0);

    return *this;
}
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Fill array with value
// @param val value to fill the array with
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
bytearray::fill (bytearray::value_type val) noexcept
{
    std::fill (data_.begin (), data_.end (), val);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Fill array with random bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
bytearray::random () noexcept
{
    std::generate (data_.begin (), data_.end (), std::rand);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief left pad bytearray
// @param siz new size in bytes
// @param val value to fill
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
bytearray::lpad (bytearray::size_type siz, bytearray::value_type val)
{
    if (siz > data_.size ())
    {
        auto old_size = data_.size ();
        resize (siz);
        std::fill (data_.begin (), data_.begin () + (siz - old_size), val);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief right pad bytearray
// @param siz new size in bytes
// @param val value to fill
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
bytearray::rpad (size_type siz, value_type val)
{
    if (siz > data_.size ())
    {
        auto old_size = data_.size ();
        resize (siz);
        std::fill (data_.begin () + old_size, data_.end (), val);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Count the elements that are equal to value
// @param value Value to compare
// @return Number of elements that are equal to value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
bytearray::count (value_type value) const noexcept
{
    return std::count (data_.begin (), data_.end (), value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if all bytes are equal to value
// @param value Value to check
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
bytearray::all_equal (value_type value) const noexcept
{
    return std::all_of (
        data_.begin (),
        data_.end (),
        [value] (std::uint8_t i) { return i == value; }
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Compare bytearray slice to another bytearray
// @param pos Start position
// @param b Bytearray
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
bytearray::compare (size_type pos, const bytearray &b) const noexcept
{
    return pos + b.size () <= size () &&
           std::equal (b.begin (), b.end (), begin () + pos);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if bytearray starts with another bytearray
// @param b Bytearray
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
bytearray::startswith (const bytearray &b) const noexcept
{
    return compare (0, b);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if bytearray ends with another bytearray
// @param b Bytearray
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
bytearray::endswith (const bytearray &b) const noexcept
{
    return compare (size () - b.size (), b);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Search for sequence inside bytearray
// @param b Sequence data
// @return Sequence position or <i>notfound</i> if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray::pos_type
bytearray::find (const bytearray &b) const noexcept
{
    auto iter = std::search (begin (), end (), b.begin (), b.end ());

    if (iter == end ())
        return notfound;

    return iter - begin ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Search for sequence inside bytearray, starting from the end
// @param b Sequence data
// @return Sequence position or <i>notfound</i> if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray::pos_type
bytearray::rfind (const bytearray &b) const noexcept
{
    auto iter = std::find_end (begin (), end (), b.begin (), b.end ());

    if (iter == end ())
        return notfound;

    return iter - begin ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create a new bytearray from a subset of bytearray
// @param start start position
// @param end end position
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
bytearray::slice (bytearray::size_type start, bytearray::size_type end) const
{
    if (data_.empty ())
        return {};

    if (start == 0 && end >= data_.size ())
        return *this;

    bytearray ret;

    if (end >= data_.size ())
        end = data_.size () - 1;

    if (start <= end)
        ret = bytearray (data_.data () + start, end - start + 1);

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief parse a hexadecimal string into bytearray
// @param s string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
bytearray::from_hexstring (const std::string &s)
{
    // resize bytearray
    data_.resize ((s.length () + 1) / 2);

    // convert hexstring to bytearray
    char buffer[3] = {0};
    constexpr int BASE = 16;
    std::string::size_type pos = 0;
    size_type idx = 0;

    if (s.length () % 2 == 1)
    {
        buffer[0] = s[pos++];
        buffer[1] = '\0';
        data_[idx++] = strtoul (buffer, nullptr, BASE);
    }

    while (pos < s.length ())
    {
        buffer[0] = s[pos++];
        buffer[1] = s[pos++];
        data_[idx++] = strtoul (buffer, nullptr, BASE);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief return a hexadecimal representation of bytearray
// @return hexadecimal string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
bytearray::to_hexstring () const
{
    std::string str (data_.size () * 2, ' ');
    std::string::size_type pos = 0;
    char buffer[3];

    for (auto c : data_)
    {
        sprintf (buffer, "%02x", c);
        str[pos++] = buffer[0];
        str[pos++] = buffer[1];
    }

    return str;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert bytearray to string
// @param encoding Byterray encoding or "", if none
// @return string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
bytearray::to_string (const std::string &encoding) const
{
    if (encoding.empty ())
        return std::string (data_.begin (), data_.end ());

    return mobius::core::conv_charset_to_utf8 (*this, encoding);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Generate hexdump from bytearray
// @return String
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
bytearray::dump (unsigned int indent) const
{
    std::string start = std::string (indent, ' ');
    std::string dump;

    for (std::uint64_t i = 0; i < data_.size (); i += 16)
    {
        dump += start;
        dump += to_hex (i, 8);
        dump += " ";

        for (int j = 0; j < 16; j++)
        {
            dump += " ";

            if (j == 8)
                dump += " ";

            if (i + j < data_.size ())
                dump += to_hex (data_[i + j], 2);

            else
                dump += "  ";
        }

        dump += "  ";

        for (int j = 0; j < 16; j++)
        {
            if (i + j < data_.size ())
            {
                auto c = data_[i + j];
                dump += (c < 32 || c > 126) ? "." : std::string (1, c);
            }
        }

        dump += '\n';
    }

    return dump;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Split bytearray into sequences
// @param sep Separator sequence data
// @return Sequences
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<bytearray>
bytearray::split (const bytearray &sep) const
{
    std::vector<bytearray> pieces;

    auto iter = begin ();
    auto next_iter = std::search (iter, end (), sep.begin (), sep.end ());

    while (next_iter != end ())
    {
        pieces.emplace_back (iter, next_iter);
        iter = next_iter + sep.size ();
        next_iter = std::search (iter, end (), sep.begin (), sep.end ());
    }

    return pieces;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Concatenate two bytearrays
// @param a1 bytearray
// @param a2 bytearray
// @return new concatenated bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray
operator+ (const bytearray &a1, const bytearray &a2)
{
    bytearray tmp = a1;
    tmp += a2;
    return tmp;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief XOR two bytearrays
// @param a1 bytearray
// @param a2 bytearray
// @return a1 ^ a2
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray
operator^ (const bytearray &a1, const bytearray &a2)
{
    bytearray tmp = a1;
    tmp ^= a2;
    return tmp;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Left shift bytearray
// @param b Bytearray
// @param n Number of bits
// @return New bytearray = b << n
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray
operator<< (const bytearray &b, int n)
{
    bytearray tmp = b;
    tmp <<= n;
    return tmp;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Right shift bytearray
// @param b Bytearray
// @param n Number of bits
// @return New bytearray = b >> n
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray
operator>> (const bytearray &b, int n)
{
    bytearray tmp = b;
    tmp >>= n;
    return tmp;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if two bytearrays are different
// @param a bytearray
// @param b bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator!= (const bytearray &a, const bytearray &b)
{
    return !(a == b);
}

} // namespace mobius::core
