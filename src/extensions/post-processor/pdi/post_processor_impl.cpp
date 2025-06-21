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
#include <mobius/core/log.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/core/string_functions.hpp>
#include <unordered_map>
#include <string>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Known field names
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::unordered_map<std::string, std::string> FIELDS = {
    {"accountid", "cpf"},
    {"accountholdername", "fullname"},
    {"admin_email", "email"},
    {"apps_email", "email"},
    {"billing_cpf", "cpf"},
    {"billing_email", "email"},
    {"billing_name", "fullname"},
    {"billingname", "fullname"},
    {"cadastro_email", "email"},
    {"cardholdername", "fullname"},
    {"cardholdernamefield", "fullname"},
    {"cnpj", "cnpj"},
    {"contactemail", "email"},
    {"cpf", "cpf"},
    {"cpfresponsavel", "cpf"},
    {"email", "email"},
    {"fiscal_document", "cpf"},
    {"fullname", "fullname"},
    {"inscricao", "cnpj"},
    {"invoice_fullname", "fullname"},
    {"legal_document", "cpf"},
    {"prestador.inscricao", "cnpj"},
    {"root_taxid", "cpf"},
    {"taxid", "cpf"},
    {"tomador.inscricao", "cnpj"},
    {"user-mail", "email"},
    {"vatcompanybr", "cnpj"},
};

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

} // namespace

namespace mobius::extension
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Process evidence
// @param evidence Evidence to process
// @return Processed evidences
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::framework::model::evidence>
post_processor_impl::process_evidence (
    mobius::framework::model::evidence evidence
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    std::vector<mobius::framework::model::evidence> evidences;

    // Check if the evidence type is "autofill"
    if (evidence.get_type () != "autofill")
        return {};

    // Get the field name and value attributes
    auto field_name = evidence.get_attribute<std::string> ("field_name");
    auto value = evidence.get_attribute<std::string> ("value");
    auto l_field_name = mobius::core::string::tolower (field_name);

    if (field_name.empty () || value.empty ())
        return {};

    // Check if the field name is known
    auto iter = FIELDS.find (l_field_name);
    if (iter != FIELDS.end ())
    {
        // Validate the value based on the field type
        auto type_iter = TYPE_VALIDATORS.find (iter->second);
        if (type_iter == TYPE_VALIDATORS.end () || type_iter->second (value))
        {
            auto e = item_.new_evidence ("pdi");
            e.set_attribute ("pdi_type", iter->second);
            e.set_attribute ("value", value);

            auto metadata = mobius::core::pod::map ();
            metadata.set (
                "username",
                evidence.get_attribute<std::string> ("username")
            );
            metadata.set (
                "app_name",
                evidence.get_attribute<std::string> ("app_name")
            );
            metadata.set (
                "field_name",
                evidence.get_attribute<std::string> ("field_name")
            );

            e.set_attribute ("metadata", metadata);
            e.add_source (evidence);

            // Add the new evidence to the list
            evidences.push_back (std::move (e));
        }
    }

    // If field name is not known, use PDI validators to gather new field names
    else
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

    return evidences;
}

} // namespace mobius::extension
