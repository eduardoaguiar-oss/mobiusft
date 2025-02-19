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
from navigation_bar import NavigationBar

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
(ICON_VIEW_ICON,
 ICON_VIEW_NAME,
 ICON_VIEW_OBJ
 ) = range(3)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: icon mode
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class IconView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__control = control
        self.__mediator = pymobius.mediator.copy()
        self.__icons = {}
        self.name = 'Icon View'

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'view_icon.png')
        self.icon_data = open(path, 'rb').read()

        # build widget
        self.__widget = mobius.ui.container()
        self.__widget.show()

        vbox = mobius.ui.box(mobius.ui.box.orientation_vertical)
        vbox.set_spacing(5)
        vbox.set_visible(True)
        self.__widget.set_content(vbox)

        # navigation bar
        self.__navigation_bar = NavigationBar(control)
        self.__navigation_bar.show()
        vbox.add_child(self.__navigation_bar.get_ui_widget(), mobius.ui.box.fill_none)

        # icon view
        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        vbox.add_child(sw, mobius.ui.box.fill_with_widget)

        model = Gtk.ListStore.new([GdkPixbuf.Pixbuf, str, object])

        self.__iconview = Gtk.IconView.new_with_model(model)
        self.__iconview.set_pixbuf_column(0)
        self.__iconview.set_text_column(1)
        self.__iconview.set_item_width(192)
        self.__iconview.set_selection_mode(Gtk.SelectionMode.MULTIPLE)
        self.__iconview.connect('item-activated', self.__on_item_activated)
        self.__iconview.connect('selection-changed', self.__on_selection_changed)
        self.__iconview.show()
        sw.add(self.__iconview)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, folder):

        try:
            self.__folder = folder
            self.__retrieve_folder(folder)
            self.__widget.show_content()

        except Exception as e:
            self.__widget.set_message(str(e))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save current state
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_folder(self, folder):
        model = Gtk.ListStore.new([GdkPixbuf.Pixbuf, str, object])

        # order folders and then files by name (case-insensitive)
        folders = list(
            sorted(((c.name, c) for c in folder.get_children() if c.is_folder()), key=lambda v: v[0].lower()))
        files = list(
            sorted(((c.name, c) for c in folder.get_children() if not c.is_folder()), key=lambda v: v[0].lower()))

        for name, entry in folders + files:
            icon = self.__get_icon(entry)
            model.append((icon, entry.name, entry))

        self.__iconview.set_model(model)
        self.__navigation_bar.set_folder(folder)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get icon for entry
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_icon(self, entry):

        if entry.is_folder():
            icon_id = 'folder'
        else:
            icon_id = 'file'

        icon = self.__icons.get(icon_id)

        if not icon:
            path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, icon_id + '.svg')
            image = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_large)
            icon = image.get_ui_widget().get_pixbuf()
            self.__icons[icon_id] = icon

        return icon

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle selection changed event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_selection_changed(self, widget, *args):
        entries = []
        model = self.__iconview.get_model()

        for treepath in widget.get_selected_items():
            e = model[treepath][ICON_VIEW_OBJ]
            entries.append(e)

        self.__control.on_entry_selection_changed(entries)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_item_activated event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_item_activated(self, widget, path, *args):
        model = widget.get_model()
        entry = model[path][ICON_VIEW_OBJ]

        if entry.is_folder():
            self.__control.on_folder_activated(entry)
