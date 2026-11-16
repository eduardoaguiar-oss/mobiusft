#ifndef MOBIUS_CORE_KFF_FILE_HPP
#define MOBIUS_CORE_KFF_FILE_HPP

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
#include <mobius/core/bytearray.hpp>
#include <mobius/core/pod/data.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace mobius::core::kff
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File attributes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr std::uint16_t FILE_ATTRIBUTE_PASSWORD = 1;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief KFF file class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file () = default;
    explicit file (std::int64_t);
    file (std::int64_t, std::uint16_t, std::uint64_t);

    bool is_alert () const;
    std::uint16_t get_type () const;

    bool has_size () const;
    std::int64_t get_size () const;
    void set_size (std::uint64_t size);

    bool has_hash (std::uint16_t) const;
    std::string get_hash (std::uint16_t) const;
    void set_hash (std::uint16_t, const std::string &);
    void set_hash (std::uint16_t, const mobius::core::bytearray &);
    void remove_hash (std::uint16_t);

    bool has_attribute (std::uint16_t) const;
    void set_attribute (std::uint16_t, const mobius::core::pod::data &);
    mobius::core::pod::data get_attribute (std::uint16_t) const;
    void remove_attribute (std::uint16_t);

    bool has_thumbnail () const;
    mobius::core::bytearray get_thumbnail () const;
    void set_thumbnail (const mobius::core::bytearray &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if the file is valid
    // @return True if the file is valid, false otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit
    operator bool () const noexcept
    {
        return bool (impl_);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get attribute as a specific type
    // @tparam T The type to convert the attribute to
    // @param attr The attribute identifier
    // @return The attribute value converted to the specified type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    template <typename T>
    T
    get_attribute_as (std::uint16_t attr) const
    {
        return static_cast<T> (get_attribute (attr));
    }

  private:
    // @brief Forward declaration of implementation class
    class impl;

    // @brief Pointer to implementation
    std::shared_ptr<impl> impl_;
};

} // namespace mobius::core::kff

#endif // MOBIUS_CORE_KFF_FILE_HPP