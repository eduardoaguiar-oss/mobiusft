#ifndef LIBMOBIUS_PYTHON_CORE_GENERATOR_HPP
#define LIBMOBIUS_PYTHON_CORE_GENERATOR_HPP

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
#include <mobius/core/generator.hpp>
#include <optional>
#include <pymobius.hpp>

namespace mobius::py
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type-erased state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class pymobius_core_generator_state
{
  public:
    virtual ~pymobius_core_generator_state () = default;

    // @return New reference; nullptr at end (no error) or on error (error set)
    virtual PyObject *next () = 0;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief State for generator<T>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T> class pymobius_core_generator_state_impl final : public pymobius_core_generator_state
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Constructor
    // @param gen Generator to wrap
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit pymobius_core_generator_state_impl (mobius::core::generator<T> g)
        : generator_ (std::move (g))
    {
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get next value from generator
    // @return New reference; nullptr at end (no error) or on error (error set)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    PyObject *
    next () final
    {
        if (!iter_)
            iter_.emplace (generator_.begin ()); // first value fetched here
        else
            ++*iter_;

        if (*iter_ == std::default_sentinel)
            return nullptr;

        return mobius::py::to_pyobject (**iter_);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get underlying generator
    // @return Copy of the wrapped generator
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::generator<T>
    get_generator () const
    {
        return generator_;
    }

  private:
    // @brief Wrapped generator
    mobius::core::generator<T> generator_;

    // @brief Iterator for the generator
    std::optional<typename mobius::core::generator<T>::iterator> iter_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Data structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct pymobius_core_generator_o
{
    PyObject_HEAD // Python object header
        pymobius_core_generator_state *obj;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Helper functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject *get_core_generator_type ();
bool pymobius_core_generator_check (PyObject *);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.generator</i> Python object from C++ object
// @param obj C++ object
// @return New mobius.core.generator object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T>
PyObject *
to_pyobject (const mobius::core::generator<T> &generator)
{
    auto self = PyObject_New (pymobius_core_generator_o, get_core_generator_type ());
    if (!self)
        return nullptr;

    try
    {
        self->obj = nullptr;
        self->obj = new pymobius_core_generator_state_impl<T> (generator);
    }
    catch (const std::exception &e)
    {
        Py_DECREF (self);
        set_runtime_error (e.what ());
        return nullptr;
    }

    return reinterpret_cast<PyObject *> (self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create mobius::core::generator from PyObject
// @param value Python Object
// @return mobius::core::generator object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T> struct pyobject_converter<mobius::core::generator<T>>
{
    static mobius::core::generator<T>
    from (PyObject *value)
    {
        if (!pymobius_core_generator_check (value))
            throw std::invalid_argument ("Expected mobius.core.generator object");

        auto py_value = reinterpret_cast<pymobius_core_generator_o *> (value);

        if (!py_value->obj)
            throw std::runtime_error ("mobius.core.generator is exhausted");

        auto state = dynamic_cast<pymobius_core_generator_state_impl<T> *> (py_value->obj);

        if (!state)
            throw std::invalid_argument ("mobius.core.generator has a different element type");

        return state->get_generator ();
    }
};

} // namespace mobius::py

#endif