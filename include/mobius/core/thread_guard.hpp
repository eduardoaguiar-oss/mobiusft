#ifndef MOBIUS_CORE_THREAD_GUARD_HPP
#define MOBIUS_CORE_THREAD_GUARD_HPP

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
#include <any>
#include <memory>
#include <string>

namespace mobius::core
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Thread guard class
// @author Eduardo Aguiar
//
// The <i>thread_guard</i> class and the get/set/has/remove_thread_resource
// functions are designed to handle resources that are specific for each
// thread, such as sqlite3 database connections.
//
// Usage:
//   1. When starting a new thread, create an instance of thread_guard:
//        thread_guard guard;
//        ...
//        ...
//
//   2. Internal code of Mobius objects and functions calls for
//      get/set/has/remove_thread_resource functions
//
//   3. When thread_guard goes out of scope, its destructor automatically
//      deletes all resources set in that thread.
//
// Main thread must not create thread_guard instances, because a static
// instance is already created.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class thread_guard
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  thread_guard ();
  thread_guard (const thread_guard&) noexcept = default;
  thread_guard (thread_guard&&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  thread_guard& operator=(const thread_guard&) noexcept = default;
  thread_guard& operator=(thread_guard&&) noexcept = default;

private:
  // @brief Forward declaration
  class impl;

  // @brief Implementation pointer
  std::shared_ptr<impl> impl_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool has_thread_resource (const std::string&);
std::any get_thread_resource (const std::string&);
void set_thread_resource (const std::string&, const std::any&);
void remove_thread_resource (const std::string&);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get thread resource
// @param resource_id Resource ID
// @return any object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T> T
get_thread_resource (const std::string& resource_id)
{
  return std::any_cast <T> (get_thread_resource (resource_id));
}

} // namespace mobius::core

#endif


