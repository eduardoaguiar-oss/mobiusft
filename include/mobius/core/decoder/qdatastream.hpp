#ifndef MOBIUS_CORE_DECODER_QDATASTREAM_HPP
#define MOBIUS_CORE_DECODER_QDATASTREAM_HPP

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
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/pod/data.hpp>

namespace mobius::core::decoder
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief QDataStream decoder
// @author Eduardo Aguiar
// @see https://surfer.nmr.mgh.harvard.edu/ftp/dist/freesurfer/tutorial_versions/freesurfer/lib/qt/qt_doc/html/datastreamformat.html
// @see https://code.qt.io/cgit/qt/qtbase.git/tree/src/corelib/kernel/qmetatype.h
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class qdatastream
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief QDataStream format versions
  // @see qdatastream.h@qtbase
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  enum version
  {
    QT_1_0 = 1,
    QT_2_0 = 2,
    QT_2_1 = 3,
    QT_3_0 = 4,
    QT_3_1 = 5,
    QT_3_3 = 6,
    QT_4_0 = 7,
    QT_4_1 = QT_4_0,
    QT_4_2 = 8,
    QT_4_3 = 9,
    QT_4_4 = 10,
    QT_4_5 = 11,
    QT_4_6 = 12,
    QT_4_7 = QT_4_6,
    QT_4_8 = QT_4_7,
    QT_4_9 = QT_4_8,
    QT_5_0 = 13,
    QT_5_1 = 14,
    QT_5_2 = 15,
    QT_5_3 = QT_5_2,
    QT_5_4 = 16,
    QT_5_5 = QT_5_4,
    QT_5_6 = 17,
    QT_5_7 = QT_5_6,
    QT_5_8 = QT_5_7,
    QT_5_9 = QT_5_8,
    QT_5_10 = QT_5_9,
    QT_5_11 = QT_5_10,
    QT_5_12 = 18,
    QT_5_13 = 19,
    QT_5_14 = QT_5_13,
    QT_5_15 = QT_5_14,
    QT_6_0 = QT_5_15,
    QT_NEWEST = QT_6_0,
  };

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructor
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  explicit qdatastream (const mobius::core::io::reader&, std::uint32_t version = 19);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  operator bool () const noexcept
  {
    return bool (decoder_);
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get qint8
  // @return Qint8 value
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::int8_t
  get_qint8 ()
  {
    return decoder_.get_int8 ();
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get quint8
  // @return Quint8 value
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint8_t
  get_quint8 ()
  {
    return decoder_.get_uint8 ();
  }
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get qint16
  // @return Qint16 value
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::int16_t
  get_qint16 ()
  {
    return decoder_.get_int16_be ();
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get qint32
  // @return Qint32 value
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::int32_t
  get_qint32 ()
  {
    return decoder_.get_int32_be ();
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get qint64
  // @return Qint64 value
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::int64_t
  get_qint64 ()
  {
    return decoder_.get_int64_be ();
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string get_qstring ();
  mobius::core::datetime::date get_qdate ();
  mobius::core::datetime::time get_qtime ();
  mobius::core::datetime::datetime get_qdatetime ();
  mobius::core::bytearray get_qbytearray ();
  mobius::core::bytearray get_qimage ();
  mobius::core::bytearray get_qpixmap ();
  mobius::core::pod::data get_qvariant ();

private:
  // @brief Internal data decoder object
  mobius::core::decoder::data_decoder decoder_;

  // @brief QDataStream version
  std::uint32_t version_ = 0;
};

} // namespace mobius::core::decoder

#endif


