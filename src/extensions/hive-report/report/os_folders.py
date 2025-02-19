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
# @brief O.S. folders report
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class OSFoldersReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'os-folders'
        self.name = 'Folders'
        self.group = 'os'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()
        environment = {}

        # set report name
        self.viewer.set_report_name('O.S. folders of computer <%s>' % get_computer_name(registry))

        # bootdir
        bootdir = get_data_as_string(
            registry.get_data_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\BootDir'))
        self.viewer.add_row(('BootDir', bootdir))

        # systemdrive, systemroot
        systemroot = get_data_as_string(
            registry.get_data_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SystemRoot'))
        systemdrive = systemroot.split('\\')[0]

        self.viewer.add_row(('SystemDrive', systemdrive))
        self.viewer.add_row(('SystemRoot', systemroot))

        environment['systemdrive'] = systemdrive
        environment['systemroot'] = systemroot

        # windir
        windir = get_data_as_string(
            registry.get_data_by_path('HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment\\windir'))
        self.viewer.add_row(('WinDir', expand_sz(windir, environment)))
        environment['windir'] = windir

        # temp and tmp
        tempdir = get_data_as_string(
            registry.get_data_by_path('HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment\\temp'))
        self.viewer.add_row(('Temp', expand_sz(tempdir, environment)))

        tmpdir = get_data_as_string(
            registry.get_data_by_path('HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment\\tmp'))
        self.viewer.add_row(('Tmp', expand_sz(tmpdir, environment)))

        # programdir
        programfilesdir = get_data_as_string(
            registry.get_data_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ProgramFilesDir'))
        self.viewer.add_row(('ProgramFiles', programfilesdir))

        # commonfilesdir
        commonfilesdir = get_data_as_string(
            registry.get_data_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CommonFilesDir'))
        self.viewer.add_row(('CommonFiles', commonfilesdir))

        # profilesdir
        profilesdir = get_data_as_string(registry.get_data_by_path(
            'HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\ProfilesDirectory'))
        self.viewer.add_row(('ProfilesDir', expand_sz(profilesdir, environment)))

        # mediapath
        mediapath = get_data_as_string(
            registry.get_data_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\MediaPath'))

        if not mediapath:
            mediapathunexpanded = get_data_as_string(
                registry.get_data_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\MediaPathUnexpanded'))
            mediapath = expand_sz(mediapathunexpanded, environment)

        self.viewer.add_row(('MediaPath', mediapath))

        # shell folders
        key = registry.get_key_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders')
        if key:
            for value in key.values:
                path = get_data_as_string(value.data)
                self.viewer.add_row((value.name, path))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'tableview')

        column = self.viewer.add_column('name')
        column.is_sortable = True

        column = self.viewer.add_column('path')
        column.is_sortable = True

        self.viewer.set_report_id('registry.' + self.id)
        self.viewer.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
