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
import datetime

import pymobius

from common import *
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Network list
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class NetworkListReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'network-list'
        self.name = 'WiFi connection points'
        self.group = 'computer'
        self.description = 'list of WiFi connection points'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_report_name(f'{self.name} of computer <{get_computer_name(registry)}>')

        # retrieve data
        access_points = []

        nl_key = registry.get_key_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkList')
        if nl_key:
            keys = []
            managed_key = nl_key.get_key_by_path('Signatures\\Managed')
            if managed_key:
                keys += list(managed_key.subkeys)

            unmanaged_key = nl_key.get_key_by_path('Signatures\\UnManaged')
            if unmanaged_key:
                keys += list(unmanaged_key.subkeys)

            for key in keys:
                default_gateway_mac = key.get_data_by_name('DefaultGatewayMac')

                if default_gateway_mac:
                    default_gateway_mac = mobius.encoder.hexstring(default_gateway_mac.data[:6], ':')

                profile_guid = get_data_as_string(key.get_data_by_name('ProfileGuid'))
                profile_key = nl_key.get_key_by_path('Profiles\\%s' % profile_guid)

                if profile_key:
                    profile_name = get_data_as_string(profile_key.get_data_by_name('ProfileName'))
                    description = get_data_as_string(profile_key.get_data_by_name('Description'))
                    date_created = self.get_nl_profile_datetime(profile_key.get_data_by_name('DateCreated'))
                    date_last_connected = self.get_nl_profile_datetime(
                        profile_key.get_data_by_name('DateLastConnected'))

                    access_points.append((date_created, profile_name, description, default_gateway_mac))
                    if date_created != date_last_connected:
                        access_points.append((date_last_connected, profile_name, description, default_gateway_mac))

        # fill data
        for row in sorted(access_points):
            self.viewer.add_row(row)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Return datetime for a given profile date
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_nl_profile_datetime(self, data):
        value = None

        if data:
            year, month, weekday, day, hh, mm, ss, ms = struct.unpack('<HHHHHHHH', data.data)
            value = datetime.datetime(year, month, day, hh, mm, ss, ms * 1000)

        return value

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'tableview')

        column = self.viewer.add_column('date/time')
        column.is_sortable = True

        column = self.viewer.add_column('network name')
        column.is_sortable = True

        column = self.viewer.add_column('network description')

        column = self.viewer.add_column('gateway MAC address')
        column.is_sortable = True

        self.viewer.set_report_id('registry.' + self.id)
        self.viewer.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
