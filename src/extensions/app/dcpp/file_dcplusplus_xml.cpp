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
#include "file_dcplusplus_xml.hpp"
#include <mobius/core/decoder/xml/dom.hpp>
#include <mobius/core/log.hpp>

namespace mobius::extension::app::dcpp
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_dcplusplus_xml::file_dcplusplus_xml (
    const mobius::core::io::reader &reader
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader || reader.get_size () < 16)
        return;

    try
    {
        mobius::core::decoder::xml::dom dom (reader);

        auto root = dom.get_root_element ();
        if (!root || root.get_name () != "DCPlusPlus")
            return;

        auto settings = root.get_child_by_path ("Settings");
        if (!settings)
            return;

        for (const auto &child : settings.get_children ())
        {
            auto type = child.get_property ("type");
            auto value_text = child.get_content ();
            auto value_name = child.get_name ();

            if (type == "string")
                data_.set (value_name, value_text);

            else if (type == "int" || type == "int64")
                data_.set (
                    value_name,
                    static_cast<std::int64_t> (std::stoll (value_text))
                );

            else
                log.development (
                    __LINE__, "Unhandled data type in DCPlusPlus.xml: " + type
                );
        }

        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::dcpp