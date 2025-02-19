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
# @brief Encrypted volumes report
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class EncryptedVolumesReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'encrypted-volumes'
        self.name = 'Encrypted volumes'
        self.group = 'user'
        self.description = 'List of encrypted volumes used'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_report_name(f'{self.name} of computer <{get_computer_name(registry)}>')

        # user keys
        for username, key in iter_hkey_users(registry):
            self.retrieve_folder_lock6(username, key)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'tableview')

        column = self.viewer.add_column('username')
        column.is_sortable = True

        column = self.viewer.add_column('application')
        column.is_sortable = True

        column = self.viewer.add_column('volume path')
        column.is_sortable = True

        column = self.viewer.add_column('count', column_type='int')
        column.is_sortable = True

        column = self.viewer.add_column('first access')
        column.is_sortable = True

        column = self.viewer.add_column('last access')
        column.is_sortable = True

        self.viewer.set_report_id('registry.' + self.id)
        self.viewer.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Folder locker 6
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def retrieve_folder_lock6(self, username, key):
        flk_key = key.get_key_by_path('SOFTWARE\\FolderLock6\\Logs')
        if not flk_key:
            return

        # search for volumes
        volumes = set()

        for value in flk_key.values:
            if value.name.endswith('CNT'):
                name = value.name[:-3]
                volumes.add(name)

        # retrieve volume information
        RE_FLK_DATETIME = re.compile('([0-9]+)-([0-9]+)-([0-9]+) ([0-9]+):([0-9]+):([0-9]+)')

        for vol in volumes:
            count = int(get_data_as_string(flk_key.get_data_by_name(vol + 'CNT')))

            match = RE_FLK_DATETIME.match(get_data_as_string(flk_key.get_data_by_name(vol + 'TIME0')))
            if match:
                d = [int(x) for x in match.groups()]
                first_time = '%04d-%02d-%02d %02d:%02d:%02d' % (d[2], d[0], d[1], d[3], d[4], d[5])
            else:
                first_time = ''

            match = RE_FLK_DATETIME.match(get_data_as_string(flk_key.get_data_by_name(vol + ('TIME%d' % (count - 1)))))
            if match:
                d = [int(x) for x in match.groups()]
                last_time = '%04d-%02d-%02d %02d:%02d:%02d' % (d[2], d[0], d[1], d[3], d[4], d[5])
            else:
                last_time = ''

            self.viewer.add_row((username, 'Folder Locker 6', vol, count, first_time, last_time))
