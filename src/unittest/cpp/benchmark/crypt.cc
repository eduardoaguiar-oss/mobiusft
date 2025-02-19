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
#include <mobius/crypt/hash.h>
#include <mobius/bytearray.h>
#include <mobius/benchmark.h>

static void
benchmark_hash ()
{
  const std::string H_IDS[] = {"adler32", "crc32", "md4", "md5", "sha1", "sha2-224", "sha2-256", "sha2-384", "sha2-512", "sha2-512-224", "sha2-512-256", "zip"};

  mobius::bytearray data (512);
  data.random ();

  for (const std::string& h_id : H_IDS)
    {
      mobius::benchmark benchmark ("mobius::crypt::hash -> " + h_id);
      mobius::crypt::hash h (h_id);
      std::uint64_t count = 0;

      while (count % 100000 || benchmark.mtime () < 1000)
        {
          data[count % 512]++;
          h.update (data);
          count++;
        }

      benchmark.end (count * 512);
    }
}

void
benchmark_crypt ()
{
  benchmark_hash ();
}
