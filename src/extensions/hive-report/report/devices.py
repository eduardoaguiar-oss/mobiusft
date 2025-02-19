# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023 Eduardo Aguiar
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import pymobius

from common import *
from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief known GUIDs for DeviceClasses
# @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff553412%28v=VS.85%29.aspx
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ENUM_DEVICE_GUID = {

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548596%28v=vs.85%29.aspx
    '059c561e-05ae-4b61-b69d-55b61ee54a7b': 'KSMFT_CATEGORY_MULTIPLEXER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548530%28v=vs.85%29.aspx
    '07dad660-22f1-11d1-a9f4-00c04fbbde8f': 'KSCATEGORY_VBICODEC',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545033%28v=vs.85%29.aspx
    '0850302a-b344-4fda-9be9-90576b8d46f0': 'GUID_BTHPORT_DEVICE_INTERFACE',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548322%28v=vs.85%29.aspx
    '085aff00-62ce-11cf-a5d6-28db04c10000': 'KSCATEGORY_BRIDGE',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545029%28v=vs.85%29.aspx
    '095780c3-48a1-4570-bd95-46707f78c2dc': 'GUID_AVC_CLASS',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548495%28v=vs.85%29.aspx
    '0a4252a0-7e70-11d0-a5d6-28db04c10000': 'KSCATEGORY_SPLITTER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548578%28v=vs.85%29.aspx
    '11064c48-3648-4ed0-932e-05ce8ac811b7': 'KSMFT_CATEGORY_AUDIO_EFFECT',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548607%28v=vs.85%29.aspx
    '12e17c21-532c-4a6e-8a1c-40825a736397': 'KSMFT_CATEGORY_VIDEO_EFFECT',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545950%28v=vs.85%29.aspx
    '152e5811-feb9-4b00-90f4-d32947ae1681': 'GUID_DEVINTERFACE_SIDESHOW',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548374%28v=vs.85%29.aspx
    '19689bf6-c384-48fd-ad51-90e58c79f70b': 'KSCATEGORY_ENCODER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545988%28v=vs.85%29.aspx
    '1ad9e4f0-f88d-4360-bab9-4c2d55e564cd': 'GUID_DEVINTERFACE_VIDEO_OUTPUT_ARRIVAL',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff546042%28v=vs.85%29.aspx
    '1ca05180-a699-450a-9a0c-de4fbe3ddd89': 'GUID_DISPLAY_DEVICE_ARRIVAL',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548351%28v=vs.85%29.aspx
    '1e84c900-7e70-11d0-a5d6-28db04c10000': 'KSCATEGORY_DATACOMPRESSOR',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545866%28v=vs.85%29.aspx
    '2564aa4f-dddb-4495-b497-6ad4a84163d7': 'GUID_DEVINTERFACE_I2C',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548356%28v=vs.85%29.aspx
    '2721ae20-7e70-11d0-a5d6-28db04c10000': 'KSCATEGORY_DATADECOMPRESSOR',

    # @reference http://social.msdn.microsoft.com/forums/en-US/vbgeneral/thread/c3e74fe7-0bd9-40eb-895f-439ddff6df9d
    '28d78fad-5a12-11d1-ae5b-0000f803a8c2': 'GUID_DEVINTERFACE_USBPRINT',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545961%28v=vs.85%29.aspx
    '2accfe60-c130-11d2-b082-00a0c91efb8b': 'GUID_DEVINTERFACE_STORAGEPORT',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545892%28v=vs.85%29.aspx
    '2c7089aa-2e0e-11d1-b114-00c04fc2aae4': 'GUID_DEVINTERFACE_MODEM',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548362%28v=vs.85%29.aspx
    '2eb07ea0-7e70-11d0-a5d6-28db04c10000': 'KSCATEGORY_DATATRANSFORM',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548613%28v=vs.85%29.aspx
    '302ea3fc-aa5f-47f9-9f7a-c2188bb16302': 'KSMFT_CATEGORY_VIDEO_PROCESSOR',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548543%28v=vs.85%29.aspx
    '3503eac4-1f26-11d1-8ab0-00a0c9223196': 'KSCATEGORY_VIRTUAL',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545912%28v=vs.85%29.aspx
    '378de44c-56ef-11d1-bc8c-00a0c91405dd': 'GUID_DEVINTERFACE_MOUSE',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545978%28v=vs.85%29.aspx
    '3abf6f2d-71c4-462a-8a92-1e6861e6af27': 'GUID_DEVINTERFACE_USB_HOST_CONTROLLER',

    # @reference http://www.bustrace.com/products/guids/PlugAndPlay.htm
    '3c0d501a-140b-11d1-b40f-00a0c9223196': 'KSNAME_SERVER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548561%28v=vs.85%29.aspx
    '3e227e76-690d-11d2-8161-0000f8775bf1': 'KSCATEGORY_WDMAUD',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545753%28v=vs.85%29.aspx
    '3fd0f03d-92e0-45fb-b75c-5ed8ffb01021': 'GUID_DEVICE_MEMORY',

    # @reference http://www.bustrace.com/products/guids/PlugAndPlay.htm
    '4747b320-62ce-11cf-a5d6-28db04c10000': 'KSMEDIUMSETID_STANDARD',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545790%28v=vs.85%29.aspx
    '4afa3d51-74a7-11d0-be5e-00a0c9062857': 'GUID_DEVICE_THERMAL_ZONE',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545745%28v=vs.85%29.aspx
    '4afa3d52-74a7-11d0-be5e-00a0c9062857': 'GUID_DEVICE_LID',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545782%28v=vs.85%29.aspx
    '4afa3d53-74a7-11d0-be5e-00a0c9062857': 'GUID_DEVICE_SYS_BUTTON',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545860%28v=vs.85%29.aspx
    '4d1e55b2-f16f-11cf-88cb-001111000030': 'GUID_DEVINTERFACE_HID',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545943%28v=vs.85%29.aspx
    '4d36e978-e325-11ce-bfc1-08002be10318': 'GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548328%28v=vs.85%29.aspx
    '53172480-4791-11d0-a5d6-28db04c10000': 'KSCATEGORY_CLOCK',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545824%28v=vs.85%29.aspx
    '53f56307-b6bf-11d0-94f2-00a0c91efb8b': 'GUID_DEVINTERFACE_DISK',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545813%28v=vs.85%29.aspx
    '53f56308-b6bf-11d0-94f2-00a0c91efb8b': 'GUID_DEVINTERFACE_CDROM',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545941%28v=vs.85%29.aspx
    '53f5630a-b6bf-11d0-94f2-00a0c91efb8b': 'GUID_DEVINTERFACE_PARTITION',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545966%28v=vs.85%29.aspx
    '53f5630b-b6bf-11d0-94f2-00a0c91efb8b': 'GUID_DEVINTERFACE_TAPE',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff546019%28v=vs.85%29.aspx
    '53f5630c-b6bf-11d0-94f2-00a0c91efb8b': 'GUID_DEVINTERFACE_WRITEONCEDISK',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545990%28v=vs.85%29.aspx
    '53f5630d-b6bf-11d0-94f2-00a0c91efb8b': 'GUID_DEVINTERFACE_VOLUME',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545885%28v=vs.85%29.aspx
    '53f56310-b6bf-11d0-94f2-00a0c91efb8b': 'GUID_DEVINTERFACE_MEDIUMCHANGER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545846%28v=vs.85%29.aspx
    '53f56311-b6bf-11d0-94f2-00a0c91efb8b': 'GUID_DEVINTERFACE_FLOPPY',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545803%28v=vs.85%29.aspx
    '53f56312-b6bf-11d0-94f2-00a0c91efb8b': 'GUID_DEVINTERFACE_CDCHARGER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545831%28v=vs.85%29.aspx
    '5b45201d-f2f2-4f3b-85bb-30ff1f953599': 'GUID_DEVINTERFACE_DISPLAY_ADAPTER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545063%28v=vs.85%29.aspx
    '629758ee-986e-4d9e-8e47-de27f8ab054d': 'GUID_DEVICE_APPLICATIONLAUNCH_BUTTON',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff546079%28v=vs.85%29.aspx
    '616ef4d0-23ce-446d-a568-c31eb01913d0': 'GUID_VIRTUAL_AVC_CLASS',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548325%28v=vs.85%29.aspx
    '65e8773d-8f56-11d0-a3b9-00a0c9223196': 'KSCATEGORY_CAPTURE',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548493%28v=vs.85%29.aspx
    '65e8773e-8f56-11d0-a3b9-00a0c9223196': 'KSCATEGORY_RENDER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548451%28v=vs.85%29.aspx
    '67c9cc3c-69c4-11d2-8759-00a0c9223196': 'KSCATEGORY_NETWORK',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548261%28v=vs.85%29.aspx
    '6994ad04-93ef-11d0-a3cc-00a0c9223196': 'KSCATEGORY_AUDIO',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548536%28v=vs.85%29.aspx
    '6994ad05-93ef-11d0-a3cc-00a0c9223196': 'KSCATEGORY_VIDEO',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548509%28v=vs.85%29.aspx
    '6994ad06-93ef-11d0-a3cc-00a0c9223196': 'KSCATEGORY_TEXT',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545996%28v=vs.85%29.aspx
    '6ac27878-a6fa-4155-ba85-f98f491d4f33': 'GUID_DEVINTERFACE_WPD',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff537822%28v=vs.85%29.aspx
    '6bdd1fc1-810f-11d0-bec7-08002be2092f': 'BUS1394_CLASS_GUID',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545874%28v=vs.85%29.aspx
    '6bdd1fc6-810f-11d0-bec7-08002be2092f': 'GUID_DEVINTERFACE_IMAGE',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548304%28v=vs.85%29.aspx
    '71985f48-1ca1-11d3-9cc8-00c04f7971e0': 'KSCATEGORY_BDA_NETWORK_TUNER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548292%28v=vs.85%29.aspx
    '71985f49-1ca1-11d3-9cc8-00c04f7971e0': 'KSCATEGORY_BDA_NETWORK_EPG',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548284%28v=vs.85%29.aspx
    '71985f4a-1ca1-11d3-9cc8-00c04f7971e0': 'KSCATEGORY_BDA_IP_SINK',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548297%28v=vs.85%29.aspx
    '71985f4b-1ca1-11d3-9cc8-00c04f7971e0': 'KSCATEGORY_BDA_NETWORK_PROVIDER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545729%28v=vs.85%29.aspx
    '72631e54-78a4-11d0-bcf7-00aa00b7b32a': 'GUID_DEVICE_BATTERY',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548382%28v=vs.85%29.aspx
    '74f3aea8-9768-11d1-8e07-00a0c95ec22e': 'KSCATEGORY_ESCALANTE_PLATFORM_DRIVER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548390%28v=vs.85%29.aspx
    '760fed5e-9357-11d0-a3cc-00a0c9223196': 'KSCATEGORY_FILESYSTEM',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548434%28v=vs.85%29.aspx
    '7a5de1d3-01a1-452c-b481-4fa2b96271e8': 'KSCATEGORY_MULTIPLEXER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545024%28v=vs.85%29.aspx
    '7ebefbc0-3200-11d2-b4c2-00a0c9697d07': 'GUID_61883_CLASS',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545938%28v=vs.85%29.aspx
    '811fc6a5-f728-11d0-a537-0000f8753ed1': 'GUID_DEVINTERFACE_PARCLASS',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548415%28v=vs.85%29.aspx
    '830a44f2-a32d-476b-be97-42845673b35a': 'KSCATEGORY_MICROPHONE_ARRAY_PROCESSOR',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545821%28v=vs.85%29.aspx
    '86e0d1e0-8089-11d0-9ce4-08003e301f73': 'GUID_DEVINTERFACE_COMPORT',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545881%28v=vs.85%29.aspx
    '884b96c3-56ef-11d1-bc8c-00a0c91405dd': 'GUID_DEVINTERFACE_KEYBOARD',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548601%28v=vs.85%29.aspx
    '90175d57-b7ea-4901-aeb3-933a8747756f': 'KSMFT_CATEGORY_OTHER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548584%28v=vs.85%29.aspx
    '91c64bd0-f91e-4d8c-9276-db248279d975': 'KSMFT_CATEGORY_AUDIO_ENCODER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548475%28v=vs.85%29.aspx
    '97ebaaca-95bd-11d0-a3ea-00a0c9223196': 'KSCATEGORY_PROXY',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548479%28v=vs.85%29.aspx
    '97ebaacb-95bd-11d0-a3ea-00a0c9223196': 'KSCATEGORY_QUALITY',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545934%28v=vs.85%29.aspx
    '97f76ef0-f883-11d0-af1f-0000f800845c': 'GUID_DEVINTERFACE_PARALLEL',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545776%28v=vs.85%29.aspx
    '97fadb10-4e33-40ae-359c-8bef029dbdd0': 'GUID_DEVICE_PROCESSOR',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548274%28v=vs.85%29.aspx
    '9baf9572-340c-11d3-abdc-00a0c90ab16f': 'KSCATEGORY_AUDIO_GFX',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548277%28v=vs.85%29.aspx
    '9ea331fa-b91b-45f8-9285-bd2bc77afcde': 'KSCATEGORY_AUDIO_SPLITTER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548572%28v=vs.85%29.aspx
    '9ea73fb4-ef7a-4559-8d5d-719d8f0426c7': 'KSMFT_CATEGORY_AUDIO_DECODER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548315%28v=vs.85%29.aspx
    'a2e3074f-6c3d-11d3-b653-00c04f79498e': 'KSCATEGORY_BDA_TRANSPORT_INFORMATION',

    # @reference http://www.bustrace.com/products/guids/PlugAndPlay.htm
    'ad498944-762f-11d0-8dcb-00c04fc3358c': 'GUID_NDIS_LAN_CLASS',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545972%28v=vs.85%29.aspx
    'a5dcbf10-6530-11d2-901f-00c04fb951ed': 'GUID_DEVINTERFACE_USB_DEVICE',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548526%28v=vs.85%29.aspx
    'a799a800-a46d-11d0-a18c-00a02401dcd4': 'KSCATEGORY_TVTUNER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548347%28v=vs.85%29.aspx
    'a799a801-a46d-11d0-a18c-00a02401dcd4': 'KSCATEGORY_CROSSBAR',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548520%28v=vs.85%29.aspx
    'a799a802-a46d-11d0-a18c-00a02401dcd4': 'KSCATEGORY_TVAUDIO',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548550%28v=vs.85%29.aspx
    'a799a803-a46d-11d0-a18c-00a02401dcd4': 'KSCATEGORY_VPMUX',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548503%28v=vs.85%29.aspx
    'a7c7a5b1-5af3-11d1-9ced-00a024bf0407': 'KSCATEGORY_SYSAUDIO',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548594%28v=vs.85%29.aspx
    'a8700a7a-939b-44c5-99d7-76226b23b3f1': 'KSMFT_CATEGORY_DEMULTIPLEXER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548428%28v=vs.85%29.aspx
    'ad809c00-7b88-11d0-a5d6-28db04c10000': 'KSCATEGORY_MIXER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff546010%28v=vs.85%29.aspx
    'ba0c718f-4ded-49b7-bdd3-fabe28661211': 'GUID_DEVINTERFACE_WPD_PRIVATE',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545931%28v=vs.85%29.aspx
    'bf4672de-6b4e-4be4-a325-68a91ea49c09': 'GUID_DEVINTERFACE_OPM',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548253%28v=vs.85%29.aspx
    'bf963d80-c559-11d0-8a2b-00a0c9255ac1': 'KSCATEGORY_ACOUSTIC_ECHO_CANCEL',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545922%28v=vs.85%29.aspx
    'cac88484-7515-4c03-82e6-71a87abac361': 'GUID_DEVINTERFACE_NET',

    # @reference http://kztechs.googlecode.com/svn/trunk/trunk/include/ntddk/inc/ddk/w2k/gameport.h
    'cae56030-684a-11d0-d6f6-00a0c90f57da': 'GUID_GAMEENUM_BUS_ENUMERATOR',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545766%28v=vs.85%29.aspx
    'cd48a365-fa94-4ce2-a232-a1b764e5d8b4': 'GUID_DEVICE_MESSAGE_INDICATOR',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548340%28v=vs.85%29.aspx
    'cf1dda2c-9743-11d0-a3ee-00a0c9223196': 'KSCATEGORY_COMMUNICATIONSTRANSFORM',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548394%28v=vs.85%29.aspx
    'cf1dda2d-9743-11d0-a3ee-00a0c9223196': 'KSCATEGORY_INTERFACETRANSFORM',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548406%28v=vs.85%29.aspx
    'cf1dda2e-9743-11d0-a3ee-00a0c9223196': 'KSCATEGORY_MEDIUMTRANSFORM',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548602%28v=vs.85%29.aspx
    'd6c02d4b-6833-45b4-971a-05a4b04bab91': 'KSMFT_CATEGORY_VIDEO_DECODER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548472%28v=vs.85%29.aspx
    'd6c5066e-72c1-11d2-9755-0000f8004788': 'KSCATEGORY_PREFERRED_WAVEOUT_DEVICE',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548467%28v=vs.85%29.aspx
    'd6c50671-72c1-11d2-9755-0000f8004788': 'KSCATEGORY_PREFERRED_WAVEIN_DEVICE',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548460%28v=vs.85%29.aspx
    'd6c50674-72c1-11d2-9755-0000f8004788': 'KSCATEGORY_PREFERRED_MIDIOUT_DEVICE',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548516%28v=vs.85%29.aspx
    'dda54a40-1e4c-11d1-a050-405705c10000': 'KSCATEGORY_TOPOLOGY',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548501%28v=vs.85%29.aspx
    'dff220f3-f70f-11d0-b917-00a0c9223196': 'KSCATEGORY_SYNTHESIZER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545901%28v=vs.85%29.aspx
    'e6f07b5f-ee97-4a90-b076-33f57bf4eaa7': 'GUID_DEVINTERFACE_MONITOR',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548485%28v=vs.85%29.aspx
    'eb115ffc-10c8-4964-831d-6dcb02e6f23f': 'KSCATEGORY_REALTIME',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545982%28v=vs.85%29.aspx
    'f18a0e88-c30c-11d0-8815-00a0c906bed8': 'GUID_DEVINTERFACE_USB_HUB',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548611%28v=vs.85%29.aspx
    'f79eac7d-e545-4387-bdee-d647d7bde42a': 'KSMFT_CATEGORY_VIDEO_ENCODER',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548268%28v=vs.85%29.aspx
    'fbf6f530-07b9-11d2-a71e-0000f8004788': 'KSCATEGORY_AUDIO_DEVICE',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548308%28v=vs.85%29.aspx
    'fd0a5af4-b41d-11d2-9c95-00c04f7971e0': 'KSCATEGORY_BDA_RECEIVER_COMPONENT',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff545799%28v=vs.85%29.aspx
    'fde5bba4-b3f9-46fb-bdaa-0728ce3100b4': 'GUID_DEVINTERFACE_BRIGHTNESS',

    # @reference http://msdn.microsoft.com/en-us/library/windows/hardware/ff548368%28v=vs.85%29.aspx
    'ffbb6e3f-ccfe-4d84-90d9-421418b03a8e': 'KSCATEGORY_DRM_DESCRAMBLE'
}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Devices report
# @author Eduardo Aguiar
# @reference Windows Registry Forensics, by Harlan Carvey, p.110-111
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class DevicesReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, name, classlist=None):
        self.id = name.lower().replace(' ', '-')
        self.name = name.capitalize()
        self.classlist = set(classlist or [])
        self.group = 'computer'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_report_name('%s of computer <%s>' % (self.name, get_computer_name(registry)))

        # walk through device classes
        for guid_key in registry.get_key_by_mask('HKLM\\SYSTEM\\CurrentControlSet\\Control\\DeviceClasses\\*'):
            class_guid = guid_key.name[1:-1].lower()
            class_identifier = ENUM_DEVICE_GUID.get(class_guid)

            if not self.classlist or class_identifier in self.classlist:
                for key in guid_key.subkeys:
                    device_instance = get_data_as_string(key.get_data_by_name('DeviceInstance'))

                    if device_instance:
                        device_type, device_class_id, device_id = device_instance.split('\\')
                    else:
                        device_type, device_class_id, device_id = '', '', ''

                    # serial
                    if len(device_id) > 2 and device_id[1] != '&' and device_id[-2] == '&':
                        device_serial = device_id.split('&')[0]
                    else:
                        device_serial = ''

                    # name and manufacturer
                    enum_key = registry.get_key_by_path(
                        'HKLM\\SYSTEM\\CurrentControlSet\\Enum\\%s\\%s\\%s' % (device_type, device_class_id, device_id))

                    if enum_key:
                        device_name = get_data_as_string(
                            enum_key.get_data_by_name('FriendlyName')) or get_data_as_string(
                            enum_key.get_data_by_name('DeviceDesc'))
                        device_mfg = get_data_as_string(enum_key.get_data_by_name('Mfg')).split(';')[-1]
                        classname = get_data_as_string(enum_key.get_data_by_name('Class'))

                        if device_mfg.startswith('('):
                            device_mfg = ''
                    else:
                        device_name = ''
                        device_mfg = ''
                        classname = ''

                    # add device
                    self.viewer.add_row((key.last_modification_time, classname, device_type, device_name, device_serial,
                                         device_mfg, class_guid, class_identifier, device_id, device_class_id))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'tableview')

        column = self.viewer.add_column('last datetime')
        column.is_sortable = True

        column = self.viewer.add_column('classname')
        column.is_sortable = True

        column = self.viewer.add_column('type')
        column.is_sortable = True

        column = self.viewer.add_column('name')
        column.is_sortable = True

        column = self.viewer.add_column('serial', column_name='Serial number')
        column.is_sortable = True

        column = self.viewer.add_column('manufacturer')
        column.is_sortable = True

        column = self.viewer.add_column('class GUID')
        column.is_sortable = True

        column = self.viewer.add_column('class identifier')
        column.is_sortable = True

        self.viewer.add_column('device_id', column_name='Device ID')
        self.viewer.add_column('device_class_id', column_name='Device class ID')

        self.viewer.set_report_id('registry.' + self.id)
        self.viewer.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
