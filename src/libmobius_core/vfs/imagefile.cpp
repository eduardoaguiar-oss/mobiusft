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
#include <mobius/core/exception.inc>
#include <mobius/core/io/file.hpp>
#include <mobius/core/resource.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/vfs/imagefile.hpp>
#include <mobius/core/vfs/imagefile_impl_null.hpp>
#include <stdexcept>
#include <unordered_map>

namespace mobius::core::vfs
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Image file types supported
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::unordered_map<std::string, imagefile_type> IMAGEFILE_TYPES;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Build implementation, according to ID
// @param f File object
// @param id Implementation ID
// @return shared_ptr to implementation object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr<imagefile_impl_base>
_build_imagefile_implementation (const mobius::core::io::file &f, const std::string &id)
{
    // If type == "autodetect", try to detect the imagefile type automatically.
    if (id == "autodetect")
    {
        for (const auto &[_, type] : IMAGEFILE_TYPES)
        {
            if (type.is_instance (f))
                return type.builder (f);
        }
    }

    // Otherwise, if type is given, create imagefile using type implementation
    else
    {
        auto iter = IMAGEFILE_TYPES.find (id);

        if (iter != IMAGEFILE_TYPES.end ())
            return iter->second.builder (f);
    }

    // Fallback: raw imagefile
    auto iter = IMAGEFILE_TYPES.find ("raw");

    if (iter != IMAGEFILE_TYPES.end ())
        return iter->second.builder (f);

    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("Unsupported imagefile type: " + id));
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Construct object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile::imagefile ()
    : impl_ (std::make_shared<imagefile_impl_null> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor from implementation pointer
// @param impl implementation pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile::imagefile (const std::shared_ptr<imagefile_impl_base> &impl)
    : impl_ (impl)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor from file object
// @param f File object
// @param type Imagefile type (default = "autodetect")
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile::imagefile (const mobius::core::io::file &f, const std::string &type)
    : impl_ (_build_imagefile_implementation (f, type))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new imagefile object by URL
// @param url Imagefile URL
// @param type Imagefile type (default = "autodetect")
// @return Imagefile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile
new_imagefile_by_url (const std::string &url, const std::string &type)
{
    auto f = mobius::core::io::new_file_by_url (url);
    return imagefile (f, type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new imagefile object by path
// @param path Imagefile path
// @param type Imagefile type (default = "autodetect")
// @return Imagefile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile
new_imagefile_by_path (const std::string &path, const std::string &type)
{
    auto f = mobius::core::io::new_file_by_path (path);
    return imagefile (f, type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new imagefile object from file
// @param f File object
// @param type Imagefile type (default = "autodetect")
// @return Imagefile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile
new_imagefile_from_file (const mobius::core::io::file &f, const std::string &type)
{
    return imagefile (f, type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Register a new imagefile type
// @param type Imagefile type to register
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
register_imagefile_type (const imagefile_type &type)
{
    IMAGEFILE_TYPES[type.id] = type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unregister an imagefile type by ID
// @param id ID of the imagefile type to unregister
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
unregister_imagefile_type (const std::string &id)
{
    IMAGEFILE_TYPES.erase (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get imagefile types
// @return Vector of imagefile types
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<imagefile_type>
get_imagefile_types ()
{
    std::vector<imagefile_type> types (IMAGEFILE_TYPES.size ());

    // Convert the IMAGEFILE_TYPES map to a vector of imagefile_type objects
    std::transform (
        IMAGEFILE_TYPES.begin (),
        IMAGEFILE_TYPES.end (),
        types.begin (),
        [] (const auto &pair) { return pair.second; }
    );

    // Sort the imagefile types by their ID
    std::sort (
        types.begin (),
        types.end (),
        [] (const imagefile_type &a, const imagefile_type &b) { return a.id < b.id; }
    );

    return types;
}

} // namespace mobius::core::vfs
