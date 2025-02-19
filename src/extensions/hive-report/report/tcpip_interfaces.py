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
# @brief TCP/IP interfaces report
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class TCPIPInterfacesReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'tcpip-interfaces'
        self.name = 'TCP/IP interfaces'
        self.group = 'os'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_report_name('%s of computer <%s>' % (self.name, get_computer_name(registry)))

        # fill data
        for key in registry.get_key_by_mask(
                'HKLM\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\*'):
            guid = key.name

            # get name
            connection_key = registry.get_key_by_path(
                f'HKLM\\SYSTEM\\CurrentControlSet\\Control\\Network\\{{4D36E972-E325-11CE-BFC1-08002BE10318}}\\{guid}\\Connection')
            if connection_key:
                name = get_data_as_string(connection_key.get_data_by_name('Name'))
            else:
                name = None

            # check DHCP
            dhcp = get_data_as_dword(key.get_data_by_name('EnableDHCP')) == 1

            if dhcp:
                ip_address = get_data_as_string(key.get_data_by_name('DhcpIPAddress'))
                default_gateway = get_data_as_string(key.get_data_by_name('DhcpDefaultGateway'))
                nameservers = (get_data_as_string(key.get_data_by_name('DhcpNameServer')) or '').replace(',',
                                                                                                         '\n').replace(
                    ' ', '\n')
                dhcp_server = get_data_as_string(key.get_data_by_name('DhcpServer'))

            else:
                ip_address = get_data_as_string(key.get_data_by_name('IPAddress'))
                default_gateway = get_data_as_string(key.get_data_by_name('DefaultGateway'))
                nameservers = (get_data_as_string(key.get_data_by_name('NameServer')) or '').replace(',', '\n').replace(
                    ' ', '\n')
                dhcp_server = ''

            if name:
                self.viewer.add_row((name, dhcp, ip_address, default_gateway, nameservers, dhcp_server))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'tableview')
        self.viewer.add_column('name')
        self.viewer.add_column('DHCP')

        column = self.viewer.add_column('IP Address')
        column.is_sortable = True

        self.viewer.add_column('default gateway')
        self.viewer.add_column('nameservers')
        self.viewer.add_column('DHCP server')

        self.viewer.set_report_id('registry.' + self.id)
        self.viewer.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
