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
import pymobius.registry.installed_programs

from common import *
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief installed programs report
# @author Eduardo Aguiar
# @reference Forensic Analysis of the Windows Registry, by Lih Wern Wong, p.8
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class InstalledProgramReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'installed-programs'
        self.name = 'Installed programs'
        self.group = 'app'
        self.description = 'Programs listed in Control Panel->Add/Remove Programs option'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_master_report_name('%s of computer <%s>' % (self.name, get_computer_name(registry)))

        # fill data
        for program in pymobius.registry.installed_programs.get(registry):
            self.viewer.add_row_from_object(program)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'widetableview')

        column = self.viewer.add_column('username', 'User name')
        column.is_sortable = True

        column = self.viewer.add_column('install_date', 'Installation Date')
        column.is_sortable = True

        column = self.viewer.add_column('display_name', 'Name')
        column.is_sortable = True

        self.viewer.add_column('version', 'Version')

        column = self.viewer.add_column('last_modification_time', 'Last modification Date/Time')
        column.is_visible = False

        column = self.viewer.add_column('install_location', 'Installation location')
        column.is_visible = False

        column = self.viewer.add_column('publisher', 'Publisher')
        column.is_visible = False

        column = self.viewer.add_column('comments', 'Comments')
        column.is_visible = False

        column = self.viewer.add_column('url_info_about', 'About URL')
        column.is_visible = False

        column = self.viewer.add_column('url_update_info', 'Update Info URL')
        column.is_visible = False

        column = self.viewer.add_column('help_link', 'Help Link')
        column.is_visible = False

        self.viewer.set_master_report_id('registry.' + self.id)
        self.viewer.set_master_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
