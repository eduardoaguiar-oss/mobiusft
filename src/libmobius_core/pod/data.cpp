// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
#include <mobius/core/pod/data.hpp>
#include <mobius/core/pod/data_impl_null.hpp>
#include <mobius/core/pod/data_impl_bool.hpp>
#include <mobius/core/pod/data_impl_integer.hpp>
#include <mobius/core/pod/data_impl_float.hpp>
#include <mobius/core/pod/data_impl_datetime.hpp>
#include <mobius/core/pod/data_impl_string.hpp>
#include <mobius/core/pod/data_impl_bytearray.hpp>
#include <mobius/core/pod/data_impl_list.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/exception.inc>
#include <algorithm>
#include <stdexcept>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clone data vector
// @param v Data vector
// @return New data data vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::core::pod::data>
list_clone (const std::vector <mobius::core::pod::data>& v)
{
  std::vector <mobius::core::pod::data> v2;

  for (const auto& d : v)
    v2.push_back (d.clone ());

  return v2;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert data vector to std::string
// @param v Data vector
// @return String representation of data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
list_to_string (const std::vector <mobius::core::pod::data>& v)
{
  std::string text = "[";
  bool first = true;

  for (const auto& child : v)
    {
      if (first)
        first = false;

      else
        text += ',';

      text += child.to_string ();
    }

  text += ']';

  return text;
}

} // namespace

