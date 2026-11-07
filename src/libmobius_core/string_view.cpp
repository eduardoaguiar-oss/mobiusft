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
#include <mobius/core/string_view.hpp>
#include <charconv>
#include <iomanip>
#include <regex>
#include <sstream>
#include <algorithm>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Basic email validation regex
static const std::regex EMAIL_REGEX (R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");

// @brief Basic URL validation regex
static const std::regex URL_REGEX (R"((http|https)://([a-zA-Z0-9.-]+)(:[0-9]+)?(/.*)?)");

// @brief Basic IPv4 validation regex
static const std::regex IPV4_REGEX (
    R"((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))"
);

// @brief Basic IPv6 validation regex
static const std::regex IPV6_REGEX (
    R"((^([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$)|(^([0-9a-fA-F]{1,4}:){1,7}:$)|(^:([0-9a-fA-F]{1,4}:){1,6}[0-9a-fA-F]{1,4}$)|(^([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}$)|(^([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}$)|(^([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}$)|(^([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}$)|(^([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}$)|(^[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){6}|:)$)|(^:((:[0-9a-fA-F]{1,4}){7}|:)$))"
);

// @brief Cell phone (Brazil)
static const std::regex BRAZIL_MOBILE_REGEX (R"(^(\+?55\s?)?\(?(\d{2})\)?[\s.-]?9\d{4}[\s.-]?\d{4}$)");

// USA
static const std::regex USA_MOBILE_REGEX (R"(^(\+?1\s?)?\(?(\d{3})\)?[\s.-]?(\d{3})[\s.-]?(\d{4})$)");

// @brief CNPJ multipliers
static constexpr int CNPJ_MULTIPLIERS[] = {6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};
} // namespace

