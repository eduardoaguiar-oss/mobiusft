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
#include <mobius/decoder/data_decoder.h>
#include <mobius/bytearray.h>
#include <mobius/benchmark.h>
#include <mobius/io/bytearray_io.h>

static void
benchmark_decoder_uint8 ()
{
  constexpr int SIZE = 100000000;
  mobius::bytearray data (SIZE);
  data.random ();
  mobius::decoder::data_decoder decoder (data);

  mobius::benchmark benchmark ("mobius::decoder::data_decoder::get_uint8");
  std::uint64_t count = 0;
  std::uint64_t sum = 0;
  mobius::bytearray::size_type size = SIZE;
  mobius::bytearray::size_type pos = 0;

  while (pos < size && (count % 1000000 || benchmark.mtime () < 1000))
    {
      sum += decoder.get_uint8 ();
      count++;
      pos++;
    }

  benchmark.end (count);
}

static void
benchmark_decoder_uint16_le ()
{
  constexpr int SIZE = 100000000;
  mobius::bytearray data (SIZE);
  data.random ();
  mobius::decoder::data_decoder decoder (data);

  mobius::benchmark benchmark ("mobius::decoder::data_decoder::get_uint16_le");
  std::uint64_t count = 0;
  std::uint64_t sum = 0;
  mobius::bytearray::size_type size = SIZE;
  mobius::bytearray::size_type pos = 0;

  while (pos < size && (count % 1000000 || benchmark.mtime () < 1000))
    {
      sum += decoder.get_uint16_le ();
      count++;
      pos += 2;
    }

  benchmark.end (count * 2);
}

static void
benchmark_decoder_uint16_be ()
{
  constexpr int SIZE = 100000000;
  mobius::bytearray data (SIZE);
  data.random ();
  mobius::decoder::data_decoder decoder (data);

  mobius::benchmark benchmark ("mobius::decoder::data_decoder::get_uint16_be");
  std::uint64_t count = 0;
  std::uint64_t sum = 0;
  mobius::bytearray::size_type size = SIZE;
  mobius::bytearray::size_type pos = 0;

  while (pos < size && (count % 1000000 || benchmark.mtime () < 1000))
    {
      sum += decoder.get_uint16_be ();
      count++;
      pos += 2;
    }

  benchmark.end (count * 2);
}

static void
benchmark_decoder_uint32_le ()
{
  constexpr int SIZE = 100000000;
  mobius::bytearray data (SIZE);
  data.random ();
  mobius::decoder::data_decoder decoder (data);

  mobius::benchmark benchmark ("mobius::decoder::data_decoder::get_uint32_le");
  std::uint64_t count = 0;
  std::uint64_t sum = 0;
  mobius::bytearray::size_type size = SIZE;
  mobius::bytearray::size_type pos = 0;

  while (pos < size && (count % 1000000 || benchmark.mtime () < 1000))
    {
      sum += decoder.get_uint32_le ();
      count++;
      pos += 4;
    }

  benchmark.end (count * 4);
}

static void
benchmark_decoder_uint32_be ()
{
  constexpr int SIZE = 100000000;
  mobius::bytearray data (SIZE);
  data.random ();
  mobius::decoder::data_decoder decoder (data);

  mobius::benchmark benchmark ("mobius::decoder::data_decoder::get_uint32_be");
  std::uint64_t count = 0;
  std::uint64_t sum = 0;
  mobius::bytearray::size_type size = SIZE;
  mobius::bytearray::size_type pos = 0;

  while (pos < size && (count % 1000000 || benchmark.mtime () < 1000))
    {
      sum += decoder.get_uint32_be ();
      count++;
      pos += 4;
    }

  benchmark.end (count * 4);
}

static void
benchmark_decoder_uint64_le ()
{
  constexpr int SIZE = 100000000;
  mobius::bytearray data (SIZE);
  data.random ();
  mobius::decoder::data_decoder decoder (data);

  mobius::benchmark benchmark ("mobius::decoder::data_decoder::get_uint64_le");
  std::uint64_t count = 0;
  std::uint64_t sum = 0;
  mobius::bytearray::size_type size = SIZE;
  mobius::bytearray::size_type pos = 0;

  while (pos < size && (count % 1000000 || benchmark.mtime () < 1000))
    {
      sum += decoder.get_uint64_le ();
      count++;
      pos += 8;
    }

  benchmark.end (count * 8);
}

static void
benchmark_decoder_uint64_be ()
{
  constexpr int SIZE = 100000000;
  mobius::bytearray data (SIZE);
  data.random ();
  mobius::decoder::data_decoder decoder (data);

  mobius::benchmark benchmark ("mobius::decoder::data_decoder::get_uint64_be");
  std::uint64_t count = 0;
  std::uint64_t sum = 0;
  mobius::bytearray::size_type size = SIZE;
  mobius::bytearray::size_type pos = 0;

  while (pos < size && (count % 1000000 || benchmark.mtime () < 1000))
    {
      sum += decoder.get_uint64_be ();
      count++;
      pos += 8;
    }

  benchmark.end (count * 8);
}

/*
mobius::decoder::data_decoder::get_uint8 .................... 647.424 KiB/s
mobius::decoder::data_decoder::get_uint16_le ................ 1287 KiB/s
mobius::decoder::data_decoder::get_uint16_be ................ 1277.14 KiB/s
mobius::decoder::data_decoder::get_uint32_le ................ 2427.18 KiB/s
mobius::decoder::data_decoder::get_uint32_be ................ 2380.95 KiB/s
mobius::decoder::data_decoder::get_uint64_le ................ 4.69484 MiB/s
mobius::decoder::data_decoder::get_uint64_be ................ 4.62963 MiB/s

2018-03-26 - using raw uint8_t pointers
mobius::decoder::data_decoder::get_uint8 .................... 1306.41 KiB/s
mobius::decoder::data_decoder::get_uint16_le ................ 2710.4 KiB/s
mobius::decoder::data_decoder::get_uint16_be ................ 2722.45 KiB/s
mobius::decoder::data_decoder::get_uint32_le ................ 5.41884 MiB/s
mobius::decoder::data_decoder::get_uint32_be ................ 5.43326 MiB/s
mobius::decoder::data_decoder::get_uint64_le ................ 10.3306 MiB/s
mobius::decoder::data_decoder::get_uint64_be ................ 10.582 MiB/s
 */

void
benchmark_decoder ()
{
  benchmark_decoder_uint8 ();
  benchmark_decoder_uint16_le ();
  benchmark_decoder_uint16_be ();
  benchmark_decoder_uint32_le ();
  benchmark_decoder_uint32_be ();
  benchmark_decoder_uint64_le ();
  benchmark_decoder_uint64_be ();
}
