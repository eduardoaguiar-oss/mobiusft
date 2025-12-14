// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
// Eduardo Aguiar
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
#include <mobius/core/benchmark.hpp>

namespace mobius::core
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
benchmark::benchmark (const std::string &title, const std::string &unit)
    : title_ (title),
      unit_ (unit),
      start_ (std::chrono::high_resolution_clock::now ())
{
    std::size_t dots = title_.length () > 60 ? 0 : 60 - title_.length ();
    std::cout << "\033[1;39m" << title_ << "\033[0m " << std::string (dots, '.')
              << std::flush;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get time from the start of this benchmark
// @return time in microseconds
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
benchmark::mtime () const
{
    auto now = std::chrono::high_resolution_clock::now ();
    return std::chrono::duration_cast<std::chrono::milliseconds> (now - start_)
        .count ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief end benchmark
// @param count test count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
benchmark::end (std::uint64_t count)
{
    std::uint64_t time_ms = mtime ();

    double throughput = double (count) / (double (time_ms) / 100.0);
    const std::string units[] = {"", "K", "M", "G", "T"};
    int unit_idx = 0;

    while (throughput > 3160) // sqrt (10) * 1000
    {
        throughput /= 1000;
        unit_idx++;
    }

    std::cout << " \033[1;32m" << throughput << ' ' << units[unit_idx] << unit_
              << "/s\033[0m" << std::endl;
}

} // namespace mobius::core
