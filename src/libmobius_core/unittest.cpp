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
#include <iostream>
#include <mobius/core/unittest.hpp>

namespace mobius::core
{
std::size_t unittest::global_count_ = 0;
std::size_t unittest::global_errors_ = 0;
std::size_t unittest::global_unittests_ = 0;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
unittest::unittest (const std::string& title)
  : title_ (title), count_ (0), errors_ (0)
{
  global_unittests_++;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief terminate test set
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
unittest::end ()
{
  std::size_t dots = title_.length() > 60 ? 0 : 60 - title_.length();
  std::cout << "\033[1;39m" << title_ << "\033[0m " << std::string (dots, '.');

  if (!errors_)
    std::cout << " \033[1;32mOK";
  else
    std::cout << " \033[1;31mERROR";

  std::cout << "\033[0m (" << count_ - errors_ << '/' << count_ << ')' << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test whether condition is true
// @param b condition (boolean)
// @param line code line number
// @param e expression
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
unittest::assert_true (bool b, std::size_t line, const char *e)
{
  count_++;
  global_count_++;

  if (!b)
    {
      std::cout << "\033[1;31mERROR:\033[0m " << e << " == true failed at line " << line << std::endl;
      errors_++;
      global_errors_++;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief test whether condition is false
// @param b condition (boolean)
// @param line code line number
// @param e expression
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
unittest::assert_false (bool b, std::size_t line, const char *e)
{
  count_++;
  global_count_++;

  if (b)
    {
      std::cout << "\033[1;31mERROR:\033[0m " << e << " == false failed at line " << line << std::endl;
      errors_++;
      global_errors_++;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief test whether two expressions are equal
// @param b condition (boolean)
// @param line code line number
// @param e1 first expression
// @param e2 second expression
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
unittest::assert_equal (bool b, std::size_t line, const char *e1, const char *e2)
{
  count_++;
  global_count_++;

  if (!b)
    {
      std::cout << "\033[1;31mERROR:\033[0m " << e1 << " == " << e2 << " failed at line " << line << std::endl;
      errors_++;
      global_errors_++;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief test whether two expressions are not equal
// @param b condition (boolean)
// @param line code line number
// @param e1 first expression
// @param e2 second expression
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
unittest::assert_not_equal (bool b, std::size_t line, const char *e1, const char *e2)
{
  count_++;
  global_count_++;

  if (b)
    {
      std::cout << "\033[1;31mERROR:\033[0m " << e1 << " != " << e2 << " failed at line " << line << std::endl;
      errors_++;
      global_errors_++;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief test whether an expression throws an exception
// @param b whether expression threw an exception
// @param line code line number
// @param e expression
// @param exc exception
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
unittest::assert_exception (int rc, std::size_t line, const char *e, const char *exc)
{
  count_++;
  global_count_++;

  if (rc)
    {
      errors_++;
      global_errors_++;

      if (rc == 1)
        std::cout << "\033[1;31mERROR:\033[0m " << e << " did not throw exception " << exc << " at line " << line << std::endl;

      else
        std::cout << "\033[1;31mERROR:\033[0m " << e << " threw a different exception " << exc << " at line " << line << std::endl;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief generate final summary
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
unittest::final_summary ()
{
  std::cout << std::endl;
  std::cout << "unittests: " << global_unittests_ << std::endl;
  std::cout << "tests    : " << global_count_ << std::endl;
  std::cout << "errors   : " << global_errors_ << std::endl;
}

} // namespace mobius::core
