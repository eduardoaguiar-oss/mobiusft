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
#include "evidence_processor_impl.hpp"
#include <mobius/core/io/walker.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/vfs/imagefile.hpp>
#include <mobius/framework/model/evidence.hpp>
#include <unordered_set>

namespace
{
// @brief Supported file extensions
std::unordered_set<std::string> supported_extensions;

// @brief Virtual disk types
std::unordered_set<std::string> virtual_disk_image_types;

} // namespace

namespace mobius::extension::disk_image_scanner
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param item Item object
// @param profile Profile object
// @param mediator Mediator object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
evidence_processor_impl::evidence_processor_impl (
    const mobius::framework::model::item &item,
    const mobius::framework::evidence_processor::profile &,
    const mobius::framework::evidence_processor::mediator &mediator
)
    : item_ (item),
      mediator_ (mediator)
{
    for (const auto &type : mobius::core::vfs::get_imagefile_types ())
    {
        if (!type.is_raw)
        {
            std::transform (
                type.file_extensions.begin (),
                type.file_extensions.end (),
                std::inserter (supported_extensions, supported_extensions.end ()),
                [] (const std::string &s) { return mobius::core::string::tolower (s); }
            );

            if (type.is_virtual_disk)
                virtual_disk_image_types.insert (type.id);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::on_folder_entered (const mobius::core::io::folder &folder)
{
    _scan_folder (folder);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Called when processing is complete
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::on_complete ()
{
    _save_disk_images ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for Virtual Disk Images
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::_scan_folder (const mobius::core::io::folder &folder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Scan folder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto w = mobius::core::io::walker (folder);

    for (const auto &[name, f] : w.get_files_with_names ())
    {
        try
        {
            auto pos = name.find_last_of ('.');

            if (pos != std::string::npos)
            {
                std::string ext = name.substr (pos + 1);

                if (supported_extensions.find (ext) != supported_extensions.end ())
                    _process_disk_image_file (f);
            }
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")");
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Process a disk image file
// @param f File object representing the disk image
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::_process_disk_image_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        mobius::core::vfs::imagefile img (f);

        if (!img)
            return;

        disk_image disk_img;

        disk_img.type = img.get_type ();
        disk_img.metadata = img.get_attributes ();
        disk_img.f = f;
        disk_img.is_virtual = virtual_disk_image_types.find (disk_img.type) != virtual_disk_image_types.end ();

        disk_images_.emplace_back (disk_img);
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")");
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save disk images
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::_save_disk_images ()
{
    for (const auto &vdi : disk_images_)
    {
        auto e = item_.new_evidence ("disk-image");

        // Attributes
        e.set_attribute ("type", vdi.type);
        e.set_attribute ("path", vdi.f.get_path ());
        e.set_attribute ("creation_time", vdi.f.get_creation_time ());
        e.set_attribute ("modification_time", vdi.f.get_modification_time ());
        e.set_attribute ("is_virtual", vdi.is_virtual);

        // Metadata
        e.set_attribute ("metadata", vdi.metadata);

        // Sources
        e.add_source (vdi.f);

        // Notify mediator
        mediator_.on_evidence_created (e);
    }
}

} // namespace mobius::extension::disk_image_scanner
