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
import mobius
import pymobius
from gi.repository import GObject
from gi.repository import GdkPixbuf
from gi.repository import Gtk

from metadata import *
from navigation_bar import NavigationBar

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
(COMPACT_VIEW_ICON,
 COMPACT_VIEW_NAME,
 COMPACT_VIEW_SIZE,
 COMPACT_VIEW_CRTIME,
 COMPACT_VIEW_MTIME,
 COMPACT_VIEW_ATIME,
 COMPACT_VIEW_OBJ
 ) = range(7)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: Compact mode
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CompactView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__control = control
        self.__mediator = pymobius.mediator.copy()
        self.__category_icons = {}
        self.name = 'Compact View'

        self.__icons = {}
        icon_path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'view_compact.png')
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.core.ui.container()
        self.__widget.set_visible(True)

        vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbox.set_spacing(5)
        vbox.set_visible(True)
        self.__widget.set_content(vbox)

        # navigation bar
        self.__navigation_bar = NavigationBar(control)
        self.__navigation_bar.show()
        vbox.add_child(self.__navigation_bar.get_ui_widget(), mobius.core.ui.box.fill_none)

        # view
        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        vbox.add_child(sw, mobius.core.ui.box.fill_with_widget)

        model = Gtk.ListStore.new([GdkPixbuf.Pixbuf, str, GObject.TYPE_LONG, str, str, str, object])

        self.__listview = Gtk.TreeView.new_with_model(model)
        self.__listview.connect('row-activated', self.__on_row_activated)
        self.__listview.show()

        selection = self.__listview.get_selection()
        selection.set_mode(Gtk.SelectionMode.MULTIPLE)
        selection.connect('changed', self.__on_selection_changed)

        renderer = Gtk.CellRendererPixbuf()
        tvcolumn = Gtk.TreeViewColumn()
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'pixbuf', COMPACT_VIEW_ICON)
        self.__listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('Name')
        tvcolumn.set_resizable(True)
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', COMPACT_VIEW_NAME)
        tvcolumn.set_sort_column_id(1)
        self.__listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        renderer.set_property('xalign', 1.0)

        tvcolumn = Gtk.TreeViewColumn('Size (bytes)')
        tvcolumn.set_resizable(True)
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', COMPACT_VIEW_SIZE)
        tvcolumn.set_sort_column_id(2)
        self.__listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('Created')
        tvcolumn.set_resizable(True)
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', COMPACT_VIEW_CRTIME)
        tvcolumn.set_sort_column_id(3)
        self.__listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('Last Modified')
        tvcolumn.set_resizable(True)
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', COMPACT_VIEW_MTIME)
        tvcolumn.set_sort_column_id(4)
        self.__listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('Last Accessed')
        tvcolumn.set_resizable(True)
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', COMPACT_VIEW_ATIME)
        tvcolumn.set_sort_column_id(5)
        self.__listview.append_column(tvcolumn)

        sw.add(self.__listview)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save current state
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        self.__mediator.clear()

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
    # @brief set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_folder(self, folder):
        model = self.__listview.get_model()
        model.clear()

        # order folders and then files by name (case insensitive)
        folders = list(
            sorted(((c.name, c) for c in folder.get_children() if c.is_folder()), key=lambda v: v[0].lower()))
        files = list(
            sorted(((c.name, c) for c in folder.get_children() if not c.is_folder()), key=lambda v: v[0].lower()))

        for name, entry in folders + files:
            icon = self.__get_icon(entry)
            crtime = str(entry.creation_time or '')
            mtime = str(entry.modification_time or '')
            atime = str(entry.access_time or '')

            model.append((icon, entry.name, entry.size, crtime, mtime, atime, entry))

        if len(model) > 0:
            self.__listview.scroll_to_cell((0,))

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
            image = mobius.core.ui.new_icon_by_path(path, mobius.core.ui.icon.size_toolbar)
            icon = image.get_ui_widget().get_pixbuf()
            self.__icons[icon_id] = icon

        return icon

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle selection changed event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_selection_changed(self, selection, *args):
        entries = []
        model, pathlist = selection.get_selected_rows()

        for treepath in pathlist:
            e = model[treepath][COMPACT_VIEW_OBJ]
            entries.append(e)

        self.__control.on_entry_selection_changed(entries)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle button press event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_row_activated(self, widget, row, col, *args):
        model = widget.get_model()
        entry = model[row][COMPACT_VIEW_OBJ]

        if entry.is_folder():
            self.__control.on_folder_activated(entry)
