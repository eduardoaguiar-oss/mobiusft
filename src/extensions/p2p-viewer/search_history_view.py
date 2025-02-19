# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023 Eduardo Aguiar
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
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
from gi.repository import Gtk

from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief view: Search History
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SearchHistoryView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()

        self.name = 'Search History'
        icon_path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'search.png')
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.ui.container()
        self.__widget.show()

        # vpaned
        self.__vpaned = Gtk.VPaned()
        self.__vpaned.show()
        self.__widget.set_content(self.__vpaned)

        pos = mobius.framework.get_config('p2p-viewer.search-vpaned-pos')
        if pos:
            self.__vpaned.set_position(pos)

        # tableview
        self.__tableview = self.__mediator.call('ui.new-widget', 'tableview')
        self.__tableview.set_report_id('p2p.search')
        self.__tableview.set_report_name('P2P: Search History')
        self.__tableview.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
        self.__tableview.set_control(self, 'main_tableview')

        column = self.__tableview.add_column('idx', '#', column_type='int')
        column.is_sortable = True

        column = self.__tableview.add_column('timestamp', 'Date/Time (UTC)')
        column.is_sortable = True

        column = self.__tableview.add_column('username', 'User name')
        column.is_sortable = True

        column = self.__tableview.add_column('application', 'Application')
        column.is_sortable = True

        column = self.__tableview.add_column('file_count', '# files', column_type='int')
        column.is_sortable = True

        self.__tableview.add_column('Text')

        column = self.__tableview.add_column('Obj', column_type='object')
        column.is_visible = False

        self.__tableview.show()
        self.__vpaned.pack1(self.__tableview.get_ui_widget(), False, True)

        # metadata view
        self.__metadata_view = self.__mediator.call('ui.new-widget', 'tableview')
        self.__metadata_view.set_report_id('p2p.search')
        self.__metadata_view.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
        self.__metadata_view.show()

        column = self.__metadata_view.add_column('name', 'Attribute')
        column.is_sortable = True

        column = self.__metadata_view.add_column('value', 'Value')
        column.is_sortable = True

        self.__vpaned.pack2(self.__metadata_view.get_ui_widget(), True, True)

        # set initial message
        self.__widget.set_message('Select item(s) to view search history')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, data):
        self.__widget.show_content()
        self.__tableview.clear()

        for idx, search in enumerate(data.searches):
            count = search.count or search.get_file_count()
            self.__tableview.add_row(
                (idx + 1, search.timestamp, search.username, search.app, count, search.text, search))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief save current state
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        mobius.framework.set_config('p2p-viewer.search-vpaned-pos', self.__vpaned.get_position())
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle main tableview selection-changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def main_tableview_on_selection_changed(self, selection_list):
        self.__metadata_view.clear()

        if selection_list:
            row_id, row_data = selection_list[0]
            f = row_data[-1]

            for name, value in f.iter_metadata():
                self.__metadata_view.add_row((name, value))

            self.__metadata_view.set_report_name('P2P: Search "%s"' % f.text)
