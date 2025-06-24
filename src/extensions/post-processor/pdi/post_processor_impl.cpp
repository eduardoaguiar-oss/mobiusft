// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "post_processor_impl.hpp"
#include <mobius/core/application.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/core/string_functions.hpp>
#include <fstream>
#include <unordered_map>
#include <string>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Known field names
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::unordered_multimap<std::string, std::string> FIELDS;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Format CPF string
// @param cpf CPF string
// @return Formatted CPF string in the format "xxx.xxx.xxx-xx"
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
_format_cpf (const std::string &cpf)
{
    if (cpf.length () == 11)
        return cpf.substr (0, 3) + "." + cpf.substr (3, 3) + "." +
               cpf.substr (6, 3) + "-" + cpf.substr (9, 2);
    return cpf;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Format CNPJ string
// @param cnpj CNPJ string
// @return Formatted CNPJ string in the format "xx.xxx.xxx/xxxx-xx"
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
_format_cnpj (const std::string &cnpj)
{
    if (cnpj.length () == 14)
        return cnpj.substr (0, 2) + "." + cnpj.substr (2, 3) + "." +
               cnpj.substr (5, 3) + "/" + cnpj.substr (8, 4) + "-" +
               cnpj.substr (12, 2);
    return cnpj;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type validators for PDI values
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::
    unordered_map<std::string, std::function<bool (const std::string &)>>
        TYPE_VALIDATORS = {
            {"cnpj", mobius::core::string::is_cnpj},
            {"cpf", mobius::core::string::is_cpf},
            {"email", mobius::core::string::is_email},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type formatters for PDI values
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::
    unordered_map<std::string, std::function<std::string (const std::string &)>>
        TYPE_FORMATTERS = {
            {"cnpj", _format_cnpj},
            {"cpf", _format_cpf},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Validate value
// @brief type Type of value to validate
// @param value Value to validate
// @return true if valid, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
_validate_value (const std::string &type, const std::string &value)
{
    auto iter = TYPE_VALIDATORS.find (type);

    if (iter != TYPE_VALIDATORS.end ())
        return iter->second (value);

    return true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Format value based on its type
// @param type Type of value to format
// @param value Value to format
// @return Formatted value as a string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
_format_value (const std::string &type, const std::string &value)
{
    auto iter = TYPE_FORMATTERS.find (type);
    if (iter != TYPE_FORMATTERS.end ())
        return iter->second (value);

    return value;
}

} // namespace

namespace mobius::extension
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Post-processor implementation constructor
// @param coordinator Post-processor coordinator
// @param item Case item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
post_processor_impl::post_processor_impl (
    mobius::framework::ant::post_processor_coordinator &coordinator,
    const mobius::framework::model::item &item
)
    : coordinator_ (coordinator),
      item_ (item)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    auto app = mobius::core::application ();
    auto path = app.get_data_path ("pdi_autofill.txt");

    // Load the autofill configuration
    std::ifstream file (path);
    if (!file)
    {
        log.error (__LINE__, "Failed to open autofill configuration file");
        return;
    }

    std::string line;
    while (std::getline (file, line))
    {
        if (!line.empty () && line[0] != '#')
        {
            auto pos = line.find ('\t');

            if (pos != std::string::npos)
            {
                auto key = line.substr (0, pos);
                auto value = line.substr (pos + 1);
                FIELDS.emplace (key, value);
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Process evidence
// @param evidence Evidence to process
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
post_processor_impl::process_evidence (
    mobius::framework::model::evidence evidence
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Check if the evidence type is "autofill"
    if (evidence.get_type () != "autofill")
        return;

    // Get the field name and value attributes
    const auto field_name = evidence.get_attribute<std::string> ("field_name");
    const auto value = evidence.get_attribute<std::string> ("value");

    if (field_name.empty () || value.empty ())
        return;

    // Check if the field name is known
    const auto l_field_name = mobius::core::string::tolower (field_name);
    auto range = FIELDS.equal_range (l_field_name);
    bool handled = false;

    for (auto iter = range.first; iter != range.second; ++iter)
    {
        auto type = iter->second;

        if (_validate_value (type, value))
        {
            auto e = item_.new_evidence ("pdi");
            e.set_attribute ("pdi_type", type);
            e.set_attribute ("value", _format_value (type, value));

            mobius::core::pod::map metadata = {
                {"username", evidence.get_attribute<std::string> ("username")},
                {"app_name", evidence.get_attribute<std::string> ("app_name")},
                {"field_name",
                 evidence.get_attribute<std::string> ("field_name")},
            };

            e.set_attribute ("metadata", metadata);
            e.add_source (evidence);

            // Notify the coordinator about the new evidence
            coordinator_.on_new_evidence (e);
            handled = true;
        }
    }

    // If field name is not known, use PDI validators to gather new field names
    if (!handled)
    {
        for (const auto &validator : TYPE_VALIDATORS)
        {
            if (validator.second (value))
                log.development (
                    __LINE__,
                    validator.first +
                        " value found. Autofill field_name=" + field_name
                );
        }
    }
}

} // namespace mobius::extension
