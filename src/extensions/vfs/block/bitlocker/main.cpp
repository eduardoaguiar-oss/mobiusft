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
#include <mobius/bytearray.h>
#include <mobius/core/log.hpp>
#include <mobius/core/resource.hpp>
#include <mobius/decoder/data_decoder.h>
#include <mobius/string_functions.h>
#include <mobius/core/vfs/block.hpp>
#include <map>
#include <vector>

/*
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * References
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * https://en.wikipedia.org/wiki/BitLocker
 * Implementing BitLocker Drive Encryption for Forensic Analysis - Jesse D. Kornblum
 * https://eudl.eu/pdf/10.1007/978-3-319-14289-0_2
 * https://github.com/libyal/libbde/blob/main/documentation/BitLocker%20Drive%20Encryption%20(BDE)%20format.asciidoc
 * https://learn.microsoft.com/en-us/windows/security/operating-system-security/data-protection/bitlocker/

 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Bitlocker Volume Encryption
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * 1. Bitlocker volume data is encrypted using the FVEK (Full Volume Encryption Key) key
 * 2. FVEK key is encrypted using VMK (Volume Master Key) keys
 * 3. VMK key can be stored encrypted multiple types for a single volume, using different PROTECTORS
 * 4. Protectors can be:
 *   . Passphrase
 *   . Smart Card
 *   . TPM (Trust Platform Module) chip
 *   . TPM + PIN (number)
 *   . TPM + Startup key
 *   . TPM + PIN + Startup key
 *   . Startup Key
 *   . Recovery Key
 *   . External key (.bek recovery file)
 */

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::size_t BDE_HEADER_SIZE = 512;
static const mobius::bytearray BDE_SIGNATURE = "-FVE-FS-";
static const std::string BDE_GUID = "4967D63B-2E29-4AD8-8399-F6A339E3D001";
static const std::string BDE_USED_DISK_SPACE_ONLY_GUID = "92A84D3B-DD80-4D0E-9E4E-B1E3284EAED8";

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Protection descriptions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::map <std::uint16_t, const std::string> PROTECTION_DESCRIPTION =
{
  {0x0000, "Clear key"},
  {0x0100, "TPM"},
  {0x0200, "Startup Key"},
  {0x0500, "TPM and PIN"},
  {0x0800, "Recovery Key"},
  {0x2000, "User Password"},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encryption descriptions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::map <std::uint16_t, const std::string> ENCRYPTION_DESCRIPTION =
{
  {0x8000, "AES-128 (CBC with Diffuser)"},
  {0x8001, "AES-256 (CBC with Diffuser)"},
  {0x8002, "AES-128 (CBC)"},
  {0x8003, "AES-256 (CBC)"},
  {0x8004, "AES-128 (XTS)"},
  {0x8005, "AES-128 (XTS)"},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Function declarations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector <mobius::core::pod::map> _decode_fve_metadata_entries (const mobius::bytearray&);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get protection description
// @param flag Protection flag
// @return Description
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
_get_protection_description (std::uint16_t flag)
{
  auto iterator = PROTECTION_DESCRIPTION.find (flag);

  if (iterator != PROTECTION_DESCRIPTION.end ())
    return iterator->second;

  return "Unknown protection type (0x" + mobius::string::to_hex (flag, 4) + ')';
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get encryption description
// @param type Encryption type
// @return Description
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
_get_encryption_description (std::uint16_t type)
{
  auto iterator = ENCRYPTION_DESCRIPTION.find (type);

  if (iterator != ENCRYPTION_DESCRIPTION.end ())
    return iterator->second;


  return "Unknown encryption type (0x" + mobius::string::to_hex (type, 4) + ')';
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode FVE AES-CCM encrypted key (section 5.6)
// @param data Data
// @return Map containing attributes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::pod::map
_decode_fve_metadata_0005 (const mobius::bytearray& data)
{
  mobius::core::pod::map m;

  auto decoder = mobius::decoder::data_decoder (data);

  m.set ("datetime", decoder.get_nt_datetime ());
  m.set ("nonce_counter", decoder.get_uint32_le ());
  m.set ("encrypted_data", decoder.get_bytearray_by_size (data.size () - 12));

  return m;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Volume Master Key (VMK) (section 5.9)
// @param data Data
// @return Map containing attributes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::pod::map
_decode_fve_metadata_0008 (const mobius::bytearray& data)
{
  mobius::core::pod::map m;

  auto decoder = mobius::decoder::data_decoder (data);

  auto key_guid = decoder.get_guid ();
  auto mtime = decoder.get_nt_datetime ();
  auto unknown_1 = decoder.get_uint16_le ();    // maybe status (active, deleted)?
  auto protection_flag = decoder.get_uint16_le ();
  auto payload = decoder.get_bytearray_by_size (data.size () - 28);
  auto metadata = _decode_fve_metadata_entries (payload);
  auto protection_description = _get_protection_description (protection_flag);

  m.set ("key_guid", key_guid);
  m.set ("mtime", mtime);
  m.set ("unknown_1", unknown_1);
  m.set ("protection_flag", protection_flag);
  m.set ("protection_description", protection_description);
  //m.set ("metadata", metadata);

  mobius::core::log log (__FILE__, __FUNCTION__);
  log.debug (__LINE__, "[VMK]: 0x0008");
  log.debug (__LINE__, "Key GUID: " + key_guid);
  log.debug (__LINE__, "Mtime: " + to_string (mtime));
  log.debug (__LINE__, "Unknown_1: " + std::to_string (unknown_1));
  log.debug (__LINE__, "Protection flag: 0x" + mobius::string::to_hex (protection_flag, 4));
  log.debug (__LINE__, "Protection description: " + protection_description);
  log.debug (__LINE__, "Data:\n" + payload.dump ());

  return m;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode metadata entry
// @param decoder Data decoder object
// @return Entry as map
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::pod::map
_decode_fve_metadata_entry (mobius::decoder::data_decoder& decoder)
{
  mobius::core::pod::map entry;

  auto size = decoder.get_uint16_le ();
  if (!size)
    return entry;

  auto entry_type = decoder.get_uint16_le ();
  auto value_type = decoder.get_uint16_le ();
  auto version = decoder.get_uint16_le ();
  std::uint16_t data_size = (size > 8) ? size - 8 : 0;
  auto data = decoder.get_bytearray_by_size (data_size);

  mobius::core::log log (__FILE__, __FUNCTION__);
  log.debug (__LINE__, "FVE metadata entry");
  log.debug (__LINE__, "Size: " + std::to_string (size));
  log.debug (__LINE__, "Data size: " + std::to_string (size > 8 ? size - 8 : 0));
  log.debug (__LINE__, "Entry type: 0x" + mobius::string::to_hex (entry_type, 4));
  log.debug (__LINE__, "Value type: 0x" + mobius::string::to_hex (value_type, 4));
  log.debug (__LINE__, "Version: " + std::to_string (version));
  log.debug (__LINE__, "Data:\n" + data.dump ());

  entry.set ("size", size);
  entry.set ("entry_type", entry_type);
  entry.set ("value_type", value_type);
  entry.set ("version", version);

  switch (value_type)
    {
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Unicode string (UTF-16LE)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    case 0x0002:
      {
        auto text = data.to_string ("utf-16le");
        entry.set ("text", text);
      }
      break;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // FVE AES-CCM encrypted key
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    case 0x0005:
      {
        auto m = _decode_fve_metadata_0005 (data);
        entry.update (m);
      }
      break;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // FVE Volume Master key (VMK)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    case 0x0008:
      {
        auto m = _decode_fve_metadata_0008 (data);
        entry.update (m);
      }
      break;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Unknown value type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    default:
      log.development (__LINE__, "New value type found: 0x" + mobius::string::to_hex (value_type, 4));
      break;
    };

  return entry;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode metadata entries
// @param data Data
// @return Entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector <mobius::core::pod::map>
_decode_fve_metadata_entries (const mobius::bytearray& data)
{
  std::vector <mobius::core::pod::map> entries;
  auto decoder = mobius::decoder::data_decoder (data);

  while (decoder)
    {
      auto entry = _decode_fve_metadata_entry (decoder);
      entries.push_back (entry);
    }

  return entries;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode FVE metadata block
// @param bde_block BDE block object
// @param offset Offset from the beginning of BDE block
// @see Bitlocker Drive Encryption (BDE) format - section 5
//
// FVE metadata block consists of:
//   1. block header
//   2. metadata header
//   3. metadata entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static bool
_decode_fve_metadata (mobius::core::vfs::block& bde_block, std::uint64_t offset)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create data decoder
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto reader = bde_block.new_reader ();
  reader.seek (offset);
  log.debug (__LINE__, "FVE Block:\n" + reader.read (64).dump ());

  auto decoder = mobius::decoder::data_decoder (reader);
  decoder.seek (offset);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Check block header signature
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto signature = decoder.get_bytearray_by_size (8);

  if (signature != BDE_SIGNATURE)
    return false;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode FVE block header
  // Bitlocker Drive Encryption (BDE) format - section 5.1.1
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  log.debug (__LINE__, "FVE block header");
  log.debug (__LINE__, "Size: " + std::to_string (decoder.get_uint16_le ()));
  auto version = decoder.get_uint16_le ();
  log.debug (__LINE__, "Version: " + std::to_string (version));

  if (version == 2)
    {
      decoder.skip (4);
      bde_block.set_attribute ("encrypted_size", decoder.get_uint64_le ());
      decoder.skip (4);
      bde_block.set_attribute ("volume_header_sectors", decoder.get_uint32_le ());
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode FVE metadata header
  // Bitlocker Drive Encryption (BDE) format - section 5.2
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  decoder.seek (offset + 64);
  auto size = decoder.get_uint32_le ();
  auto header_version = decoder.get_uint32_le ();
  auto data_size = (size > 48) ? size - 48 : 0;
  decoder.skip (8);
  auto volume_guid = decoder.get_guid ();
  auto nonce_counter = decoder.get_uint32_le ();
  auto encryption_method = decoder.get_uint32_le ();
  auto creation_time = decoder.get_nt_datetime ();
  auto data = decoder.get_bytearray_by_size (data_size);
  auto encryption_description = _get_encryption_description (encryption_method);

  bde_block.set_attribute ("volume_guid", volume_guid);
  bde_block.set_attribute ("nonce_counter", nonce_counter);
  bde_block.set_attribute ("encryption_method", "0x" + mobius::string::to_hex (encryption_method, 8));
  bde_block.set_attribute ("encryption_description", encryption_description);
  bde_block.set_attribute ("creation_time", to_string (creation_time));

  log.debug (__LINE__, "FVE metadata header");
  log.debug (__LINE__, "Size: " + std::to_string (size));
  log.debug (__LINE__, "Version: " + std::to_string (header_version));
  log.debug (__LINE__, "Volume GUID: " + volume_guid);
  log.debug (__LINE__, "Nonce counter: " + std::to_string (nonce_counter));
  log.debug (__LINE__, "Encryption method: " + encryption_description + " (0x" + mobius::string::to_hex (encryption_method, 8));
  log.debug (__LINE__, "Creation time: " + to_string (creation_time));
  log.debug (__LINE__, "Data size: " + std::to_string (data_size));
  log.debug (__LINE__, "Data:\n" + data.dump ());

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode FVE metadata entries
  // Bitlocker Drive Encryption (BDE) format - section 5.3
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& entry : _decode_fve_metadata_entries (data))
    {
      std::uint16_t entry_type = entry.get <std::int64_t> ("entry_type");

      switch (entry_type)
        {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Volume Master Key (VMK)
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        case 0x0002:
          {
            auto v = bde_block.get_attribute <std::vector <mobius::core::pod::data>> ("vmk");
            v.push_back (entry);
            bde_block.set_attribute ("vmk", v);
          }
          break;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Full Volume Encryption Key (FVEK)
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        case 0x0003:
          bde_block.set_attribute ("fvek", entry);
          break;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Volume label
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        case 0x0007:
          {
            auto label = entry.get <std::string> ("text");
            bde_block.set_attribute ("label", label);

            auto v = mobius::string::split (label);
            if (v.size () > 0) bde_block.set_attribute ("computer_name", v[0]);
            if (v.size () > 1) bde_block.set_attribute ("logical_drive", v[1]);
            if (v.size () > 2) bde_block.set_attribute ("creation_date", v[2]);
          }
          break;

        default:
          log.development (__LINE__, "New entry type found: 0x" + mobius::string::to_hex (entry_type, 4));
          break;
        };
    }

  return true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode BPB (BIOS parameter block) fields block
// @param decoder Data decoder object
// @param bde_block BDE block object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_decode_bpb_fields (
  mobius::decoder::data_decoder& decoder,
  mobius::core::vfs::block& bde_block
)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode signature
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  decoder.skip (3);  // jump start instruction
  auto signature = decoder.get_bytearray_by_size (8);
  bde_block.set_attribute ("signature", signature.to_string ("ASCII"));

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode DOS version 2.0 BIOS parameter block (BPB)
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bde_block.set_attribute ("sector_size", decoder.get_uint16_le ());
  bde_block.set_attribute ("sectors_per_cluster", decoder.get_uint8 ());
  bde_block.set_attribute ("reserved_sectors", decoder.get_uint16_le ());
  decoder.skip (5);
  bde_block.set_attribute ("media_descriptor", decoder.get_uint8 ());
  bde_block.set_attribute ("sectors_per_fat", decoder.get_uint16_le ());

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode DOS version 3.4 BIOS parameter block (BPB)
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bde_block.set_attribute ("sectors_per_track", decoder.get_uint16_le ());
  bde_block.set_attribute ("heads", decoder.get_uint16_le ());
  bde_block.set_attribute ("hidden_sectors", decoder.get_uint32_le ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode FAT-32 info block
// @param decoder Data decoder object
// @param bde_block BDE block object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_decode_fat32_info (
  mobius::decoder::data_decoder& decoder,
  mobius::core::vfs::block& bde_block
)
{
  bde_block.set_attribute ("sector_per_fat", decoder.get_uint32_le ());
  bde_block.set_attribute ("fat32_flags", decoder.get_uint16_le ());
  bde_block.set_attribute ("fat32_version", decoder.get_uint16_le ());
  bde_block.set_attribute ("root_dir_cluster", decoder.get_uint32_le ());
  bde_block.set_attribute ("fs_info_sector", decoder.get_uint16_le ());
  bde_block.set_attribute ("backup_boot_sector", decoder.get_uint16_le ());
  decoder.skip (12);    // reserved
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Extended BPB fields block
// @param decoder Data decoder object
// @param bde_block BDE block object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_decode_extended_bpb_fields (
  mobius::decoder::data_decoder& decoder,
  mobius::core::vfs::block& bde_block
)
{
  bde_block.set_attribute ("logical_drive_number", "0x" + mobius::string::to_hex (decoder.get_uint8 (), 2));
  bde_block.set_attribute ("is_dirty", decoder.get_uint8 () == 0x01);
  auto extended_signature = decoder.get_uint8 ();

  if (extended_signature)
    {
      bde_block.set_attribute ("volume_id", "0x" + mobius::string::to_hex (decoder.get_uint32_le (), 8));
      bde_block.set_attribute ("volume_label", mobius::string::rstrip (decoder.get_string_by_size (11)));
      bde_block.set_attribute ("filesystem_type",  mobius::string::rstrip (decoder.get_string_by_size (8)));
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode FVE metadata offset block
// @param decoder Data decoder object
// @param bde_block BDE block object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_decode_fve_metadata_offset_block (
  mobius::decoder::data_decoder& decoder,
  mobius::core::vfs::block& bde_block
)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decoder offsets
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint64_t fve_block_offset[3];
  fve_block_offset[0] = decoder.get_uint64_le ();
  fve_block_offset[1] = decoder.get_uint64_le ();
  fve_block_offset[2] = decoder.get_uint64_le ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Debug info
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (int i = 0; i < 3; i++)
    {
      log.debug (__LINE__,
                   "FVE block " +
                   std::to_string (i + 1) +
                   " offset: 0x" +
                   mobius::string::to_hex (fve_block_offset[i], 8)
                );
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Try to decoder one FVE metadata block
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bool flag_fve_metadata = false;
  int i = 0;

  while (i < 3 && !flag_fve_metadata)
    {
      try
        {
          if (_decode_fve_metadata (bde_block, fve_block_offset[i]))
            flag_fve_metadata = true;
        }
      catch (const std::exception& e)
        {
          log.warning (__LINE__, e.what ());
        }

      ++i;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Bitlocker Win7 and later implementation data
// @param block Block object
// @return BDE block object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::vfs::block
_decode_win7 (const mobius::core::vfs::block& block)
{
  mobius::core::log log (__FILE__, __FUNCTION__);
  log.info (__LINE__, "Bitlocker Win7-11 found");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create bde_block
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto bde_block = mobius::core::vfs::new_slice_block (block, "bitlocker");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create decoder
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto reader = bde_block.new_reader ();
  auto decoder = mobius::decoder::data_decoder (reader);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  _decode_bpb_fields (decoder, bde_block);
  _decode_fat32_info (decoder, bde_block);
  _decode_extended_bpb_fields (decoder, bde_block);

  decoder.seek (160);
  bde_block.set_attribute ("type_guid", decoder.get_guid ());

  _decode_fve_metadata_offset_block (decoder, bde_block);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Return BDE block
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bde_block.set_handled (true);
  return bde_block;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Bitlocker To Go implementation data
// @param block Block object
// @return BDE block object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::vfs::block
_decode_to_go (const mobius::core::vfs::block& block)
{
  mobius::core::log log (__FILE__, __FUNCTION__);
  log.info (__LINE__, "Bitlocker To Go found");
  log.development (__LINE__, "Bitlocker To Go support is experimental");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create BDE block
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto bde_block = mobius::core::vfs::new_slice_block (block, "bitlocker");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create decoder
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto reader = block.new_reader ();
  auto decoder = mobius::decoder::data_decoder (reader);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Bitlocker To Go support is still experimental. So log out header data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto data = reader.read (BDE_HEADER_SIZE);
  log.development (__LINE__, "Header:\n" + data.dump ());

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  decoder.seek (0);
  _decode_bpb_fields (decoder, bde_block);
  _decode_fat32_info (decoder, bde_block);
  _decode_extended_bpb_fields (decoder, bde_block);

  decoder.seek (424);
  bde_block.set_attribute ("type_guid", decoder.get_guid ());

  _decode_fve_metadata_offset_block (decoder, bde_block);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Return BDE block
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bde_block.set_handled (true);
  return bde_block;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Try to decoder an unknown Bitlocker implementation
// @param block Block object
// @return BDE block object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::vfs::block
_decode_unknown_bitlocker (const mobius::core::vfs::block& block)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create BDE block
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto bde_block = mobius::core::vfs::new_slice_block (block, "bitlocker");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create decoder
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto reader = bde_block.new_reader ();
  auto decoder = mobius::decoder::data_decoder (reader);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Log out header data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto data = reader.read (BDE_HEADER_SIZE);

  mobius::core::log log (__FILE__, __FUNCTION__);
  log.development (__LINE__, "Unknown bitlocker implementation");
  log.development (__LINE__, "Header:\n" + data.dump ());
  decoder.seek (0);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  _decode_bpb_fields (decoder, bde_block);
  _decode_fat32_info (decoder, bde_block);
  _decode_extended_bpb_fields (decoder, bde_block);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Return BDE block
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bde_block.set_handled (true);
  return bde_block;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Try to decode block
// @author Eduardo Aguiar
// @param block Block object
// @param new_blocks Vector for newly created blocks
// @param pending_blocks Pending blocks
// @return <b>true</b> if block was decoded, <b>false</b> otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static bool
decoder (
  const mobius::core::vfs::block& block,
  std::vector <mobius::core::vfs::block>& new_blocks,
  std::vector <mobius::core::vfs::block>&
)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get signature and GUIDs
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto reader = block.new_reader ();
  auto decoder = mobius::decoder::data_decoder (reader);

  decoder.seek (3);
  auto signature = decoder.get_bytearray_by_size (8);

  decoder.seek (160);
  auto guid_win7 = decoder.get_guid ();

  decoder.seek (424);
  auto guid_to_go = decoder.get_guid ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Check if block is an instance of Bitlocker volume
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::core::vfs::block bde_block;

  if (signature == BDE_SIGNATURE)       // Vista, Win7 up to Win11
    {
      if (guid_win7 == BDE_GUID || guid_win7 == BDE_USED_DISK_SPACE_ONLY_GUID)
        bde_block = _decode_win7 (block);

      else
        bde_block = _decode_unknown_bitlocker (block);
    }

  else if (guid_to_go == BDE_GUID)      // Bitlocker To Go
    bde_block = _decode_to_go (block);

  else                                  // No bitlocker volume found
    return false;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Set BDE block description
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string description = "BitLocker Volume";

  if (bde_block.has_attribute ("volume_guid"))
    description += " - GUID: " + static_cast <std::string> (bde_block.get_attribute ("volume_guid"));

  bde_block.set_attribute ("description", description);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Add BDE block to new blocks
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  new_blocks.push_back (bde_block);
  return true;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Extension data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C"
{
const char *EXTENSION_ID = "vfs.block.bitlocker";
const char *EXTENSION_NAME = "VFS Block: Bitlocker";
const char *EXTENSION_VERSION = "1.0";
const char *EXTENSION_AUTHORS = "Eduardo Aguiar";
const char *EXTENSION_DESCRIPTION = "Bitlocker VFS block support";
} // extern "C"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Start extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
start ()
{
  mobius::core::add_resource (
     "vfs.block.decoder.bitlocker",
     "Bitlocker VFS block decoder",
     static_cast <mobius::core::vfs::block_decoder_resource_type> (decoder)
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Stop extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
stop ()
{
  mobius::core::remove_resource ("vfs.block.decoder.bitlocker");
}


