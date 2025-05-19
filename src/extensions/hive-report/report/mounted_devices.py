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
import mobius
import pymobius

from common import *
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief mounted devices report
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class MountedDevicesReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'mounted-devices'
        self.name = 'Last mounted devices'
        self.group = 'os'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_report_name('%s of computer <%s>' % (self.name, get_computer_name(registry)))

        # get mounted devices key
        mounted_devices_key = registry.get_key_by_path('HKLM\\SYSTEM\\MountedDevices')
        if not mounted_devices_key:
            return

        # get DosDevices
        devices = []

        for value in mounted_devices_key.values:
            if value.name.startswith('\\DosDevices\\'):
                letter = value.name[12:]
                name = None
                serial = None
                classname = None
                data = value.data

                if not data or len(data.data) == 0:  # ignore
                    pass

                elif len(data.data) == 12:  # fixed disk
                    disk_id, offset = struct.unpack('<IQ', data.data)
                    name = 'fixed disk (ID:%04x, offset:%08x)' % (disk_id, offset)
                    classname = 'diskdrive'

                elif data.data.startswith(b'DMIO:ID:'):  # dynamic disk
                    guid = data.data[8:]
                    name = 'dynamic disk (GUID:%s)' % mobius.core.encoder.hexstring(guid)
                    classname = 'diskdrive'

                else:  # removable disk
                    text = get_data_as_string(data)
                    device_path, guid = text[4:].replace('#', '\\').rsplit('\\', 1)
                    device_key = registry.get_key_by_path('HKLM\\SYSTEM\\CurrentControlSet\\Enum\\' + device_path)

                    if device_key:
                        name = get_data_as_string(device_key.get_data_by_name('friendlyname'))
                        classname = get_data_as_string(device_key.get_data_by_name('class'))
                        serial_data = device_path.split('\\')[-1]

                        if serial_data.count('&') == 1:
                            serial = serial_data.split('&')[0]

                    elif '#' in text:
                        info = text.split('#', 1)[1].split('&')

                        if len(info) > 3 and '#' in info[3]:
                            name = info[2][5:].replace('_', ' ')
                            classname = info[0]
                            serial = info[3].split('#')[1]

                devices.append((letter, name, serial, classname))

        # list devices
        for letter, name, serial, classname in sorted(devices):
            self.viewer.add_row((letter, name, serial, classname))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'tableview')

        column = self.viewer.add_column('drive_letter', column_name='drive letter')
        column.is_sortable = True

        column = self.viewer.add_column('name')

        column = self.viewer.add_column('serial_number', column_name='serial number')

        column = self.viewer.add_column('class')
        column.is_sortable = True

        self.viewer.set_report_id('registry.' + self.id)
        self.viewer.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
