// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <mobius/core/thread_guard.hpp>
#include <mobius/core/exception.inc>
#include <any>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Module data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Resources by thread
static std::unordered_map<std::thread::id, std::unordered_map<std::string, std::any>> resources_;

// @brief Mutex for resources map
static std::mutex mutex_;

//! brief Main thread guard
static mobius::core::thread_guard main_thread_guard_;

} // namespace


namespace mobius::core
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Thread guard implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class thread_guard::impl
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors and destructor
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl();
  impl(const impl&) = delete;
  impl(impl&&) = delete;
  ~impl();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl& operator=(const impl&) = delete;
  impl& operator=(impl&&) = delete;

private:
  std::thread::id thread_id_;
};


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
thread_guard::impl::impl()
  : thread_id_(std::this_thread::get_id ())
{
  std::lock_guard <std::mutex> lock (mutex_);

  if (resources_.find (thread_id_) != resources_.end ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("thread_guard already created"));

  resources_[thread_id_] = {};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
thread_guard::impl::~impl()
{
  std::lock_guard <std::mutex> lock (mutex_);

  resources_.erase (thread_id_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
thread_guard::thread_guard()
  : impl_ (std::make_shared<impl> ())
{
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if thread resource exists
// @param resource_id Resource ID
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
has_thread_resource (const std::string& resource_id)
{
  auto thread_id = std::this_thread::get_id ();
  std::lock_guard <std::mutex> lock (mutex_);

  auto iter = resources_.find (thread_id);
  if (iter == resources_.end ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("thread_guard object must be created first"));

  return iter->second.find(resource_id) != iter->second.end();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get thread resource
// @param resource_id Resource ID
// @return any object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::any
get_thread_resource (const std::string& resource_id)
{
  auto thread_id = std::this_thread::get_id ();
  std::lock_guard <std::mutex> lock (mutex_);

  auto iter = resources_.find (thread_id);
  if (iter == resources_.end ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("thread_guard object must be created first"));

  std::any resource;

  auto resource_iter = iter->second.find (resource_id);
  if (resource_iter != iter->second.end())
    resource = resource_iter->second;

  return resource;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set thread resource
// @param resource_id Resource ID
// @param resource_value Resource value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_thread_resource (const std::string& resource_id, const std::any& resource_value)
{
  auto thread_id = std::this_thread::get_id ();
  std::lock_guard <std::mutex> lock (mutex_);

  auto iter = resources_.find (thread_id);
  if (iter == resources_.end ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("thread_guard object must be created first"));

  iter->second[resource_id] = resource_value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove thread resource
// @param resource_id Resource ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
remove_thread_resource (const std::string& resource_id)
{
  auto thread_id = std::this_thread::get_id ();
  std::lock_guard <std::mutex> lock (mutex_);

  auto iter = resources_.find (thread_id);
  if (iter == resources_.end ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("thread_guard object must be created first"));

  iter->second.erase (resource_id);
}

} // namespace mobius::core

