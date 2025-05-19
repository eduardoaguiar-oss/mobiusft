# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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

from common import count_evidences
from common import get_icon_path
from metadata import *
from model import MODEL


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @class ListView
# @brief Implements a List View widget for displaying evidences.
# 
# This class is responsible for initializing the List View widget, managing its
# columns and rows, and providing methods to manipulate and update the data 
# displayed in the widget.
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ListView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Constructor: Initialize the ListView widget.
    # @param control An object used to handle events and actions for the ListView.
    # 
    # This method sets up the ListView widget, initializes the columns and rows,
    # and loads icons for visualization.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        mediator = pymobius.mediator.copy()
        self.__category_icons = {}
        self.__control = control
        self.name = 'Evidence List View'

        path = mediator.call('extension.get-resource-path', EXTENSION_ID, 'list-view.png')
        self.icon_data = open(path, 'rb').read()

        # build widget
        self.__listview = mediator.call('ui.new-widget', 'tableview')
        self.__listview.set_control(self, 'listview')
        self.__listview.set_report_id('evidences')
        self.__listview.set_report_name('Evidences')
        self.__listview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__listview.show()
        self.__listview.set_drag_and_drop(False)
        self.__widget = self.__listview

        column = self.__listview.add_column('icon', '', column_type='icon')

        column = self.__listview.add_column('id', 'ID')
        column.is_visible = False

        column = self.__listview.add_column('name', 'Name')
        column.is_markup = True

        column = self.__listview.add_column('count', 'Count', column_type='int')
        column = self.__listview.add_column('description', 'Description')

        # Add evidence rows to listview
        for info in MODEL:
            evidence_type = info.get('id')
            name = info.get('name')
            description = info.get('description') or name

            path = get_icon_path(evidence_type)
            image = mobius.core.ui.new_icon_by_path(path, mobius.core.ui.icon.size_dnd)
            icon = image.get_ui_widget().get_pixbuf()

            self.__listview.set_icon(evidence_type, icon)
            self.__listview.add_row((evidence_type, evidence_type, name, 0, description))

        # Add total row to listview
        path = mediator.call('extension.get-resource-path', EXTENSION_ID, 'total.png')
        image = mobius.core.ui.new_icon_by_path(path, mobius.core.ui.icon.size_dnd)
        icon = image.get_ui_widget().get_pixbuf()

        self.__listview.set_icon('total', icon)
        self.__listview.add_row(('total', None, '<b>TOTAL</b>', 0, None, None))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve the UI widget for the ListView.
    # @return Returns the UI widget associated with this ListView instance.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Updates the ListView with new data.
    # @param itemlist A list of items representing evidence data.
    # 
    # This method updates the 'Count' column values for each evidence type and
    # calculates the total count for all evidence.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
        totals = count_evidences(itemlist)
        total = 0

        for row in self.__listview:
            evidence_type = row[1]

            count = totals.get(evidence_type, 0)
            total += count

            if evidence_type:
                row[3] = count

            else:
                row[3] = total

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Selects a specific row in the ListView by its view ID.
    # @param view_id The unique identifier for the row to select.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def select_view(self, view_id):
        for idx, row in enumerate(self.__listview):
            if row[1] == view_id:
                self.__listview.select_row(idx)
                return

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handles the event where a row in the ListView is activated.
    # @param row_id The identifier of the activated row.
    # @param row_data Data associated with the activated row.
    # 
    # This method triggers the control to handle the activated view based on
    # the selected evidence type or view ID.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def listview_on_row_activated(self, row_id, row_data):
        view_id = row_data[1]

        if view_id:
            self.__control.on_view_activated(view_id)
