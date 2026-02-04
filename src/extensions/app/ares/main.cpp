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
#include "decoder_impl_arestra.hpp"
#include "decoder_impl_pbthash.hpp"
#include "decoder_impl_phash.hpp"
#include "decoder_impl_shareh.hpp"
#include "decoder_impl_sharel.hpp"
#include "decoder_impl_torrenth.hpp"
#include "vfs_processor_impl.hpp"
#include <mobius/core/file_decoder/decoder.hpp>
#include <mobius/core/resource.hpp>
#include <mobius/framework/ant/vfs_processor.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Extension data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C"
{
    const char *EXTENSION_ID = "app-ares";
    const char *EXTENSION_NAME = "Ares Galaxy";
    const char *EXTENSION_VERSION = "1.6";
    const char *EXTENSION_AUTHORS = "Eduardo Aguiar";
    const char *EXTENSION_DESCRIPTION = "Ares Galaxy support";
} // extern "C"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Start extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
start ()
{
    // Register the vfs-processor implementation
    mobius::framework::ant::register_vfs_processor_implementation<
        mobius::extension::app::ares::vfs_processor_impl> (
        EXTENSION_ID,
        EXTENSION_NAME
    );

    mobius::core::add_resource (
        "file_decoder.builder.app-ares-arestra",
        "Ares Galaxy's ___ARESTRA___* file",
        mobius::core::file_decoder::make_decoder_builder_resource<
            mobius::extension::app::ares::decoder_impl_arestra> ());

    mobius::core::add_resource (
        "file_decoder.builder.app-ares-pbthash",
        "Ares Galaxy's pbthash.dat file",
        mobius::core::file_decoder::make_decoder_builder_resource<
            mobius::extension::app::ares::decoder_impl_pbthash> ());

    mobius::core::add_resource (
        "file_decoder.builder.app-ares-phash", "Ares Galaxy's phash.dat file",
        mobius::core::file_decoder::make_decoder_builder_resource<
            mobius::extension::app::ares::decoder_impl_phash> ());

    mobius::core::add_resource (
        "file_decoder.builder.app-ares-shareh", "Ares Galaxy's ShareH.dat file",
        mobius::core::file_decoder::make_decoder_builder_resource<
            mobius::extension::app::ares::decoder_impl_shareh> ());

    mobius::core::add_resource (
        "file_decoder.builder.app-ares-sharel", "Ares Galaxy's ShareL.dat file",
        mobius::core::file_decoder::make_decoder_builder_resource<
            mobius::extension::app::ares::decoder_impl_sharel> ());

    mobius::core::add_resource (
        "file_decoder.builder.app-ares-torrenth",
        "Ares Galaxy's TorrentH.dat file",
        mobius::core::file_decoder::make_decoder_builder_resource<
            mobius::extension::app::ares::decoder_impl_torrenth> ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Stop extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
stop ()
{
    mobius::framework::ant::unregister_vfs_processor_implementation (
        EXTENSION_ID
    );

    mobius::core::remove_resource ("file_decoder.builder.app-ares-arestra");
    mobius::core::remove_resource ("file_decoder.builder.app-ares-pbthash");
    mobius::core::remove_resource ("file_decoder.builder.app-ares-phash");
    mobius::core::remove_resource ("file_decoder.builder.app-ares-shareh");
    mobius::core::remove_resource ("file_decoder.builder.app-ares-sharel");
    mobius::core::remove_resource ("file_decoder.builder.app-ares-torrenth");
}
