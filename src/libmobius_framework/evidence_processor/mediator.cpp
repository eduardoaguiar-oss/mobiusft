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
#include <mobius/core/log.hpp>
#include <mobius/framework/evidence_processor/mediator.hpp>

namespace mobius::framework::evidence_processor
{
// ==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>mediator</i> implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class mediator::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl () = default;
    impl (const impl &) = delete;
    impl (impl &&) = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl &operator= (const impl &) = delete;
    impl &operator= (impl &&) = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Function prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void add_evidence_processor (std::shared_ptr<evidence_processor_impl_base>);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Event handlers
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void on_evidence_created (mobius::framework::model::evidence);
    void on_evidence_attribute_modified (
        mobius::framework::model::evidence, const std::string &
    );
    void on_evidence_tag_modified (
        mobius::framework::model::evidence, const std::string &
    );

  private:
    // @brief Implementations
    std::vector<std::shared_ptr<evidence_processor_impl_base>> implementations_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add evidence_processor implementation
// @param implementation Evidence processor implementation to add
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
mediator::impl::add_evidence_processor (
    std::shared_ptr<evidence_processor_impl_base> processor
)
{
    implementations_.emplace_back (std::move (processor));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Handle evidence created event
// @param evidence Reference to the created evidence
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
mediator::impl::on_evidence_created (
    mobius::framework::model::evidence evidence
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    for (const auto &impl : implementations_)
    {
        try
        {
            impl->on_evidence_created (evidence);
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, e.what ());
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Handle evidence attribute modified event
// @param evidence Reference to the modified evidence
// @param attribute_id ID of the modified attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
mediator::impl::on_evidence_attribute_modified (
    mobius::framework::model::evidence evidence,
    const std::string &attribute_id
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    for (const auto &impl : implementations_)
    {
        try
        {
            impl->on_evidence_attribute_modified (evidence, attribute_id);
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, e.what ());
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Handle evidence tag modified event
// @param evidence Reference to the modified evidence
// @param tag ID of the modified tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
mediator::impl::on_evidence_tag_modified (
    mobius::framework::model::evidence evidence, const std::string &tag
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    for (const auto &impl : implementations_)
    {
        try
        {
            impl->on_evidence_tag_modified (evidence, tag);
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, e.what ());
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mediator::mediator ()
    : impl_ (std::make_shared<impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add evidence_processor implementation
// @param implementation Processor implementation to add
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
mediator::add_evidence_processor (
    std::shared_ptr<evidence_processor_impl_base> processor
)
{
    impl_->add_evidence_processor (std::move (processor));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Handle evidence created event
// @param evidence Reference to the created evidence
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
mediator::on_evidence_created (mobius::framework::model::evidence evidence)
{
    impl_->on_evidence_created (evidence);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Handle evidence attribute modified event
// @param evidence Reference to the modified evidence
// @param attribute_id ID of the modified attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
mediator::on_evidence_attribute_modified (
    mobius::framework::model::evidence evidence,
    const std::string &attribute_id
)
{
    impl_->on_evidence_attribute_modified (evidence, attribute_id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Handle evidence tag modified event
// @param evidence Reference to the modified evidence
// @param tag ID of the modified tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
mediator::on_evidence_tag_modified (
    mobius::framework::model::evidence evidence, const std::string &tag
)
{
    impl_->on_evidence_tag_modified (evidence, tag);
}

} // namespace mobius::framework::evidence_processor