namespace mobius::core::string_view
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string is composed of digits
// @param str string
// @return true if str is composed of digits, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_digit (std::string_view str)
{
    if (str.empty ())
        return false;

    return std::all_of (str.begin (), str.end (), [] (char c) { return std::isdigit (c); });
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string is a number
// @param str string
// @return true if str is a number, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_integer (std::string_view str)
{
    if (str.empty ())
        return false;

    if (str[0] != '-' && str[0] != '+' && !std::isdigit (str[0]))
        return false;

    return std::all_of (str.begin (), str.end (), [] (char c) { return std::isdigit (c); });
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string is a hexadecimal number
// @param str string
// @return true if str is a hexadecimal number, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_hex (std::string_view str)
{
    if (str.empty ())
        return false;

    return std::all_of (str.begin (), str.end (), [] (char c) { return std::isxdigit (c); });
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string is alphabetic
// @param str string
// @return true if str is alphabetic, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_alpha (std::string_view str)
{
    if (str.empty ())
        return false;

    return std::all_of (str.begin (), str.end (), [] (char c) { return std::isalpha (c); });
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string is alphanumeric
// @param str string
// @return true if str is alphanumeric, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_alnum (std::string_view str)
{
    if (str.empty ())
        return false;

    return std::all_of (str.begin (), str.end (), [] (char c) { return std::isalnum (c); });
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string is lowercase
// @param str string
// @return true if str is lowercase, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_lower (std::string_view str)
{
    if (str.empty ())
        return false;

    return std::all_of (str.begin (), str.end (), [] (char c) { return std::islower (c); });
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string is uppercase
// @param str string
// @return true if str is uppercase, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_upper (std::string_view str)
{
    if (str.empty ())
        return false;

    return std::all_of (str.begin (), str.end (), [] (char c) { return std::isupper (c); });
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string contains only whitespace characters
// @param str string
// @return true if str contains only whitespace characters, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_space (std::string_view str)
{
    if (str.empty ())
        return false;

    return std::all_of (str.begin (), str.end (), [] (char c) { return std::isspace (c); });
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string is a valid numeric CPF (Brazilian individual taxpayer identification number)
// @param str string
// @return true if str is a valid numeric CPF, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_numeric_cpf (std::string_view str)
{
    // CPF must have 11 digits
    if (str.length () != 11 || !is_digit (str))
        return false;

    // Calculate first verification digit
    int sum = 0;
    for (int i = 0; i < 9; i++)
        sum += (str[i] - '0') * (10 - i);

    int remainder = sum % 11;
    int dv1 = remainder < 2 ? 0 : 11 - remainder;

    // Check first verification digit
    if (dv1 != (str[9] - '0'))
        return false;

    // Calculate second verification digit
    sum = 0;
    for (int i = 0; i < 10; i++)
        sum += (str[i] - '0') * (11 - i);

    remainder = sum % 11;
    int dv2 = remainder < 2 ? 0 : 11 - remainder;

    // Check second verification digit
    return dv2 == (str[10] - '0');
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string is a valid numeric CNPJ (Brazilian National Registry of Legal Entities)
// @param str string Unformatted CNPJ (14 digits)
// @return true if str is a valid CNPJ, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_numeric_cnpj (std::string_view str)
{
    // CNPJ must have 14 digits
    if (str.length () != 14 || !is_digit (str))
        return false;

    // Calculate first verification digit
    int sum = 0;
    for (int i = 0; i < 12; i++)
        sum += (str[i] - '0') * (CNPJ_MULTIPLIERS[i + 1]);

    int remainder = sum % 11;
    int dv1 = remainder < 2 ? 0 : 11 - remainder;

    // Check first verification digit
    if (dv1 != (str[12] - '0'))
        return false;

    // Calculate second verification digit
    sum = 0;
    for (int i = 0; i < 12; i++)
        sum += (str[i] - '0') * (CNPJ_MULTIPLIERS[i]);

    sum += dv1 * 2;
    remainder = sum % 11;
    int dv2 = remainder < 2 ? 0 : 11 - remainder;

    // Check second verification digit
    return dv2 == (str[13] - '0');
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string is a valid mobile phone number (Brazil or USA)
// @param str string
// @return true if str is a valid mobile phone number, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_mobile_phone (std::string_view str)
{
    return std::regex_match (str.begin (), str.end (), BRAZIL_MOBILE_REGEX) ||
           std::regex_match (str.begin (), str.end (), USA_MOBILE_REGEX);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string is a valid email address
// @param str string
// @return true if str is a valid email address, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_email (std::string_view str)
{
    return std::regex_match (str.begin (), str.end (), EMAIL_REGEX);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string is a valid URL
// @param str string
// @return true if str is a valid URL, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_url (std::string_view str)
{
    return std::regex_match (str.begin (), str.end (), URL_REGEX);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string is a valid IPv4 address
// @param str string
// @return true if str is a valid IPv4 address, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_ipv4 (std::string_view str)
{
    return std::regex_match (str.begin (), str.end (), IPV4_REGEX);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if a string is a valid IPv6 address
// @param str string
// @return true if str is a valid IPv6 address, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_ipv6 (std::string_view str)
{
    return std::regex_match (str.begin (), str.end (), IPV6_REGEX);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief strip chars from both ends of a string
// @param str string
// @param chars chars
// @return a new string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string_view
strip (std::string_view str, std::string_view chars)
{
    const auto pos1 = str.find_first_not_of (chars);
    const auto pos2 = str.find_last_not_of (chars);

    if (pos1 == std::string::npos || pos2 == std::string::npos)
        return {};

    return str.substr (pos1, pos2 - pos1 + 1);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief strip chars from the beginning of a string
// @param str string
// @param chars chars
// @return a new string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string_view
lstrip (std::string_view str, std::string_view chars)
{
    const auto pos = str.find_first_not_of (chars);

    if (pos == std::string::npos)
        return {};

    return str.substr (pos);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief strip chars from the end of a string
// @param str string
// @param chars chars
// @return a new string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string_view
rstrip (std::string_view str, std::string_view chars)
{
    const auto pos = str.find_last_not_of (chars);

    if (pos == std::string::npos)
        return {};

    return str.substr (0, pos + 1);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief split string into parts
// @param str string
// @param sep separator string
// @return vector containing parts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<std::string_view>
split (std::string_view str, std::string_view sep)
{
    std::vector<std::string_view> result;
    std::string_view::size_type start = 0;
    std::string_view::size_type end = str.find (sep);

    while (end != std::string_view::npos)
    {
        result.push_back (str.substr (start, end - start));
        start = end + sep.length ();
        end = str.find (sep, start);
    }

    result.push_back (str.substr (start));
    return result;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get n word from string
// @param str string
// @param n word index (negative values count from right to left)
// @param sep separator string
// @return word
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string_view
word (std::string_view str, int n, std::string_view sep)
{
    std::string_view word;

    // split string
    auto d = split (str, sep);

    if (n < 0)
        n = d.size () + n;

    // get n word
    if (n >= 0 && n < static_cast<int> (d.size ()))
        word = d[n];

    return word;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert string_view to integer, if possible
// @param str string
// @return integer value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
to_integer (std::string_view str)
{
    std::int64_t result;
    auto [_, ec] = std::from_chars (str.data (), str.data () + str.size (), result);

    if (ec == std::errc())
        return result;
    else
        throw std::invalid_argument("Invalid integer string");
}

} // namespace mobius::core::string_view
