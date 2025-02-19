#ifndef MOBIUS_UNITTEST_H
#define MOBIUS_UNITTEST_H

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
#include <string>
#include <cstddef>

#define ASSERT_TRUE(b) assert_true (b, __LINE__, #b)
#define ASSERT_FALSE(b) assert_false (b, __LINE__, #b)
#define ASSERT_EQUAL(e1,e2) assert_equal ((e1) == (e2), __LINE__, #e1, #e2)
#define ASSERT_NOT_EQUAL(e1,e2) assert_not_equal ((e1) == (e2), __LINE__, #e1, #e2)
#define ASSERT_EXCEPTION(t,e,exc)                                       \
  {                                                                     \
    int rc;                                                             \
    try {                                                               \
        e;                                                              \
        rc = 1;                                                         \
      }                                                                 \
    catch (const exc&) {                                                       \
        rc = 0;                                                         \
      }                                                                 \
    catch (...) {                                                       \
        rc = 2;                                                         \
      }                                                                 \
    t.assert_exception (rc, __LINE__, #e, #exc);                        \
  }

namespace mobius
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief unittest class
//! \author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class unittest
{
  static std::size_t global_count_;
  static std::size_t global_errors_;
  static std::size_t global_unittests_;

  std::string title_;
  std::size_t count_;
  std::size_t errors_;

public:
  explicit unittest (const std::string&);
  void assert_true (bool, std::size_t, const char *);
  void assert_false (bool, std::size_t, const char *);
  void assert_equal (bool, std::size_t, const char *, const char *);
  void assert_not_equal (bool, std::size_t, const char *, const char *);
  void assert_exception (int, std::size_t, const char *, const char *);
  void end ();
  static void final_summary ();
};

} // namespace mobius

#endif
