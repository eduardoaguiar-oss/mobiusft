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
# @brief Enumerated devices report
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class EnumDevicesReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'enumerated-devices'
        self.name = 'Enumerated devices'
        self.group = 'computer'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_report_name('%s of computer <%s>' % (self.name, get_computer_name(registry)))

        # walk through enum keys
        for device_type_key in registry.get_key_by_mask('HKLM\\SYSTEM\\CurrentControlSet\\Enum\\*'):
            device_type = device_type_key.name

            for device_class_key in device_type_key.subkeys:
                device_class_id = device_class_key.name

                for key in device_class_key.subkeys:
                    device_id = key.name

                    # serial
                    if len(device_id) > 2 and device_id[1] != '&' and device_id[-2] == '&':
                        device_serial = device_id.split('&')[0]
                    else:
                        device_serial = ''

                    # name and manufacturer
                    device_name = (get_data_as_string(key.get_data_by_name('FriendlyName')) or
                                   get_data_as_string(key.get_data_by_name('DeviceDesc')).split(';')[-1])
                    device_mfg = get_data_as_string(key.get_data_by_name('Mfg')).split(';')[-1]
                    classname = get_data_as_string(key.get_data_by_name('Class'))

                    if device_mfg.startswith('('):
                        device_mfg = ''

                    # add device
                    self.viewer.add_row((key.last_modification_time, classname, device_type, device_name, device_serial,
                                         device_mfg, device_id, device_class_id))

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

        column = self.viewer.add_column('serial', column_name='serial number')
        column.is_sortable = True

        column = self.viewer.add_column('manufacturer')
        column.is_sortable = True

        self.viewer.add_column('device_id', column_name='device ID')
        self.viewer.add_column('device_class_id', column_name='device class ID')

        self.viewer.set_report_id('registry.' + self.id)
        self.viewer.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
