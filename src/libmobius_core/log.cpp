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
#include <fstream>
#include <mobius/core/log.hpp>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace mobius::core
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Log implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class log_impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    log_impl (const log_impl &) = delete;
    log_impl (log_impl &&) = delete;
    log_impl () = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    log_impl &operator= (const log_impl &) = delete;
    log_impl &operator= (log_impl &&) = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Function prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void error (const std::string &, const std::string &, std::size_t,
                const std::string &);
    void warning (const std::string &, const std::string &, std::size_t,
                  const std::string &);
    void info (const std::string &, const std::string &, std::size_t,
               const std::string &);
    void development (const std::string &, const std::string &, std::size_t,
                      const std::string &);
    void debug (const std::string &, const std::string &, std::size_t,
                const std::string &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if an error occurred
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    has_errors () const
    {
        return has_errors_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Set debug (on/off)
    // @param flag (true/false)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    set_debug (bool flag)
    {
        debug_flag_ = flag;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get events
    // @return Event list
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<event>
    get_events () const
    {
        return events_;
    }

  private:
    // @brief Has error flag
    bool has_errors_ = false;

    // @brief Debug flag
    bool debug_flag_ = false;

    // @brief Events
    std::vector<event> events_;
};

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Control variables
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Log file full path
static std::string log_path_;

// @brief Write mutex
static std::mutex log_mutex_;

// @brief Log implementation map per thread
static std::unordered_map<std::thread::id, std::shared_ptr<log_impl>> impl_map_;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Write event to log file
// @param type Event type
// @param filename Source file name
// @param funcname Source function name
// @param line_number Source file line number
// @param text Text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
_write_log (const std::string &type, const std::string &filename,
            const std::string &funcname, std::size_t line_number,
            const std::string &text)
{
    auto now = mobius::core::datetime::now ();

    std::lock_guard<std::mutex> lock (log_mutex_);

    std::ofstream out (log_path_, std::ios_base::app);

    out << now << ' ' << type << " " << funcname << '@' << filename << ':'
        << line_number << " " << text << std::endl;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set log file path
// @param path Log file path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_logfile_path (const std::string &path)
{
    std::lock_guard<std::mutex> lock (log_mutex_);
    log_path_ = path;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param type Event type
// @param filename File name
// @param funcname Function name
// @param line_number Line number
// @param text Text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
event::event (event::type type, const std::string &filename,
              const std::string &funcname, std::size_t line_number,
              const std::string &text)
    : type_ (type),
      filename_ (filename),
      funcname_ (funcname),
      line_number_ (line_number),
      text_ (text),
      timestamp_ (mobius::core::datetime::now ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add error event
// @param filename Source file name
// @param funcname Source function name
// @param line_number Source file line number
// @param text Text message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
log_impl::error (const std::string &filename, const std::string &funcname,
                 std::size_t line_number, const std::string &text)
{
    events_.emplace_back (event::type::error, filename, funcname, line_number,
                          text);
    _write_log ("ERR", filename, funcname, line_number, text);

    has_errors_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add warning event
// @param filename Source file name
// @param funcname Source function name
// @param line_number Source file line number
// @param text Text message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
log_impl::warning (const std::string &filename, const std::string &funcname,
                   std::size_t line_number, const std::string &text)
{
    events_.emplace_back (event::type::warning, filename, funcname, line_number,
                          text);
    _write_log ("WRN", filename, funcname, line_number, text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add info event
// @param filename Source file name
// @param funcname Source function name
// @param line_number Source file line number
// @param text Text message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
log_impl::info (const std::string &filename, const std::string &funcname,
                std::size_t line_number, const std::string &text)
{
    events_.emplace_back (event::type::info, filename, funcname, line_number,
                          text);
    _write_log ("INF", filename, funcname, line_number, text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add development event
// @param filename Source file name
// @param funcname Source function name
// @param line_number Source file line number
// @param text Text message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
log_impl::development (const std::string &filename, const std::string &funcname,
                       std::size_t line_number, const std::string &text)
{
    events_.emplace_back (event::type::development, filename, funcname,
                          line_number, text);

    _write_log ("DEV", filename, funcname, line_number, text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add debug event
// @param filename Source file name
// @param funcname Source function name
// @param line_number Source file line number
// @param text Text message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
log_impl::debug (const std::string &filename, const std::string &funcname,
                 std::size_t line_number, const std::string &text)
{
    if (debug_flag_)
        events_.emplace_back (event::type::debug, filename, funcname,
                              line_number, text);

    _write_log ("DBG", filename, funcname, line_number, text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param file_name Source file name
// @param function_name Source function name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
log::log (const std::string &filename, const std::string &funcname)
    : filename_ (filename),
      funcname_ (funcname)
{
    auto thread_id = std::this_thread::get_id ();

    std::lock_guard<std::mutex> lock (log_mutex_);

    auto iter = impl_map_.find (thread_id);

    if (iter == impl_map_.end ())
    {
        impl_ = std::make_shared<log_impl> ();
        impl_map_[thread_id] = impl_;
    }

    else
        impl_ = iter->second;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
log::~log ()
{
    // Remove impl from implementation map, as it is the last
    if (impl_.use_count () <= 2)
    {
        auto thread_id = std::this_thread::get_id ();
        std::lock_guard<std::mutex> lock (log_mutex_);
        impl_map_.erase (thread_id);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add error event
// @param line_number Source file line number
// @param text Text message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
log::error (std::size_t line_number, const std::string &text)
{
    impl_->error (filename_, funcname_, line_number, text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add warning event
// @param line_number Source file line number
// @param text Text message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
log::warning (std::size_t line_number, const std::string &text)
{
    impl_->warning (filename_, funcname_, line_number, text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add info event
// @param line_number Source file line number
// @param text Text message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
log::info (std::size_t line_number, const std::string &text)
{
    impl_->info (filename_, funcname_, line_number, text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add development event
// @param line_number Source file line number
// @param text Text message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
log::development (std::size_t line_number, const std::string &text)
{
    impl_->development (filename_, funcname_, line_number, text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add debug event
// @param line_number Source file line number
// @param text Text message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
log::debug (std::size_t line_number, const std::string &text)
{
    impl_->debug (filename_, funcname_, line_number, text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if an error occurred
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
log::has_errors () const
{
    return impl_->has_errors ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set debug mode (on/off)
// @param flag (true/false)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
log::set_debug (bool flag)
{
    impl_->set_debug (flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get events
// @return Event list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<event>
log::get_events () const
{
    return impl_->get_events ();
}

} // namespace mobius::core
