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
#include "CShareaza.h"
#include <unordered_map>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief List of protocols
//! \see StdAfx.h
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::unordered_map <int, std::string> PROTOCOLS =
{
  {-1, "Any"},
  {0, "Null"},
  {1, "Gnutella v1"},
  {2, "Gnutella v2"},
  {3, "EDonkey"},
  {4, "HTTP"},
  {5, "FTP"},
  {6, "BitTorrent"},
  {7, "KAD"},
  {8, "DC"},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief List of vendors
//! \see Vendors.xml
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::unordered_map <std::string, std::string> VENDORS =
{
  {"RAZA", "Shareaza"},
  {"RAZB", "ShareazaBeta"},
  {"RZCB", "ShareazaPlus"},
  {"ACQL", "Acqlite"},
  {"ACQX", "Acquisition"},
  {"AGIO", "Adagio"},
  {"AGNT", "Agentella"},
  {"ARES", "Ares"},
  {"ATOM", "AtomWire"},
  {"BEAR", "BearShare"},
  {"CULT", "Cultiv8r"},
  {"DC++", "DC++"},
  {"ED2K", "eDonkey2000"},
  {"ENVY", "Envy"},
  {"FOXY", "Foxy"},
  {"FUST", "Fusteeno"},
  {"GDNA", "GnucDNA"},
  {"GIFT", "giFT-Gnutella"},
  {"GNEW", "GNewtellium"},
  {"GNOT", "Gnotella"},
  {"GNTD", "Gtk-Gnutella"},
  {"GNTG", "Gnutelligentsia"},
  {"GNUC", "Gnucleus"},
  {"GNUT", "Gnut"},
  {"GNZL", "Gnoozle"},
  {"GTKG", "Gtk-Gnutella"},
  {"HSLG", "Hagelslag"},
  {"HYDR", "Hydranode"},
  {"JOET", "Joetella"},
  {"LIME", "LimeWire"},
  {"MACT", "Mactella"},
  {"MESH", "iMesh"},
  {"MLDK", "MLdonkey"},
  {"MMMM", "Morpheus"},
  {"MNAP", "MyNapster"},
  {"MOOD", "MoodAmp"},
  {"MRPH", "Morpheus"},
  {"MUTE", "Mutella"},
  {"MXIE", "Mxie"},
  {"NAPS", "NapShare"},
  {"OCFG", "OpenCola"},
  {"OPRA", "Opera"},
  {"PEER", "PeerProject"},
  {"PHEX", "Phex"},
  {"QTEL", "Qtella"},
  {"SHLN", "Sharelin"},
  {"SNOW", "FrostWire"},
  {"SNUT", "SwapNut"},
  {"SWAP", "Swapper.NET"},
  {"SWFT", "SwiftPeer"},
  {"TFLS", "TrustyFiles"},
  {"TOAD", "ToadNode"},
  {"WSHR", "WireShare"},
  {"XOLO", "Xolox"},
  {"XTLA", "XTella"},
  {"ZIGA", "Ziga"},
};

} // namespace

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get protocol name
//! \param id Protocol ID
//! \return Protocol name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_protocol_name (int id)
{
  std::string name;

  auto iter = PROTOCOLS.find (id);

  if (iter != PROTOCOLS.end ())
    name = iter->second;

  return name;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get vendor name
//! \param code Vendor code
//! \return Vendor name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_vendor_name (const std::string& code)
{
  std::string name;

  auto iter = VENDORS.find (code);

  if (iter != VENDORS.end ())
    name = iter->second;

  return name;
}

} // namespace mobius::extension::app::shareaza
