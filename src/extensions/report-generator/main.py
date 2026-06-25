# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008-2026 Eduardo Aguiar
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
import os.path

import mobius
import pymobius
from gi.repository import GLib
from gi.repository import Gtk

from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Report Generator view
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ReportGeneratorView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__iped_path = None

        self.name = f'{EXTENSION_NAME} v{EXTENSION_VERSION}'
        icon_path = self.__mediator.call('extension.get-icon-path', EXTENSION_ID)
        self.icon_data = open(icon_path, 'rb').read()

        # vpaned
        self.__vpaned = Gtk.VPaned()
        self.__vpaned.show()
        self.__widget = self.__vpaned

        pos = mobius.framework.get_config('vfs-viewer.disks-vpaned-position')

        if pos:
            self.__vpaned.set_position(pos)

        # hbox
        self.__hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        self.__hbox.set_spacing(5)
        self.__hbox.set_visible(True)
        self.__vpaned.pack1(self.__hbox.get_ui_widget(), True, False)

        # report list
        self.__report_listview = self.__mediator.call('ui.new-widget', 'tableview')
        self.__report_listview.set_report_id('report.list')
        self.__report_listview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__report_listview.set_multiple_selection(True)
        self.__report_listview.show()

        column = self.__report_listview.add_column('id', 'ID')
        column.is_sortable = True
        
        self.__report_listview.add_column('name', 'Name')
        column.is_sortable = True
        
        self.__report_listview.add_column('object', column_type='object')
        self.__report_listview.set_sort_column_id(1)

        self.__hbox.add_child(self.__report_listview.get_ui_widget(), mobius.core.ui.box.fill_with_widget)

        # side buttons
        vbuttonbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbuttonbox.set_visible(True)
        self.__hbox.add_child(vbuttonbox, mobius.core.ui.box.fill_none)

        button = mobius.core.ui.button()
        button.set_icon_by_name('list-add')
        button.set_text("Add image file")
        button.set_visible(True)
        #button.set_callback('clicked', self.__on_add_imagefile)
        vbuttonbox.add_child(button, mobius.core.ui.box.fill_none)

        button = mobius.core.ui.button()
        button.set_icon_by_name('list-add')
        button.set_text("Add device")
        button.set_visible(True)
        #button.set_callback('clicked', self.__on_add_device)
        vbuttonbox.add_child(button, mobius.core.ui.box.fill_none)

        self.__remove_button = mobius.core.ui.button()
        self.__remove_button.set_icon_by_name('list-remove')
        self.__remove_button.set_text("_Remove")
        self.__remove_button.set_sensitive(False)
        self.__remove_button.set_visible(True)
        #self.__remove_button.set_callback('clicked', self.__on_remove_disk)
        vbuttonbox.add_child(self.__remove_button, mobius.core.ui.box.fill_none)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set status text
    # @param text Text
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_status(self, text):
        if text:
            t = str(datetime.datetime.now())[:19]
            text = t + ' ' + text

        GLib.idle_add(self.__status_label.set_text, text or '')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
        print(itemlist)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set IPED path
    # @param path Full path
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_iped_path(self, path):
        self.__iped_path = path

        self.__processing_view.set_iped_path(path)
        self.__report_view.set_iped_path(path)
        self.__preferences_view.set_iped_path(path)

        if not path:
            self.__view_selector.set_current_view('preferences')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_destroy view
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        self.__mediator.clear()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    mobius.core.add_resource('view.report-generator', 'Report Generator view', ReportGeneratorView)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('view.report-generator')
