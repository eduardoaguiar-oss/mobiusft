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
# @brief User Profiles report
# @author Eduardo Aguiar
# @reference Windows Registry Forensics, by Harlan Carvey, p.137-138
# @reference http://technet.microsoft.com/en-us/library/cc775560%28WS.10%29.aspx
# @reference Microsoft Windows Internals 4th ed, by Russinovich and Solomon, p.188
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class UserProfileReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'user-profiles'
        self.name = 'Profiles'
        self.group = 'user'
        self.description = 'List of user profiles resident on the system'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_report_name('User profiles of computer <%s>' % get_computer_name(registry))

        # get %systemroot% and %systemdrive%
        env = {}

        systemroot = get_data_as_string(
            registry.get_data_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SystemRoot'))
        if systemroot:
            env['systemroot'] = systemroot
            env['systemdrive'] = systemroot.split('\\')[0]

        # fill data
        for key in registry.get_key_by_mask('HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\*'):
            sid = key.name
            flags = get_data_as_dword(key.get_data_by_name('Flags'))
            lowtime = get_data_as_dword(key.get_data_by_name('ProfileLoadTimeLow'), 0)
            hightime = get_data_as_dword(key.get_data_by_name('ProfileLoadTimeHigh'), 0)

            if lowtime and hightime:
                loadtime = get_nt_datetime(hightime << 32 | lowtime)
            else:
                loadtime = None

            imagepath = expand_sz(get_data_as_string(key.get_data_by_name('ProfileImagePath')), env)

            self.viewer.add_row((key.last_modification_time, sid, flags, loadtime, imagepath))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'tableview')

        column = self.viewer.add_column('mtime', 'last modification time')
        column.is_sortable = True

        column = self.viewer.add_column('sid', 'SID')
        column.is_sortable = True

        self.viewer.add_column('flags', column_type='int')

        column = self.viewer.add_column('loadtime', 'last load time')
        column.is_sortable = True

        self.viewer.add_column('imagepath', 'image path')
        self.viewer.set_report_id('registry.' + self.id)
        self.viewer.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
