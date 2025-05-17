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
// along with this program. If not, see <http://www.gnu.org/licenses/>.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <atomic>
#include <map>
#include <mobius/core/decoder/inifile.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/io/line_reader.hpp>
#include <mobius/core/string_functions.hpp>
#include <stdexcept>
#include <utility>

namespace mobius::core::decoder
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief inifile implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class inifile::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl (const impl &) = delete;
    impl (impl &&) = delete;
    impl (const mobius::core::io::reader &, const std::string &,
          const std::string &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl &operator= (const impl &) = delete;
    impl &operator= (impl &&) = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool has_value (const std::string &, const std::string &) const;
    std::string get_value (const std::string &, const std::string &) const;
    void set_case_sensitive (bool);
    void set_comment_char (char);

  private:
    // @brief Line reader object
    mutable mobius::core::io::line_reader line_reader_;

    // @brief Flag is case sensitive
    bool is_case_sensitive_ = false;

    // @brief Comment char
    char comment_char_ = ';';

    // @brief Flag is loaded
    mutable std::atomic_bool is_loaded_ = false;

    // @brief Map of group,key -> value
    mutable std::map<std::pair<std::string, std::string>, std::string> values_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load () const;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param encoding File encoding
// @param separator Line separator
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inifile::impl::impl (const mobius::core::io::reader &reader,
                     const std::string &encoding, const std::string &separator)
    : line_reader_ (reader, encoding, separator)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if inifile has a given value
// @param group Group name
// @param key Key
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
inifile::impl::has_value (const std::string &group,
                          const std::string &key) const
{
    _load ();

    std::string a_group =
        (is_case_sensitive_) ? group : mobius::core::string::tolower (group);
    std::string a_key =
        (is_case_sensitive_) ? key : mobius::core::string::tolower (key);

    return values_.find ({a_group, a_key}) != values_.end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get value
// @param group Group name
// @param key Key
// @return Value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
inifile::impl::get_value (const std::string &group,
                          const std::string &key) const
{
    std::string value;

    _load ();

    std::string a_group =
        (is_case_sensitive_) ? group : mobius::core::string::tolower (group);
    std::string a_key =
        (is_case_sensitive_) ? key : mobius::core::string::tolower (key);

    auto iter = values_.find ({a_group, a_key});

    if (iter != values_.end ())
        value = iter->second;

    return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set group and key to be case sensitive or not
// @param flag Flag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
inifile::impl::set_case_sensitive (bool flag)
{
    if (is_loaded_)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("could not change case sensitive because "
                                  "inifile is already loaded"));

    is_case_sensitive_ = flag;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set char used to start a comment
// @param c Char
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
inifile::impl::set_comment_char (char c)
{
    if (is_loaded_)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG (
            "could not set comment char because inifile is already loaded"));

    comment_char_ = c;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load values
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
inifile::impl::_load () const
{
    if (is_loaded_)
        return;

    std::string line;
    std::string group;

    while (line_reader_.read (line))
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Strip comments
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto pos = line.find (comment_char_);

        if (pos != std::string::npos)
            line.erase (pos);

        if (!line.empty ())
        {
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Handle groups
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            if (line[0] == '[')
            {
                pos = line.find (']');

                if (pos != std::string::npos)
                    group =
                        mobius::core::string::strip (line.substr (1, pos - 1));
            }

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Handle values
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            else
            {
                pos = line.find ('=');

                if (pos != std::string::npos)
                {
                    auto key =
                        mobius::core::string::strip (line.substr (0, pos));
                    auto value =
                        mobius::core::string::strip (line.substr (pos + 1));

                    if (!is_case_sensitive_)
                    {
                        group = mobius::core::string::tolower (group);
                        key = mobius::core::string::tolower (key);
                    }

                    values_[{group, key}] = value;
                }
            }
        }
    }

    is_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param encoding File encoding
// @param separator Line separator
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inifile::inifile (const mobius::core::io::reader &reader,
                  const std::string &encoding, const std::string &separator)
    : impl_ (std::make_shared<impl> (reader, encoding, separator))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if inifile has a given value
// @param group Group name
// @param name Value name
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
inifile::has_value (const std::string &group, const std::string &name) const
{
    return impl_->has_value (group, name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get value
// @param group Group name
// @param name Value name
// @return Value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
inifile::get_value (const std::string &group, const std::string &name) const
{
    return impl_->get_value (group, name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set group and key to be case sensitive or not
// @param flag Flag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
inifile::set_case_sensitive (bool flag)
{
    impl_->set_case_sensitive (flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set char used to start a comment
// @param c Char
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
inifile::set_comment_char (char c)
{
    impl_->set_comment_char (c);
}

} // namespace mobius::core::decoder
