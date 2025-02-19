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

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# References
#  [01] A Windows Registry Quick-Reference for the Everyday Examiner
#       Derrick J. Farmer dfarmer03@gmail.com
#       http://eptuners.com/forensics/A%20Windows%20Registry%20Quick%20Reference.pdf, visited in 2011-12-30
#
#  [02] Windows Registry Forensics
#       Harlan Carvey
#       ISBN: 978-1-59749-580-6
#
#  [03] Forensic Analysis of the Windows Registry
#       Peter Davies
#       http://www.pkdavies.co.uk/downloads/registry_examination.pdf, visited in 2011-12-30
#
# Forensic Analysis of the Windows Registry, by Lih Wern Wong
#   http://www.forensicfocus.com/downloads/forensic-analysis-windows-registry.pdf
#
# @todo WOW64 symlinks http://msdn.microsoft.com/en-us/library/aa384253%28v=vs.85%29.aspx#wow64_symbolic_links
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import mobius

from report.autorun import AutoRunReport
from report.cached_credentials import UserCachedCredentialReport
from report.computer_info import ComputerInfoReport
from report.devices import DevicesReport
from report.devices import ENUM_DEVICE_GUID
from report.email_accounts import EmailAccountsReport
from report.encrypted_volumes import EncryptedVolumesReport
from report.enum_devices import EnumDevicesReport
from report.gigatribe import GigatribeAccountsReport
from report.gigatribe import GigatribeDownloadFoldersReport
from report.gigatribe import GigatribeRequestedPasswordsReport
from report.installed_programs import InstalledProgramReport
from report.internet_explorer import InternetExplorerAutoCompleteReport
from report.internet_explorer import InternetExplorerDownloadFolderReport
from report.internet_explorer import InternetExplorerTypedURLReport
from report.lsa_secrets import LSASecretsReport
from report.mounted_devices import MountedDevicesReport
from report.mru import OpenSaveMRUReport
from report.mru import RunMRUReport
from report.my_network_places import MyNetworkPlacesReport
from report.my_recent_documents import MyRecentDocumentsByFileTypeReport
from report.my_recent_documents import MyRecentDocumentsReport
from report.network_list import NetworkListReport
from report.os_folders import OSFoldersReport
from report.os_info import OSInfoReport
from report.os_shared_folders import SharedFoldersReport
from report.printer_ports import PrinterPortsReport
from report.product_keys import ProductKeyReport
from report.search_assistant import SearchAssistantReport
from report.services import ServicesReport
from report.tcpip_interfaces import TCPIPInterfacesReport
from report.user_accounts import UserAccountReport
from report.user_assist import UserAssistReport
from report.user_profiles import UserProfileReport
from report.word_wheel_query import WordWheelQueryReport

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief List of report object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
REPORT_OBJECTS = [

    # Operating System
    OSInfoReport(),
    AutoRunReport(),
    OSFoldersReport(),
    MountedDevicesReport(),
    LSASecretsReport(),
    ServicesReport(),
    SharedFoldersReport(),
    TCPIPInterfacesReport(),

    # Computer
    ComputerInfoReport(),
    NetworkListReport(),
    EnumDevicesReport(),
    DevicesReport('all devices'),
    DevicesReport('disk drives', ['GUID_DEVINTERFACE_DISK']),
    DevicesReport('display adapters', ['GUID_DEVINTERFACE_DISPLAY_ADAPTER']),
    DevicesReport('HID devices', ['GUID_DEVINTERFACE_MOUSE', 'GUID_DEVINTERFACE_KEYBOARD', 'GUID_DEVINTERFACE_HID',
                                  'GUID_DEVICE_SYS_BUTTON']),
    DevicesReport('imaging devices', ['GUID_DEVINTERFACE_IMAGE']),
    DevicesReport('optical drives', ['GUID_DEVINTERFACE_CDROM']),
    DevicesReport('network devices',
                  ['GUID_DEVINTERFACE_NET', 'GUID_BTHPORT_DEVICE_INTERFACE', 'GUID_DEVINTERFACE_MODEM']),
    DevicesReport('processors', ['GUID_DEVICE_PROCESSOR']),
    DevicesReport('stream devices',
                  [identifier for identifier in ENUM_DEVICE_GUID.values() if identifier.startswith('KS')]),

    # User
    UserAccountReport(),
    UserCachedCredentialReport(),
    EncryptedVolumesReport(),
    RunMRUReport(),
    OpenSaveMRUReport(),
    MyNetworkPlacesReport(),
    MyRecentDocumentsReport(),
    MyRecentDocumentsByFileTypeReport(),
    PrinterPortsReport(),
    UserProfileReport(),
    SearchAssistantReport(),
    UserAssistReport(),
    WordWheelQueryReport(),

    # Application
    EmailAccountsReport(),
    InstalledProgramReport(),
    InternetExplorerAutoCompleteReport(),
    InternetExplorerDownloadFolderReport(),
    InternetExplorerTypedURLReport(),
    ProductKeyReport(),

    # Gigatribe
    GigatribeAccountsReport(),
    GigatribeDownloadFoldersReport(),
    GigatribeRequestedPasswordsReport(),
]


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    for r in REPORT_OBJECTS:
        mobius.core.add_resource(f'registry.report.{r.id}', f'Hive Report: {r.name}', r)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    for r in REPORT_OBJECTS:
        mobius.core.remove_resource('registry.report.' + r.id)
