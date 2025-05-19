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
import traceback

import mobius
import mobius.core.io
import pymobius
from gi.repository import Gtk

from compact_view import CompactView
from file_finder_view import FileFinderView
from file_view import FileView
from icon_view import IconView
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief VFS Entry class
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class vfs_entry(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, f, parent):
        self.__folder_obj = f
        self.__parent = parent

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get attribute
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __getattr__(self, name):
        if name in self.__dict__:
            return name

        return getattr(self.__folder_obj, name)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get parent folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_parent(self):
        return self.__parent

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get children
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_children(self):
        children = []

        try:
            for c in self.__folder_obj.get_children():
                e = vfs_entry(c, self)
                children.append(e)
        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

        return children


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief VFS Root Folder class
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class vfs_root_folder(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, vfs):
        self.__children = []
        self.__vfs = vfs
        self.path = '/'

        for root_entry in vfs.get_root_entries():
            e = vfs_entry(root_entry, self)
            self.__children.append(e)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get children
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_children(self):
        return self.__children

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get parent
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_parent(self):
        return None


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief File Explorer View
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class FileExplorerView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__item = None
        self.__folder = None
        self.__selected_entries = []

        self.name = f'{EXTENSION_NAME} v{EXTENSION_VERSION}'
        icon_path = self.__mediator.call('extension.get-icon-path', EXTENSION_ID)
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.core.ui.container()
        self.__widget.show()

        self.__vpaned = Gtk.VPaned()
        self.__vpaned.set_border_width(5)
        self.__vpaned.show()
        self.__widget.set_content(self.__vpaned)

        position = mobius.framework.get_config('file-explorer.vpaned-position')

        if position:
            self.__vpaned.set_position(int(position))

        # views
        self.__view_selector = self.__mediator.call('ui.new-widget', 'view-selector')
        self.__view_selector.show()
        self.__vpaned.pack1(self.__view_selector.get_ui_widget(), True, True)

        view = CompactView(self)
        self.__view_selector.add('compact', view)

        view = IconView(self)
        self.__view_selector.add('icon', view)

        view = FileFinderView(self)
        self.__view_selector.add('finder', view)

        # toolbar buttons
        toolbar = self.__view_selector.get_toolbar()

        toolitem = Gtk.SeparatorToolItem()
        toolitem.show()
        toolbar.insert(toolitem, -1)

        image = mobius.core.ui.new_icon_by_name('go-up', mobius.core.ui.icon.size_toolbar)
        image.show()

        self.__go_up_toolbutton = Gtk.ToolButton.new(image.get_ui_widget(), '')
        self.__go_up_toolbutton.set_tooltip_text("Go up")
        self.__go_up_toolbutton.connect("clicked", self.__on_go_up)
        self.__go_up_toolbutton.show()
        toolbar.insert(self.__go_up_toolbutton, -1)

        image = mobius.core.ui.new_icon_by_name('document-save-as', mobius.core.ui.icon.size_toolbar)
        image.show()

        self.__export_toolbutton = Gtk.ToolButton.new(image.get_ui_widget(), '')
        self.__export_toolbutton.set_tooltip_text("Export...")
        self.__export_toolbutton.connect("clicked", self.__on_export)
        self.__export_toolbutton.set_sensitive(False)
        self.__export_toolbutton.show()
        toolbar.insert(self.__export_toolbutton, -1)

        # file view
        self.__fileview = FileView(self)
        self.__fileview.show()
        self.__vpaned.pack2(self.__fileview.get_ui_widget(), False, True)

        # start widget
        self.__widget.set_message('Select item to explore file tree')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
        if len(itemlist) == 1:
            self.__set_item(itemlist[0])

        else:
            self.__widget.set_message('Select item to explore file tree')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set selected item
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __set_item(self, item):
        if self.__item and self.__item.uid == item.uid:
            return

        datasource = item.get_datasource()

        if not datasource:
            self.__widget.set_message('Item has no datasource')

        elif datasource.get_type() == 'vfs':

            if datasource.is_available():
                folder = vfs_root_folder(datasource.get_vfs())
                self.__set_folder(folder)

            else:
                self.__widget.set_message('Datasource is not available')

        else:
            self.__widget.set_message('Item cannot be navigated')

        self.__item = item

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set current folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __set_folder(self, folder):
        self.__folder = folder
        self.__view_selector.set_data(folder)
        self.__widget.show_content()

        is_go_up_enabled = folder.get_parent() is not None
        self.__go_up_toolbutton.set_sensitive(is_go_up_enabled)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_go_up event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_go_up(self, widget, *args):
        parent = self.__folder.get_parent()
        self.__set_folder(parent)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_export event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export(self, widget, *args):

        # choose destination folder
        fs = Gtk.FileChooserDialog(title='Choose destination folder', action=Gtk.FileChooserAction.SELECT_FOLDER)
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        fs.set_do_overwrite_confirmation(True)

        # run dialog
        rc = fs.run()
        dest_folder = fs.get_filename()
        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        # export data
        for entry in self.__selected_entries:
            self.__export_entry(entry, dest_folder)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Export entry to destination folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __export_entry(self, entry, dest_folder):
        if entry.is_folder():
            self.__export_folder(entry, dest_folder)
        else:
            self.__export_file(entry, dest_folder)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Export folder to destination folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __export_folder(self, folder, dest_folder):
        path = dest_folder + '/' + folder.name

        f = mobius.core.io.new_folder_by_path(path)
        f.create()

        for child in folder.get_children():
            self.__export_entry(child, path)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Export file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __export_file(self, f, dest_folder):
        reader = f.new_reader()
        if not reader:
            return

        path = dest_folder + '/' + f.name
        fp = open(path, 'wb')
        data = reader.read(65536)

        while data:
            fp.write(data)
            data = reader.read(65536)

        fp.close()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_destroy view
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        for view in self.__view_selector:
            view.on_destroy()

        mobius.framework.set_config('file-explorer.vpaned-position', self.__vpaned.get_position())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_entry_selection_changed event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_entry_selection_changed(self, entries):
        self.__export_toolbutton.set_sensitive(bool(entries))
        self.__selected_entries = entries
        self.__fileview.set_data(entries)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_folder_activated
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_folder_activated(self, folder):
        self.__set_folder(folder)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    mobius.core.add_resource('view.file-explorer', 'View: File Explorer', FileExplorerView)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('view.file-explorer')
