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
#include <cstdint>
#include <memory>
#include <string>

namespace mobius::core::kff
{
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
    int get_type () const;

    bool has_size () const;
    std::int64_t get_size () const;
    void set_size (std::uint64_t size);

    bool has_hash (std::uint16_t) const;
    std::string get_hash (std::uint16_t) const;
    void set_hash (std::uint16_t, const std::string &);
    void remove_hash (std::uint16_t);

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

  private:
    // @brief Forward declaration of implementation class
    class impl;

    // @brief Pointer to implementation
    std::shared_ptr<impl> impl_;
};

} // namespace mobius::core::kff

#endif // MOBIUS_CORE_KFF_FILE_HPP