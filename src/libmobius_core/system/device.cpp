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
#include <libudev.h>
#include <mobius/core/exception.inc>
#include <mobius/core/exception_posix.inc>
#include <mobius/core/io/reader_impl_base.hpp>
#include <mobius/core/system/device.hpp>
#include <stdexcept>

namespace mobius::core::system
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Device reader implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class reader_impl_device : public mobius::core::io::reader_impl_base
{
  public:
    explicit reader_impl_device (const device &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if reader is seekable
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_seekable () const override
    {
        return true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if <b>reader.get_size</b> is available
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_sizeable () const override
    {
        return true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get data size
    // @return data size in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    size_type
    get_size () const override
    {
        return size_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get read position
    // @return read position in bytes from the beginning of data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    offset_type
    tell () const override
    {
        return pos_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get read position
    // @return read position in bytes from the beginning of data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    eof () const override
    {
        return pos_ >= size_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get default block size
    // @return Block size in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    size_type
    get_block_size () const override
    {
        return 65536;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Virtual methods
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::bytearray read (size_type) override;
    void seek (offset_type, whence_type = whence_type::beginning) override;

  private:
    std::shared_ptr<FILE> fp_;
    size_type size_;
    size_type pos_ = 0;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param dev device
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
reader_impl_device::reader_impl_device (const device &dev)
{
    // get device size in bytes
    const std::string sectors_str = dev.get_sysattr ("size");
    const std::string sector_size_str =
        dev.get_sysattr ("queue/logical_block_size");

    if (sectors_str.empty () || sector_size_str.empty ())
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("unable to get device size"));

    size_ = stoull (sectors_str) * stoull (sector_size_str);

    // open device
    const std::string node = dev.get_node ();
    FILE *fp = fopen (node.c_str (), "rb");
    if (!fp)
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

    fp_ = std::shared_ptr<FILE> (fp, fclose);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set read position
// @param offset offset in bytes
// @param w either beginning, current or end
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
reader_impl_device::seek (offset_type offset, whence_type w)
{
    // calculate offset from the beginning of data
    offset_type abs_offset;

    if (w == whence_type::beginning)
        abs_offset = offset;

    else if (w == whence_type::current)
        abs_offset = pos_ + offset;

    else if (w == whence_type::end)
        abs_offset = size_ - 1 + offset;

    else
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid whence_type"));

    // update current pos, if possible
    if (fseek (fp_.get (), abs_offset, 0) == -1)
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

    pos_ = abs_offset;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief read bytes from reader
// @param size size in bytes
// @return bytearray containing data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
reader_impl_device::read (size_type size)
{
    size = std::min (size_ - pos_, size);
    mobius::core::bytearray buffer (size);
    size_t count = fread (buffer.data (), 1, size, fp_.get ());

    if (count == 0 && ferror (fp_.get ()))
    {
        clearerr (fp_.get ());
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
    }

    pos_ += count;
    buffer.resize (count);

    return buffer;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create object
// @param dev udev_device pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
device::device (udev_device *dev)
{
    impl_ = std::shared_ptr<udev_device> (dev, udev_device_unref);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief check if device is initialized
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
device::is_initialized () const
{
    return udev_device_get_is_initialized (impl_.get ()) == 1;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get device type
// @return device type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
device::get_type () const
{
    std::string value;
    const char *p_value = udev_device_get_devtype (impl_.get ());

    if (p_value)
        value = p_value;

    return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get device node path (e.g. /dev/sda)
// @return device node
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
device::get_node () const
{
    std::string value;
    const char *p_value = udev_device_get_devnode (impl_.get ());

    if (p_value)
        value = p_value;

    return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get device subsystem
// @return device subsystem
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
device::get_subsystem () const
{
    std::string value;
    const char *p_value = udev_device_get_subsystem (impl_.get ());

    if (p_value)
        value = p_value;

    return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get device property
// @param name property name
// @return property value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
device::get_property (const std::string &name) const
{
    std::string value;
    const char *p_value =
        udev_device_get_property_value (impl_.get (), name.c_str ());

    if (p_value)
        value = p_value;

    return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get device property list
// @return property list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
device::property_list_type
device::get_property_list () const
{
    property_list_type property_list;

    udev_list_entry *p = udev_device_get_properties_list_entry (impl_.get ());

    while (p)
    {
        property_list[udev_list_entry_get_name (p)] =
            udev_list_entry_get_value (p);
        p = udev_list_entry_get_next (p);
    }

    return property_list;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get device sysattr
// @param name sysattr name
// @return sysattr value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
device::get_sysattr (const std::string &name) const
{
    std::string value;
    const char *p_value =
        udev_device_get_sysattr_value (impl_.get (), name.c_str ());

    if (p_value)
        value = p_value;

    return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get device sysattr list
// @return sysattr list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
device::sysattr_list_type
device::get_sysattr_list () const
{
    sysattr_list_type sysattr_list;

    udev_list_entry *p = udev_device_get_sysattr_list_entry (impl_.get ());

    while (p)
    {
        sysattr_list.push_back (udev_list_entry_get_name (p));
        p = udev_list_entry_get_next (p);
    }

    return sysattr_list;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create new reader for device
// @return reader
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::reader
device::new_reader () const
{
    return mobius::core::io::reader (
        std::make_shared<reader_impl_device> (*this));
}

} // namespace mobius::core::system
