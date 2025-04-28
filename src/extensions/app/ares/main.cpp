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
#include "decoder_impl_arestra.hpp"
#include "decoder_impl_pbthash.hpp"
#include "decoder_impl_phash.hpp"
#include "decoder_impl_shareh.hpp"
#include "decoder_impl_sharel.hpp"
#include "decoder_impl_torrenth.hpp"
#include "evidence_loader_impl.hpp"
#include <mobius/core/resource.hpp>
#include <mobius/core/file_decoder/decoder.hpp>
#include <mobius/framework/evidence_loader.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Extension data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C"
{
const char *EXTENSION_ID = "app-ares";
const char *EXTENSION_NAME = "Ares Galaxy";
const char *EXTENSION_VERSION = "1.3";
const char *EXTENSION_AUTHORS = "Eduardo Aguiar";
const char *EXTENSION_DESCRIPTION = "Ares Galaxy support";
} // extern "C"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Start extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
start ()
{
  mobius::core::add_resource (
     "evidence_loader.builder.app-ares",
     "Ares Galaxy evidence loader",
     mobius::framework::new_evidence_loader_builder_resource <mobius::extension::app::ares::evidence_loader_impl> ()
  );

  mobius::core::add_resource (
     "file_decoder.builder.app-ares-arestra",
     "Ares Galaxy's ___ARESTRA___* file",
     mobius::core::file_decoder::make_decoder_builder_resource <mobius::extension::app::ares::decoder_impl_arestra> ()
  );

  mobius::core::add_resource (
     "file_decoder.builder.app-ares-pbthash",
     "Ares Galaxy's pbthash.dat file",
     mobius::core::file_decoder::make_decoder_builder_resource <mobius::extension::app::ares::decoder_impl_pbthash> ()
  );

  mobius::core::add_resource (
     "file_decoder.builder.app-ares-phash",
     "Ares Galaxy's phash.dat file",
     mobius::core::file_decoder::make_decoder_builder_resource <mobius::extension::app::ares::decoder_impl_phash> ()
  );

  mobius::core::add_resource (
     "file_decoder.builder.app-ares-shareh",
     "Ares Galaxy's ShareH.dat file",
     mobius::core::file_decoder::make_decoder_builder_resource <mobius::extension::app::ares::decoder_impl_shareh> ()
  );

  mobius::core::add_resource (
     "file_decoder.builder.app-ares-sharel",
     "Ares Galaxy's ShareL.dat file",
     mobius::core::file_decoder::make_decoder_builder_resource <mobius::extension::app::ares::decoder_impl_sharel> ()
  );

  mobius::core::add_resource (
     "file_decoder.builder.app-ares-torrenth",
     "Ares Galaxy's TorrentH.dat file",
     mobius::core::file_decoder::make_decoder_builder_resource <mobius::extension::app::ares::decoder_impl_torrenth> ()
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Stop extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
stop ()
{
  mobius::core::remove_resource ("evidence_loader.builder.app-ares");
  mobius::core::remove_resource ("evidence_loader.supported.app-ares");
  mobius::core::remove_resource ("file_decoder.builder.app-ares-arestra");
  mobius::core::remove_resource ("file_decoder.builder.app-ares-pbthash");
  mobius::core::remove_resource ("file_decoder.builder.app-ares-phash");
  mobius::core::remove_resource ("file_decoder.builder.app-ares-shareh");
  mobius::core::remove_resource ("file_decoder.builder.app-ares-sharel");
  mobius::core::remove_resource ("file_decoder.builder.app-ares-torrenth");
}
