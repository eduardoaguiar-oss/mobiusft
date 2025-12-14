#ifndef MOBIUS_FRAMEWORK_MODEL_EVIDENCE_HPP
#define MOBIUS_FRAMEWORK_MODEL_EVIDENCE_HPP

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
#include <mobius/core/io/file.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/framework/model/item.hpp>
#include <cstdint>
#include <memory>
#include <map>
#include <set>
#include <string>

namespace mobius::framework::model
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Evidence object
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class evidence
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Datatypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    enum class source_type
    {
        none,
        file,
        evidence,
        folder
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Evidence source class
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct source
    {
        source_type type;
        std::uint64_t source_uid;
        std::string description;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence (item, std::int64_t, const std::string &);
    evidence () noexcept = default;
    evidence (evidence &&) noexcept = default;
    evidence (const evidence &) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence &operator= (const evidence &) noexcept = default;
    evidence &operator= (evidence &&) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Function prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    item get_item () const;
    std::int64_t get_uid () const;
    std::string get_type () const;

    bool has_attribute (const std::string &) const;
    mobius::core::pod::data get_attribute (const std::string &) const;
    void set_attribute (const std::string &, const mobius::core::pod::data &);
    void
    set_attributes (const std::map<std::string, mobius::core::pod::data> &);
    void remove_attribute (const std::string &);
    std::map<std::string, mobius::core::pod::data> get_attributes () const;

    bool has_tag (const std::string &) const;
    void set_tag (const std::string &);
    void set_tags (const std::set<std::string> &);
    void reset_tag (const std::string &);
    std::set<std::string> get_tags () const;

    void add_source (const mobius::core::io::file &);
    void add_source (const mobius::core::io::folder &);
    void add_source (const evidence &);
    std::vector<evidence::source> get_sources () const;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get attribute
    // @param id Attribute ID
    // @param value Default value
    // @return Attribute value
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    template <typename T>
    T
    get_attribute (const std::string &id, const T &value = T {}) const
    {
        if (has_attribute (id))
            return static_cast<T> (get_attribute (id));

        return value;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if object is valid
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit
    operator bool () const noexcept
    {
        return bool (impl_);
    }

  private:
    // @brief Implementation class forward declaration
    class impl;

    // @brief Implementation pointer
    std::shared_ptr<impl> impl_;
};

} // namespace mobius::framework::model

#endif