namespace mobius::core::pod
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data ()
 : impl_ (std::make_shared <data_impl_null> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param p shared_ptr to data_impl_base
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data (const std::shared_ptr <data_impl_base>& p)
 : impl_ (p)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param b Boolean value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data (bool b)
 : impl_ (std::make_shared <data_impl_bool> (b))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param i Integer value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data (int i)
 : impl_ (std::make_shared <data_impl_integer> (i))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param i Integer value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data (std::int64_t i)
 : impl_ (std::make_shared <data_impl_integer> (i))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param i Integer value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data (std::uint64_t i)
 : impl_ (std::make_shared <data_impl_integer> (static_cast <std::int64_t> (i)))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param i Integer value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data (std::uint32_t i)
 : impl_ (std::make_shared <data_impl_integer> (static_cast <std::int64_t> (i)))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param v Float value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data (double v)
 : impl_ (std::make_shared <data_impl_float> (v))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param v Float value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data (long double v)
 : impl_ (std::make_shared <data_impl_float> (v))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param v Datetime value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data (const mobius::datetime::datetime& v)
 : impl_ (std::make_shared <data_impl_datetime> (v))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param s C string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data (const char *s)
{
  if (s)
    impl_ = std::make_shared <data_impl_string> (s);

  else
    impl_ = std::make_shared <data_impl_null> ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param s C++ string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data (const std::string& s)
 : impl_ (std::make_shared <data_impl_string> (s))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param b Bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data (const mobius::bytearray& b)
 : impl_ (std::make_shared <data_impl_bytearray> (b))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param l initializer_list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data (const std::initializer_list <data>& l)
 : impl_ (std::make_shared <data_impl_list> ())
{
  auto p = std::static_pointer_cast <data_impl_list> (impl_);

  for (const auto& d : l)
    p->append (d.impl_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param v Vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::data (const std::vector <data>& v)
 : impl_ (std::make_shared <data_impl_list> ())
{
  auto p = std::static_pointer_cast <data_impl_list> (impl_);

  for (const auto& d : v)
    p->append (d.impl_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Assignment operator
// @param b Boolean value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data&
data::operator= (bool b)
{
  impl_ = std::make_shared <data_impl_bool> (b);
  return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Assignment operator
// @param i Integer value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data&
data::operator= (int i)
{
  impl_ = std::make_shared <data_impl_integer> (i);
  return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Assignment operator
// @param i Integer value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data&
data::operator= (std::int64_t i)
{
  impl_ = std::make_shared <data_impl_integer> (i);
  return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Assignment operator
// @param v Real value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data&
data::operator= (double v)
{
  impl_ = std::make_shared <data_impl_float> (v);
  return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Assignment operator
// @param v Real value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data&
data::operator= (long double v)
{
  impl_ = std::make_shared <data_impl_float> (v);
  return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Assignment operator
// @param v Datetime value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data&
data::operator= (const mobius::datetime::datetime& v)
{
  impl_ = std::make_shared <data_impl_datetime> (v);
  return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Assignment operator
// @param s C string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data&
data::operator= (const char *s)
{
  if (s)
    impl_ = std::make_shared <data_impl_string> (s);

  else
    impl_ = std::make_shared <data_impl_null> ();

  return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Assignment operator
// @param s C++ string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data&
data::operator= (const std::string& s)
{
  impl_ = std::make_shared <data_impl_string> (s);
  return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Assignment operator
// @param b Bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data&
data::operator= (const mobius::bytearray& b)
{
  impl_ = std::make_shared <data_impl_bytearray> (b);
  return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Assignment operator
// @param l initializer_list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data&
data::operator= (const std::initializer_list <data>& l)
{
  auto p = std::make_shared <data_impl_list> ();
  impl_ = p;

  for (const auto& d : l)
    p->append (d.impl_);

  return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Assignment operator
// @param v Vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data&
data::operator= (const std::vector <data>& v)
{
  auto p = std::make_shared <data_impl_list> ();
  impl_ = p;

  for (const auto& d : v)
    p->append (d.impl_);

  return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert data to boolean
// @return Bool value if type == boolean, otherwise exception
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::operator bool () const
{
  if (impl_->get_type () == type::boolean)
    return std::static_pointer_cast <data_impl_bool> (impl_)->get_value ();

  throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot convert data to bool"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert data to int64_t
// @return int64_t value if type == integer, otherwise exception
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::operator std::int64_t () const
{
  if (impl_->get_type () == type::integer)
    return std::static_pointer_cast <data_impl_integer> (impl_)->get_value ();

  throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot convert data to std::int64_t"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert data to long double
// @return Long double value if type == real, otherwise exception
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::operator long double () const
{
  if (impl_->get_type () == type::floatn)
    return std::static_pointer_cast <data_impl_float> (impl_)->get_value ();

  throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot convert data to long double"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert data to datetime
// @return Long double value if type == datetime, otherwise exception
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::operator mobius::datetime::datetime () const
{
  if (impl_->get_type () == type::datetime)
    return std::static_pointer_cast <data_impl_datetime> (impl_)->get_value ();

  throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot convert data to datetime"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert data to string
// @return Bool value if type == string, otherwise exception
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::operator std::string () const
{
  if (impl_->get_type () == type::string)
    return std::static_pointer_cast <data_impl_string> (impl_)->get_value ();

  else if (impl_->get_type () == type::bytearray)
    return std::static_pointer_cast <data_impl_bytearray> (impl_)->get_value ().to_string ();

  else if (impl_->get_type () == type::integer)
    return std::to_string (std::static_pointer_cast <data_impl_integer> (impl_)->get_value ());

  throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot convert data to string"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert data to bytearray
// @return Bytearray value if type == bytearray, otherwise exception
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::operator mobius::bytearray () const
{
  if (impl_->get_type () == type::bytearray)
    return std::static_pointer_cast <data_impl_bytearray> (impl_)->get_value ();

  throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot convert data to bytearray"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert data to std::vector <data>
// @return Vector value if type == list, otherwise exception
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data::operator std::vector <data> () const
{
  if (impl_->get_type () == type::list)
    {
      auto p = std::static_pointer_cast <data_impl_list> (impl_);
      return std::vector <data> (p->begin (), p->end ());
    }

  throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot convert data to list"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clone object
// @return New data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data
data::clone () const
{
  if (is_null ())
    return data ();

  else if (is_bool ())
    return data (std::static_pointer_cast <data_impl_bool> (impl_)->get_value ());

  else if (is_integer ())
    return data (std::static_pointer_cast <data_impl_integer> (impl_)->get_value ());

  else if (is_float ())
    return data (std::static_pointer_cast <data_impl_float> (impl_)->get_value ());

  else if (is_datetime ())
    return data (std::static_pointer_cast <data_impl_datetime> (impl_)->get_value ());

  else if (is_string ())
    return data (std::static_pointer_cast <data_impl_string> (impl_)->get_value ());

  else if (is_bytearray ())
    return data (std::static_pointer_cast <data_impl_bytearray> (impl_)->get_value ());

  else if (is_list ())
    return list_clone (std::vector <data> (*this));

  else if (is_map ())
    return data (map (*this).clone ());

  throw std::out_of_range (MOBIUS_EXCEPTION_MSG ("unknown data type"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert data to std::string
// @return String representation of data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
data::to_string () const
{
  if (is_null ())
    return "null";

  else if (is_bool ())
    return operator bool () ? "true" : "false";

  else if (is_integer ())
    return std::to_string (operator std::int64_t ());

  else if (is_float ())
    return std::to_string (operator long double ());

  else if (is_datetime ())
    return mobius::datetime::to_string (operator mobius::datetime::datetime ());

  else if (is_string ())
    return operator std::string ();

  else if (is_bytearray ())
    return "0x" + operator mobius::bytearray ().to_hexstring ();

  else if (is_list ())
    return list_to_string (std::vector <data> (*this));

  else if (is_map ())
    return map (*this).to_string ();

  return "<unknown value>";
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Ostream inserter for data
// @param os Ostream reference
// @param d Data object
// @return Ostream reference
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::ostream&
operator<< (std::ostream& os, const mobius::core::pod::data& d)
{
  return os << d.to_string ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Compare two data objects
// @param a First object
// @param b Second object
// @return <b>true</b> if objects are equal <b>false<b> otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator== (const data& a, const data& b)
{
  bool rc = false;

  if (a.get_type () == b.get_type ())
    {
      if (a.is_null ())
        rc = true;

      else if (a.is_bool ())
        rc = (bool (a) == bool (b));

      else if (a.is_integer ())
        rc = (std::int64_t (a) == std::int64_t (b));

      else if (a.is_float ())
        rc = (static_cast <long double> (a) == static_cast <long double> (b));

      else if (a.is_datetime ())
        rc = (mobius::datetime::datetime (a) == mobius::datetime::datetime (b));

      else if (a.is_string ())
        rc = (std::string (a) == std::string (b));

      else if (a.is_bytearray ())
        rc = (bytearray (a) == bytearray (b));

      else if (a.is_list ())
        rc = (std::vector <data> (a) == std::vector <data> (b));

      else if (a.is_map ())
        rc = (map (a) == map (b));
    }

  return rc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Compare two data objects
// @param a First object
// @param b Second object
// @return <b>true</b> if objects are different <b>false<b> otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator!= (const data& a, const data& b)
{
  return ! (a == b);
}

} // namespace mobius::core::pod


