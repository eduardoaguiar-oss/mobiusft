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
#include <mobius/bytearray.h>
#include <mobius/benchmark.h>
#include <mobius/vfs/imagefile.h>

namespace
{
static const std::string URL = "file:///tmp/benchmark.ewf";
}

static void
benchmark_imagefile_ewf ()
{
  auto f = mobius::io::new_file_by_url (URL);
  if (f.exists ())
    f.remove ();

  mobius::vfs::imagefile out (mobius::vfs::new_imagefile_by_url (URL, "ewf"));
  out.set_attribute ("segment_size", 100000000000);
  auto writer = out.new_writer ();

  mobius::benchmark benchmark ("mobius::imagefile::ewf (write)");
  mobius::bytearray data (512);
  std::uint64_t count = 0;

  while (count % 10000 || benchmark.mtime () < 1000)
    {
      data[count % 512]++;
      writer.write (data);
      count++;
    }

  benchmark.end (count * 512);
}

static void
benchmark_imagefile_ewf_fast ()
{
  auto f = mobius::io::new_file_by_url (URL);
  if (f.exists ())
    f.remove ();

  mobius::vfs::imagefile out (mobius::vfs::new_imagefile_by_url (URL, "ewf"));
  out.set_attribute ("segment_size", 100000000000);
  out.set_attribute ("compression_level", 1);
  auto writer = out.new_writer ();

  mobius::benchmark benchmark ("mobius::imagefile::ewf (write - compression=fast)");
  mobius::bytearray data (512);
  std::uint64_t count = 0;

  while (count % 10000 || benchmark.mtime () < 1000)
    {
      data[count % 512]++;
      writer.write (data);
      count++;
    }

  benchmark.end (count * 512);
}

static void
benchmark_imagefile_ewf_best ()
{
  auto f = mobius::io::new_file_by_url (URL);
  if (f.exists ())
    f.remove ();

  mobius::vfs::imagefile out (mobius::vfs::new_imagefile_by_url (URL, "ewf"));
  out.set_attribute ("segment_size", 100000000000);
  out.set_attribute ("compression_level", 9);
  auto writer = out.new_writer ();

  mobius::benchmark benchmark ("mobius::imagefile::ewf (write - compression=best)");
  mobius::bytearray data (512);
  std::uint64_t count = 0;

  while (count % 10000 || benchmark.mtime () < 1000)
    {
      data[count % 512]++;
      writer.write (data);
      count++;
    }

  benchmark.end (count * 512);
}

void
benchmark_imagefile ()
{
  benchmark_imagefile_ewf ();
  benchmark_imagefile_ewf_fast ();
  benchmark_imagefile_ewf_best ();
}
