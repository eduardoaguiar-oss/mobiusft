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
#include <mobius/core/os/win/registry/hive_data.hpp>
#include <mobius/core/os/win/registry/hive_decoder.hpp>

namespace mobius::core::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::uint32_t INVALID_OFFSET = 0xffffffff;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class hive_data::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl () = default;
    impl (const mobius::core::io::reader &, offset_type, std::uint32_t,
          std::uint32_t);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief check if object is valid
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const { return offset_ != INVALID_OFFSET; }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief get offset
    // @return offset
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    offset_type
    get_offset () const
    {
        return offset_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief get size
    // @return size
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_size () const
    {
        return size_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief get type
    // @return type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    data_type
    get_type () const
    {
        return type_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief get data
    // @return data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::bytearray
    get_data () const
    {
        _load_data ();
        return data_;
    }

  private:
    // @brief generic reader
    mobius::core::io::reader reader_;

    // @brief offset in bytes
    offset_type offset_ = INVALID_OFFSET;

    // @brief data size in bytes
    std::uint32_t size_;

    // @brief data type
    data_type type_;

    // @brief data buffer
    mutable mobius::core::bytearray data_;

    // @brief data loaded flag
    mutable bool data_loaded_ = false;

    // helper functions
    void _load_data () const;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param offset Offset in bytes
// @param size Data size
// @param type Data type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_data::impl::impl (const mobius::core::io::reader &reader,
                       offset_type offset, std::uint32_t size,
                       std::uint32_t type)
    : reader_ (reader),
      offset_ (offset),
      size_ (size),
      type_ (static_cast<data_type> (type))
{
    // If the MSB of data_size is 1, data is stored directly in the data offset
    // field
    constexpr std::uint32_t HIVE_DATA_IN_OFFSET = 0x80000000;

    if (size_ & HIVE_DATA_IN_OFFSET)
    {
        size_ ^= HIVE_DATA_IN_OFFSET;

        if (size_)
        {
            data_ =
                mobius::core::bytearray {std::uint8_t (offset_ & 0xff),
                                         std::uint8_t ((offset_ >> 8) & 0xff),
                                         std::uint8_t ((offset_ >> 16) & 0xff),
                                         std::uint8_t ((offset_ >> 24) & 0xff)};

            if (size_ < 4)
                data_.resize (size_);
        }

        data_loaded_ = true;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load data on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hive_data::impl::_load_data () const
{
    if (data_loaded_ || offset_ == INVALID_OFFSET)
        return;

    // create decoder
    hive_decoder decoder (reader_);
    data_ = decoder.decode_data (offset_, size_);

    // set data loaded
    data_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_data::hive_data ()
    : impl_ (std::make_shared<impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param offset Offset in bytes
// @param size Data size
// @param type Data type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_data::hive_data (const mobius::core::io::reader &reader,
                      offset_type offset, std::uint32_t size,
                      std::uint32_t type)
    : impl_ (std::make_shared<impl> (reader, offset, size, type))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief check if object is valid
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_data::operator bool () const { return impl_->operator bool (); }

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get offset
// @return offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_data::offset_type
hive_data::get_offset () const
{
    return impl_->get_offset ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get size
// @return size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
hive_data::get_size () const
{
    return impl_->get_size ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get type
// @return type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_data::data_type
hive_data::get_type () const
{
    return impl_->get_type ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get data
// @return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
hive_data::get_data () const
{
    return impl_->get_data ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get data as DWORD
// @return Dword value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
hive_data::get_data_as_dword () const
{
    if (!impl_->operator bool ())
        return {};

    std::uint32_t value = 0;
    auto data = get_data ();

    if (get_type () == data_type::reg_dword_big_endian)
    {
        value = std::uint32_t (data[3]) | (std::uint32_t (data[2]) << 8) |
                (std::uint32_t (data[1]) << 16) |
                (std::uint32_t (data[0]) << 24);
    }

    else if (get_type () == data_type::reg_dword || data.size () == 4)
    {
        value = std::uint32_t (data[0]) | (std::uint32_t (data[1]) << 8) |
                (std::uint32_t (data[2]) << 16) |
                (std::uint32_t (data[3]) << 24);
    }

    return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get data as QWORD
// @return qword value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
hive_data::get_data_as_qword () const
{
    if (!impl_->operator bool ())
        return {};

    std::uint64_t value = 0;
    auto data = get_data ();

    if (get_type () == data_type::reg_qword || data.size () == 8)
    {
        value =
            std::uint64_t (data[0]) | (std::uint64_t (data[1]) << 8) |
            (std::uint64_t (data[2]) << 16) | (std::uint64_t (data[3]) << 24) |
            (std::uint64_t (data[4]) << 32) | (std::uint64_t (data[5]) << 40) |
            (std::uint64_t (data[6]) << 48) | (std::uint64_t (data[7]) << 56);
    }

    return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get data as UTF-8 string
// @param encoding data encoding
// @return string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
hive_data::get_data_as_string (const std::string &encoding) const
{
    if (!impl_->operator bool ())
        return {};

    std::string str;
    auto data = get_data ();
    auto type = get_type ();

    if (data && (type == data_type::reg_sz || type == data_type::reg_expand_sz))
        str = data.to_string (encoding);

    return str;
}

} // namespace mobius::core::os::win::registry
