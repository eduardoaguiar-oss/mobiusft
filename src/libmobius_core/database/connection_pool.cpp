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
#include <mobius/core/database/connection_pool.hpp>
#include <mobius/core/database/connection.hpp>
#include <mobius/core/database/database.hpp>
#include <mobius/core/exception.inc>
#include <chrono>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>

namespace mobius::core::database
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief main thread ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::thread::id main_thread_id = std::this_thread::get_id ();
} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Implementation class for connection_pool
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class connection_pool::impl
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl () = default;
  impl (const std::string&, unsigned int);
  impl (impl&&) = delete;
  impl (const impl&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  database get_database ();
  void set_path (const std::string&);
  void acquire ();
  void release ();

private:
  // @brief database file path
  std::string path_;

  // @brief max connections for this pool
  unsigned int max_ = 32;

  // @brief pool mutex
  std::mutex mutex_;

  // @brief pool of database objects
  std::unordered_map <std::thread::id, database> pool_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param path Database path
// @param max Maximum number of simultaneous connections
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
connection_pool::impl::impl (const std::string& path, unsigned int max)
  : path_ (path), max_ (max)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set database path
// @param path Database path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
connection_pool::impl::set_path (const std::string& path)
{
  if (!path_.empty ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("Database path cannot be changed"));

  path_ = path;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get database object
// @return Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
database
connection_pool::impl::get_database ()
{
  if (path_.empty ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("Database path not set"));

  // main thread: acquire connection if necessary
  auto thread_id = std::this_thread::get_id ();

  if (thread_id == main_thread_id)
    acquire ();

  // get database object
  std::lock_guard <std::mutex> lock (mutex_);

  auto iter = pool_.find (thread_id);

  if (iter == pool_.end ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("No acquired connection found"));

  return iter->second;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Acquire a connection
// @return A connection object for this thread
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
connection_pool::impl::acquire ()
{
  if (path_.empty ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("Database path not set"));

  auto thread_id = std::this_thread::get_id ();

  std::lock_guard <std::mutex> lock (mutex_);
  auto iter = pool_.find (thread_id);

  // main thread: create connection if necessary
  if (thread_id == main_thread_id)
    {
      if (iter == pool_.end ())
        pool_[thread_id] = database (path_);
    }

  // secondary threads: create only one connection per thread
  else
    {
      if (iter != pool_.end ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("Connection has already been acquired"));

      pool_[thread_id] = database (path_);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Release connection
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
connection_pool::impl::release ()
{
  auto thread_id = std::this_thread::get_id ();

  // release only connections from secondary threads
  if (thread_id != main_thread_id)
    {
      std::lock_guard <std::mutex> lock (mutex_);
      pool_.erase (thread_id);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
connection_pool::connection_pool ()
  : impl_ (std::make_shared <impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create connection_pool object
// @param path database file path
// @param max maximum number of connections opened
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
connection_pool::connection_pool (
  const std::string& path,
  unsigned int max)
  : impl_ (std::make_shared <impl> (path, max))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set database path
// @param path database file path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
connection_pool::set_path (const std::string& path)
{
  impl_->set_path (path);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Acquire a connection
// @return A connection object for this thread
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
connection
connection_pool::acquire ()
{
  impl_->acquire ();
  return connection (*this);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get database
// @return Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
database
connection_pool::get_database () const
{
  return impl_->get_database ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Release connection
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
connection_pool::release ()
{
  impl_->release ();
}

} // namespace mobius::core::database


