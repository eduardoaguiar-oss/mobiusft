// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "evidence_processor_impl.hpp"
#include <mobius/core/log.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/core/string_functions.hpp>
#include <format>
#include <unordered_map>
#include <string>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Return formatted CPF if valid, empty string otherwise
// @param field_name Name of the field associated with the CPF
// @param text Text to validate as CPF
// @return Validated CPF string or empty string if invalid
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
_cpf_validator (const std::string &, const std::string &text)
{
    if (mobius::core::string::is_formatted_cpf (text))
        return text;

    if (mobius::core::string::is_numeric_cpf (text))
    {
        return text.substr (0, 3) + "." + text.substr (3, 3) + "." +
               text.substr (6, 3) + "-" + text.substr (9, 2);
    }

    return {};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Return formatted CNPJ if valid, empty string otherwise
// @param field_name Name of the field associated with the CNPJ
// @param text Text to validate as CNPJ
// @return Validated CNPJ string or empty string if invalid
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
_cnpj_validator (const std::string &, const std::string &text)
{
    if (mobius::core::string::is_formatted_cnpj (text))
        return text;

    if (mobius::core::string::is_numeric_cnpj (text))
    {
        return text.substr (0, 2) + "." + text.substr (2, 3) + "." +
               text.substr (5, 3) + "/" + text.substr (8, 4) + "-" +
               text.substr (12, 2);
    }

    return {};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Return e-mail if valid, empty string otherwise
// @param field_name Name of the field associated with the e-mail
// @param text Text to validate as e-mail
// @return Validated e-mail string or empty string if invalid
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
_email_validator (const std::string &field_name, const std::string &text)
{
    const std::string lower_field_name =
        mobius::core::string::tolower (field_name);

    if (lower_field_name.find ("email") != std::string::npos &&
        mobius::core::string::is_email (text))
        return text;

    return {};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Value validators
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::unordered_multimap<
    std::string,
    std::function<std::string (const std::string &, const std::string &)>>
    VALUE_VALIDATORS = {
        {"cpf", _cpf_validator},
        {"cnpj", _cnpj_validator},
        {"email", _email_validator},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Validate value
// @brief type Type of value to validate
// @param value Value to validate
// @return Type, formatted value if valid, empty string otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::pair<std::string, std::string>
_validate_value (const std::string &field_name, const std::string &value)
{
    // Check if the value is valid for any known field name and derive PDI evidence
    for (const auto &[type, validator] : VALUE_VALIDATORS)
    {
        auto validated_value = validator (field_name, value);

        if (!validated_value.empty ())
            return {type, validated_value};
    }

    return {"", ""};
}

} // namespace

namespace mobius::extension::evidence_processor::derived_pdis
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Evidence-processor implementation constructor
// @param item Case item
// @param profile Evidence-processor profile
// @param mediator Evidence-processor mediator
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
evidence_processor_impl::evidence_processor_impl (
    const mobius::framework::model::item &item,
    const mobius::framework::evidence_processor::profile &,
    const mobius::framework::evidence_processor::mediator &mediator
)
    : item_ (item),
      mediator_ (mediator)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Handle evidence creation
// @param e Evidence to process
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::on_evidence_created (
    mobius::framework::model::evidence e
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        evidences_processed_++;

        if (e.get_type () == "autofill")
            _process_autofill (e);

        else if (e.get_type () == "searched-text")
            _process_searched_text (e);
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Handle evidence attribute modification
// @param e Evidence being modified
// @param attr_id ID of the modified attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::on_evidence_attribute_modified (
    mobius::framework::model::evidence e, const std::string &attr_id
)
{
    if (e.get_type () == "autofill" && attr_id == "value")
        _process_autofill (e);

    else if (e.get_type () == "searched-text" && attr_id == "text")
        _process_searched_text (e);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Handle processing stop event
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::on_stop ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    log.info (
        __LINE__, std::format (
                      "Evidences derived/processed: {} of {}",
                      evidences_derived_.load (), evidences_processed_.load ()
                  )
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Process autofill evidence
// @param e Evidence to process
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::_process_autofill (
    mobius::framework::model::evidence e
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        const auto field_name = e.get_attribute<std::string> ("field_name");
        const auto value = e.get_attribute<std::string> ("value");
        auto [pdi_type, pdi_value] = _validate_value (field_name, value);

        if (!pdi_type.empty ())
        {
            auto pdi_e = item_.new_evidence ("pdi");

            pdi_e.set_attribute ("pdi_type", pdi_type);
            pdi_e.set_attribute ("value", pdi_value);

            mobius::core::pod::map metadata = {
                {"field_name", field_name},
                {"original_value", value},
                {"username", e.get_attribute<std::string> ("username")},
                {"app_name", e.get_attribute<std::string> ("app_name")},
            };

            pdi_e.set_attribute ("metadata", metadata);
            pdi_e.add_source (e);

            // Notify the coordinator about the new evidence
            mediator_.on_evidence_created (pdi_e);
            evidences_derived_++;
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Process searched-text evidence
// @param e Evidence to process
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::_process_searched_text (
    mobius::framework::model::evidence e
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        const auto search_type = e.get_attribute<std::string> ("search_type");
        const auto text = e.get_attribute<std::string> ("text");
        auto [pdi_type, pdi_value] = _validate_value (search_type, text);

        if (!pdi_type.empty ())
        {
            auto pdi_e = item_.new_evidence ("pdi");

            pdi_e.set_attribute ("pdi_type", pdi_type);
            pdi_e.set_attribute ("value", pdi_value);

            mobius::core::pod::map metadata = {
                {"search_type", search_type},
                {"text", text},
                {"username", e.get_attribute<std::string> ("username")},
                {"timestamp", e.get_attribute ("timestamp")},
            };

            pdi_e.set_attribute ("metadata", metadata);
            pdi_e.add_source (e);

            // Notify the coordinator about the new evidence
            mediator_.on_evidence_created (pdi_e);
            evidences_derived_++;
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::evidence_processor::derived_pdis
