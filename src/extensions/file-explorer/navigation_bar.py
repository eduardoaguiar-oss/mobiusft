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
from gi.repository import GdkPixbuf
from gi.repository import Gtk

from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
NBAR_COMBO_ICON, NBAR_COMBO_PATH, NBAR_COMBO_OBJ = range(3)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Navigation bar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class NavigationBar(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__control = control
        self.__mediator = pymobius.mediator.copy()

        # combobox
        datastore = Gtk.ListStore.new([GdkPixbuf.Pixbuf, str, object])

        self.__combobox = Gtk.ComboBox.new_with_model(datastore)
        self.__combobox.connect('changed', self.__on_combobox_selection_changed)
        self.__combobox.show()
        self.__widget = self.__combobox

        renderer = Gtk.CellRendererPixbuf()
        self.__combobox.pack_start(renderer, False)
        self.__combobox.add_attribute(renderer, 'pixbuf', NBAR_COMBO_ICON)

        renderer = Gtk.CellRendererText()
        renderer.set_padding(5, 0)
        self.__combobox.pack_start(renderer, True)
        self.__combobox.add_attribute(renderer, 'text', NBAR_COMBO_PATH)

        # folder icon
        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'folder.svg')
        image = mobius.core.ui.new_icon_by_path(path, mobius.core.ui.icon.size_menu)
        self.__folder_icon = image.get_ui_widget().get_pixbuf()

        self.__selecting_item = False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief show widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def show(self):
        self.__widget.show()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set current folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_folder(self, folder):
        model = self.__combobox.get_model()
        model.clear()

        while folder:
            model.append((self.__folder_icon, folder.path, folder))
            folder = folder.get_parent()

        self.__selecting_item = True
        self.__combobox.set_active(0)
        self.__selecting_item = False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle on_selection_changed event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_combobox_selection_changed(self, combobox, *args):
        combo_iter = combobox.get_active_iter()

        if combo_iter and not self.__selecting_item:
            model = combobox.get_model()
            folder = model.get_value(combo_iter, NBAR_COMBO_OBJ)
            self.__control.on_folder_activated(folder)
