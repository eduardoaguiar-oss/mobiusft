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
import os.path
import shutil
import traceback

import mobius
import pymobius.registry.main
from gi.repository import Gtk

from filelist_view import FileListView
from metadata import *
from registry_treeview import RegistryTreeView
from report_view import ReportView

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ANT_ID = 'registry'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get registry name
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_registry_name(registry):
    # get timestamp from SYSTEM or from any other system hivefile
    timestamp = None
    path = None
    name = None

    for f in registry.files:
        if not timestamp and f.role != 'NTUSER':
            timestamp = f.last_modification_time
            path = f.path

        elif f.role == 'SYSTEM':
            timestamp = f.last_modification_time
            path = f.path

    # build name
    if timestamp:
        name = '%s %s' % (timestamp, path.rsplit('\\', 1)[0])

    return name


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief widget constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
VIEW_FILELIST, VIEW_REGISTRY, VIEW_REPORT, VIEW_FILETREE = range(4)
REGISTRYVIEW_LOGICAL_MODE, REGISTRYVIEW_PHYSICAL_MODE = range(2)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief view: Win registry
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class HiveView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__data = []
        app = mobius.core.application()

        self.name = '%s v%s (Registry Viewer)' % (EXTENSION_NAME, EXTENSION_VERSION)
        icon_path = self.__mediator.call('extension.get-icon-path', EXTENSION_ID)
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.core.ui.container()
        self.__widget.show()

        self.__view_selector = self.__mediator.call('ui.new-widget', 'view-selector')
        self.__view_selector.show()
        self.__widget.set_content(self.__view_selector.get_ui_widget())

        view = FileListView()
        self.__view_selector.add('hivefiles', view)

        view = RegistryTreeView(REGISTRYVIEW_LOGICAL_MODE)
        self.__view_selector.add('logical-view', view)

        view = RegistryTreeView(REGISTRYVIEW_PHYSICAL_MODE)
        self.__view_selector.add('physical-view', view)

        view = ReportView()
        self.__view_selector.add('report', view)

        # export registry toolitem
        toolbar = self.__view_selector.get_toolbar()

        toolitem = Gtk.SeparatorToolItem()
        toolitem.show()
        toolbar.insert(toolitem, -1)

        self.__refresh_toolitem = Gtk.ToolButton.new()
        self.__refresh_toolitem.set_icon_name('view-refresh')
        self.__refresh_toolitem.set_sensitive(False)
        self.__refresh_toolitem.show()
        self.__refresh_toolitem.set_tooltip_text("Reload registry files")
        self.__refresh_toolitem.connect("clicked", self.__on_registry_reload)
        toolbar.insert(self.__refresh_toolitem, -1)

        self.__export_toolitem = Gtk.ToolButton.new()
        self.__export_toolitem.set_icon_name('document-save-as')
        self.__export_toolitem.set_sensitive(False)
        self.__export_toolitem.show()
        self.__export_toolitem.set_tooltip_text("Export registry files...")
        self.__export_toolitem.connect("clicked", self.__on_registry_export)
        toolbar.insert(self.__export_toolitem, -1)

        # registry selection menu tool item
        icon = mobius.core.ui.new_icon_by_name('go-jump', mobius.core.ui.icon.size_toolbar)
        icon.set_visible(True)

        self.__registry_menu_toolitem = Gtk.MenuToolButton.new(icon.get_ui_widget(), '')
        self.__registry_menu_toolitem.set_sensitive(False)
        self.__registry_menu_toolitem.show()
        self.__registry_menu_toolitem.set_tooltip_text("Select registry...")
        toolbar.insert(self.__registry_menu_toolitem, -1)

        # view data
        self.__widget.set_message('Select item to view registry data')
        self.__registry = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):

        if len(itemlist) == 1:
            self.__retrieve_item(itemlist[0])

        else:
            self.__widget.set_message('Select item to view registry data')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_destroy view
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        for view in self.__view_selector:
            view.on_destroy()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve registry data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_item(self, item):
        self.__item = item
        self.__registry = None
        self.__registry_idx = -1

        ant = pymobius.registry.main.Ant(item)

        # show "retrieving message", if necessary
        if not item.has_ant(ANT_ID):
            self.__widget.set_message('Retrieving registry files...')
            mobius.core.ui.flush()

        # retrieve data
        try:
            self.__data = ant.get_data()
            error_msg = None

        except Exception as e:
            self.__data = None
            error_msg = str(e)
            mobius.core.logf(f'ERR {str(e)}\n{traceback.format_exc()}')

        # if it has data, show
        if self.__data:
            self.__registry_idx = 0
            self.__registry = self.__data[self.__registry_idx]
            self.__view_selector.set_data(self.__registry)
            self.__export_toolitem.set_sensitive(True)
            self.__refresh_toolitem.set_sensitive(True)
            self.__widget.show_content()

            # populate menu_tool_button
            menu = Gtk.Menu()
            menu.show()

            for idx, registry in enumerate(self.__data):
                name = get_registry_name(registry) or f'Registry {idx + 1:d}'
                menuitem = Gtk.MenuItem.new_with_label(name)
                menuitem.connect('activate', self.__on_registry_selected, idx)
                menu.append(menuitem)
                menuitem.show()

            self.__registry_menu_toolitem.set_menu(menu)
            self.__registry_menu_toolitem.set_sensitive(True)

            # @todo call turing to get hashes and passwords
            for r in self.__data:
                pass  # self.__mediator.emit ('registry.updated', r)

        else:
            self.__widget.set_message(error_msg or 'Item has no registry data')
            self.__export_toolitem.set_sensitive(False)
            self.__refresh_toolitem.set_sensitive(False)
            self.__registry_menu_toolitem.set_sensitive(False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Select new registry
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_registry_selected(self, widget, idx, *args):
        if idx != self.__registry_idx:
            self.__registry = self.__data[idx]
            self.__registry_idx = idx
            self.__view_selector.set_data(self.__registry)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Reload registry data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_registry_reload(self, widget, *args):
        ant = pymobius.registry.main.Ant(self.__item)
        ant.remove_data()
        self.__retrieve_item(self.__item)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Export registry files to folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_registry_export(self, widget, *args):

        # choose destination folder
        fs = Gtk.FileChooserDialog(title='Export to folder', action=Gtk.FileChooserAction.SELECT_FOLDER)
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        fs.set_do_overwrite_confirmation(True)

        # run dialog
        rc = fs.run()
        dest_dir = fs.get_filename()
        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        # export data
        source_dir = self.__item.get_data_path(os.path.join('registry', f'{self.__registry_idx + 1:04d}'))

        for filename in os.listdir(source_dir):
            shutil.copy(os.path.join(source_dir, filename), dest_dir)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    mobius.core.add_resource('view.registry', 'Hive view', HiveView)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('view.registry')
