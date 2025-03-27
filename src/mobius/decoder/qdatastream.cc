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
#include "qdatastream.h"
#include <mobius/datetime/conv_julian.h>
#include <mobius/exception.inc>
#include <stdexcept>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr int QT_LocalTime = 0;
constexpr int QT_UTC = 1;
constexpr int QT_OffsetFromUTC = 2;
constexpr int QT_TimeZone = 3;
} // namespace

namespace mobius::decoder
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param version Serialization version
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
qdatastream::qdatastream (const mobius::io::reader& reader, std::uint32_t version)
  : decoder_ (reader), version_ (version)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode QString
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
qdatastream::get_qstring ()
{
  std::string s;

  auto size = decoder_.get_uint32_be ();

  if (size != 0xffffffff)
    s = decoder_.get_string_by_size (size, "UTF-16BE");

  return s;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Qbytearray
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
qdatastream::get_qbytearray ()
{
  mobius::bytearray data;

  auto size = decoder_.get_uint32_be ();

  if (size != 0xffffffff)
    data = decoder_.get_bytearray_by_size (size);

  return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode QDate
// @return value
// @see https://github.com/qt/qtbase/blob/dev/src/gui/image/qdatetime.cpp (operator>>)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::date
qdatastream::get_qdate ()
{
  mobius::datetime::date dt;

  if (version_ < QT_5_0)
    {
      auto value = decoder_.get_uint32_be ();
      if (value)
        dt = mobius::datetime::date_from_julian (value);
    }

  else
    {
      auto value = decoder_.get_int64_be ();
      dt = mobius::datetime::date_from_julian (value);
    }

  return dt;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode QTime
// @return value
// @see https://github.com/qt/qtbase/blob/dev/src/gui/image/qdatetime.cpp (operator>>)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::time
qdatastream::get_qtime ()
{
  mobius::datetime::time tm;

  auto value = decoder_.get_uint32_be ();

  if (version_ < QT_4_0)
    {
      if (value)
        tm.from_day_seconds (value / 1000);
    }

  else
    tm.from_day_seconds (value / 1000);

  return tm;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode QDateTime
// @return value
// @see https://github.com/qt/qtbase/blob/dev/src/gui/image/qdatetime.cpp (operator>>)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
qdatastream::get_qdatetime ()
{
  mobius::datetime::datetime d;

  if (version_ >= QT_5_2)
    {
      throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("qdatetime serialization not implemented for QT versions >= 5.2"));
    }

  else if (version_ == QT_5_0)
    {
      auto dt = get_qdate ();
      auto tm = get_qtime ();
      auto ts = decoder_.get_uint8 ();  // all datetimes in UTC
      std::ignore = ts;

      d = mobius::datetime::datetime (dt, tm);
    }

  else if (version_ >= QT_4_0)
    {
      auto dt = get_qdate ();
      auto tm = get_qtime ();
      auto ts = decoder_.get_uint8 ();
      std::ignore = ts;

      d = mobius::datetime::datetime (dt, tm);
    }

  else
    {
      auto dt = get_qdate ();
      auto tm = get_qtime ();

      d = mobius::datetime::datetime (dt, tm);
    }

  return d;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode QImage
// @return value
// @see https://github.com/qt/qtbase/blob/dev/src/gui/image/qimage.cpp (operator>>)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
qdatastream::get_qimage ()
{
  mobius::bytearray data;

  // check if image is null
  std::int32_t data_marker = 1;

  if (version_ >= 5)
    data_marker = get_qint32 ();

  // read PNG data
  if (data_marker)
    {
      auto pos = decoder_.tell ();

      // check signature
      auto signature = decoder_.get_bytearray_by_size (8);

      if (signature != "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a")
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid PNG data"));

      // read chunks until type == IEND
      std::string type;

      do
        {
          auto length = decoder_.get_uint32_be ();
          type = decoder_.get_string_by_size (4);
          decoder_.skip (length + 4);
        } while (type != "IEND");

      // read complete data
      auto size = decoder_.tell () - pos;
      decoder_.seek (pos);
      data = decoder_.get_bytearray_by_size (size);
    }

  return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode QPixmap
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
qdatastream::get_qpixmap ()
{
  return get_qimage ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode QVariant
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::pod::data
qdatastream::get_qvariant ()
{
  mobius::pod::data data;

  auto datatype = decoder_.get_uint32_be ();
  bool is_null = decoder_.get_uint8 ();

  switch (datatype)
  {
    case 12:            // QBytearray
        data = is_null ? mobius::bytearray () : get_qbytearray ();
        break;

    default:
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("unhandled datatype: " + std::to_string (datatype)));
  };

  return data;
}

} // namespace mobius::decoder


