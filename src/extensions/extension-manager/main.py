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
import glob

import mobius
import pymobius
from gi.repository import Gtk

from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
COLOR_UPDATE = '#0000FF'

(TREEITEM_ICON_NAME,
 TREEITEM_ID,
 TREEITEM_NAME,
 TREEITEM_INSTALLED_VERSION,
 TREEITEM_AVAILABLE_VERSION,
 TREEITEM_COLOR,
 TREEITEM_OBJECT) = range(7)

ICON_NAME_RUNNING = Gtk.STOCK_YES
ICON_NAME_NOT_RUNNING = Gtk.STOCK_NO


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Extension proxy
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Extension(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = None
        self.name = None
        self.author = None
        self.description = None
        self.installed_version = None
        self.available_version = None
        self.available_path = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Return True if extension is installed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def is_installed(self):
        return self.id in pymobius.mediator.call('extension.list')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Return True if extension is running
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def is_running(self):
        return pymobius.mediator.call('extension.is-running', self.id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Return True if extension is updateable/installable
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def is_updateable(self):
        flag = False

        if self.available_version:
            if self.installed_version:
                installed_version = [int(n) for n in self.installed_version.split('.')]
                available_version = [int(n) for n in self.available_version.split('.')]

                if available_version > installed_version:
                    flag = True

            else:
                flag = True  # new extension.

        return flag

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get icon data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_icon_data(self):

        if self.is_installed():
            icon_path = pymobius.mediator.call('extension.get-icon-path', self.id)
            data = open(icon_path, 'rb').read()

        else:
            data = pymobius.mediator.call('extension.get-icon-data-from-file', self.available_path)

        return data

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Start extension
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def start(self):
        pymobius.mediator.call('extension.start-api', self.id)
        pymobius.mediator.call('extension.start', self.id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Stop extension
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def stop(self):
        pymobius.mediator.call('extension.stop', self.id)
        pymobius.mediator.call('extension.stop-api', self.id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Install extension
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def install(self):
        pymobius.mediator.call('extension.install', self.available_path)
        self.installed_version = self.available_version

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Uninstall extension
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def uninstall(self):
        pymobius.mediator.call('extension.uninstall', self.id)
        self.installed_version = None


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Create extension object from ID
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def extension_new_from_id(extension_id):
    metadata = pymobius.mediator.call('extension.get-metadata', extension_id)

    extension = Extension()
    extension.id = extension_id
    extension.name = metadata.name
    extension.installed_version = metadata.version
    extension.author = metadata.author
    extension.description = metadata.description

    return extension


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Create extension object from a file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def extension_new_from_path(path):
    metadata = pymobius.mediator.call('extension.get-metadata-from-file', path)

    extension = Extension()
    extension.id = metadata.id
    extension.name = metadata.name
    extension.author = metadata.author
    extension.description = metadata.description
    extension.available_version = metadata.version
    extension.available_path = path

    return extension


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Extension manager data model
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class DataModel(object):

    def __init__(self):
        self.__extensions = {}

    def __iter__(self):
        return iter(self.__extensions.values())

    def retrieve(self):
        self.__extensions = {}

        # retrieve running extensions
        for extension_id in pymobius.mediator.call('extension.list'):
            extension = extension_new_from_id(extension_id)
            self.__extensions[extension.id] = extension

        # retrieve available extensions
        app = mobius.core.application()
        mobius_directory = app.get_data_path('extensions')

        for path in glob.glob('%s/*.mobius' % mobius_directory):
            extension = extension_new_from_path(path)

            if extension.id in self.__extensions:
                installed_extension = self.__extensions[extension.id]
                extension.installed_version = installed_extension.installed_version
            self.__extensions[extension.id] = extension


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Extension manager widget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Widget(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Build widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, mediator, *args):
        self.__widget = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        self.__widget.set_border_width(10)
        self.__widget.set_spacing(5)
        self.__mediator = mediator.copy()

        # menubar
        menubar = Gtk.MenuBar()
        menubar.show()
        self.__widget.add_child(menubar, mobius.core.ui.box.fill_none)

        item = Gtk.MenuItem.new_with_mnemonic('_File')
        item.show()
        menubar.append(item)

        menu = Gtk.Menu()
        menu.show()
        item.set_submenu(menu)

        item = Gtk.MenuItem.new_with_mnemonic('_Install from file...')
        item.connect("activate", self.on_extension_install_from_file)
        item.show()
        menu.append(item)

        self.uninstall_menuitem = Gtk.MenuItem.new_with_mnemonic('_Uninstall extensions...')
        self.uninstall_menuitem.connect("activate", self.on_extension_uninstall)
        self.uninstall_menuitem.set_sensitive(False)
        self.uninstall_menuitem.show()
        menu.append(self.uninstall_menuitem)

        self.update_menuitem = Gtk.MenuItem.new_with_mnemonic('_Install/update extensions')
        self.update_menuitem.connect("activate", self.on_extension_update)
        self.update_menuitem.set_sensitive(False)
        self.update_menuitem.show()
        menu.append(self.update_menuitem)

        item = Gtk.SeparatorMenuItem.new()
        item.show()
        menu.append(item)

        item = Gtk.MenuItem.new_with_mnemonic('_Quit')
        item.connect("activate", self.on_extension_close)
        item.show()
        menu.append(item)

        item = Gtk.MenuItem.new_with_mnemonic('_Action')
        item.show()
        menubar.append(item)

        menu = Gtk.Menu()
        menu.show()
        item.set_submenu(menu)

        self.start_menuitem = Gtk.MenuItem.new_with_mnemonic('St_art extension')
        self.start_menuitem.connect("activate", self.on_extension_start)
        self.start_menuitem.set_sensitive(False)
        self.start_menuitem.show()
        menu.append(self.start_menuitem)

        self.stop_menuitem = Gtk.MenuItem.new_with_mnemonic('St_op extension')
        self.stop_menuitem.connect("activate", self.on_extension_stop)
        self.stop_menuitem.set_sensitive(False)
        self.stop_menuitem.show()
        menu.append(self.stop_menuitem)

        # toolbar
        toolbar = Gtk.Toolbar()
        toolbar.set_style(Gtk.ToolbarStyle.ICONS)
        toolbar.show()
        self.__widget.add_child(toolbar, mobius.core.ui.box.fill_none)

        toolitem = Gtk.ToolButton.new()
        toolitem.set_icon_name('list-add')
        toolitem.connect("clicked", self.on_extension_install_from_file)
        toolitem.show()
        toolitem.set_tooltip_text("install extension from file")
        toolbar.insert(toolitem, -1)

        toolitem = Gtk.SeparatorToolItem()
        toolitem.show()
        toolbar.insert(toolitem, -1)

        self.start_toolitem = Gtk.ToolButton.new()
        self.start_toolitem.set_icon_name('media-playback-start')
        self.start_toolitem.connect("clicked", self.on_extension_start)
        self.start_toolitem.set_sensitive(False)
        self.start_toolitem.show()
        self.start_toolitem.set_tooltip_text("start extension")
        toolbar.insert(self.start_toolitem, -1)

        self.stop_toolitem = Gtk.ToolButton.new()
        self.stop_toolitem.set_icon_name('media-playback-stop')
        self.stop_toolitem.connect("clicked", self.on_extension_stop)
        self.stop_toolitem.set_sensitive(False)
        self.stop_toolitem.show()
        self.stop_toolitem.set_tooltip_text("stop extension")
        toolbar.insert(self.stop_toolitem, -1)

        # extensions listview
        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        self.__widget.add_child(sw, mobius.core.ui.box.fill_with_widget)

        model = Gtk.ListStore.new([str, str, str, str, str, str, object])
        model.set_sort_column_id(TREEITEM_NAME, Gtk.SortType.ASCENDING)

        self.listview = Gtk.TreeView.new_with_model(model)
        self.listview.show()

        selection = self.listview.get_selection()
        selection.set_mode(Gtk.SelectionMode.MULTIPLE)
        selection.connect('changed', self.on_extension_selection_changed)

        renderer = Gtk.CellRendererPixbuf()
        tvcolumn = Gtk.TreeViewColumn()
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'icon-name', TREEITEM_ICON_NAME)
        self.listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('Extension')
        tvcolumn.set_resizable(True)
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', TREEITEM_NAME)
        tvcolumn.add_attribute(renderer, 'foreground', TREEITEM_COLOR)
        self.listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('Installed')
        tvcolumn.set_resizable(True)
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', TREEITEM_INSTALLED_VERSION)
        tvcolumn.add_attribute(renderer, 'foreground', TREEITEM_COLOR)
        self.listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('Available')
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', TREEITEM_AVAILABLE_VERSION)
        tvcolumn.add_attribute(renderer, 'foreground', TREEITEM_COLOR)
        self.listview.append_column(tvcolumn)

        sw.add(self.listview)

        # extension details
        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(10)
        hbox.set_visible(True)
        self.__widget.add_child(hbox, mobius.core.ui.box.fill_none)

        self.__detail_icon = mobius.core.ui.new_icon_by_name('image-missing', mobius.core.ui.icon.size_large)
        self.__detail_icon.set_visible(True)
        hbox.add_child(self.__detail_icon, mobius.core.ui.box.fill_none)

        self.detail_label = Gtk.Label.new()
        self.detail_label.set_justify(Gtk.Justification.LEFT)
        self.detail_label.set_single_line_mode(False)
        self.detail_label.set_line_wrap(True)
        self.detail_label.set_xalign(0.0)
        self.detail_label.set_yalign(0.5)
        self.detail_label.set_markup('<b>No extension selected</b>')
        self.detail_label.show()
        hbox.add_child(self.detail_label, mobius.core.ui.box.fill_with_widget)

        # status bar
        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.IN)
        frame.show()
        self.__widget.add_child(frame, mobius.core.ui.box.fill_none)

        self.status_label = mobius.core.ui.label()
        self.status_label.set_selectable(True)
        self.status_label.set_halign(mobius.core.ui.label.align_left)
        self.status_label.set_visible(True)
        frame.add(self.status_label.get_ui_widget())

        # show info
        self.retrieve_extensions()
        self.__widget.set_visible(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_widget(self):
        return self.__widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle close button
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_extension_close(self, widget, *args):
        self.__mediator.call('ui.working-area.close', self.working_area.id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Show save/ignore/cancel dialog if there are modified items
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_widget_stopped(self):
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle extension->selection changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_extension_selection_changed(self, selection, *args):
        model, pathlist = selection.get_selected_rows()

        if len(pathlist) == 1:
            self.on_extension_single_selection(selection)

        elif len(pathlist) > 1:
            count = selection.count_selected_rows()
            self.__detail_icon.set_icon_by_name('image-missing', mobius.core.ui.icon.size_large)
            self.detail_label.set_markup(f'<b>{count:d} extensions selected</b>')

        else:  # Nothing selected
            self.__detail_icon.set_icon_by_name('image-missing', mobius.core.ui.icon.size_large)
            self.detail_label.set_markup('<b>No extension selected</b>')

        self.update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle single selection
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_extension_single_selection(self, selection):
        model, pathlist = selection.get_selected_rows()
        treeiter = model.get_iter(pathlist[0])
        extension = model.get_value(treeiter, TREEITEM_OBJECT)

        icon_data = extension.get_icon_data()
        if icon_data:
            self.__detail_icon.set_icon_from_data(icon_data, mobius.core.ui.icon.size_large)

        text = f'<b>{extension.name}</b>'

        if extension.is_installed():
            version = extension.installed_version
        else:
            version = extension.available_version
        text += f' v{version}.'

        if extension.author:
            text += f' By {extension.author}.'

        if extension.description:
            text += f' {extension.description}.'

        self.detail_label.set_markup(text)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle extension->install from file event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_extension_install_from_file(self, widget, *args):

        # choose file(s)
        fs = Gtk.FileChooserDialog(title='Choose extension files')
        fs.set_select_multiple(True)
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)

        filefilter = Gtk.FileFilter()
        filefilter.set_name('Mobius extension (*.mobius)')
        filefilter.add_pattern('*.mobius')
        fs.add_filter(filefilter)

        rc = fs.run()
        pathlist = fs.get_filenames()
        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        # install extensions
        count = 0

        for path in pathlist:
            extension = extension_new_from_path(path)

            if self.install_extension(extension) and self.start_extension(extension):
                count += 1

        # update window
        if count > 1:
            self.status_label.set_text(f'{count:d} extensions were installed')

        self.retrieve_extensions()
        self.update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle extension->uninstall event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_extension_uninstall(self, widget, *args):

        # show confirmation dialog
        dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
        dialog.text = "You are about to uninstall extension(s). Are you sure?"
        dialog.add_button(mobius.core.ui.message_dialog.button_yes)
        dialog.add_button(mobius.core.ui.message_dialog.button_no)
        dialog.set_default_response(mobius.core.ui.message_dialog.button_no)
        rc = dialog.run()

        if rc != mobius.core.ui.message_dialog.button_yes:
            return

        # uninstall extensions
        model, pathlist = self.listview.get_selection().get_selected_rows()
        treeiterlist = [model.get_iter(path) for path in pathlist]
        count = 0

        for treeiter in treeiterlist:
            extension = model.get_value(treeiter, TREEITEM_OBJECT)

            if extension.id != EXTENSION_ID and self.uninstall_extension(extension):
                if extension.available_version:
                    self.update_listview_row(treeiter)
                else:
                    model.remove(treeiter)

                count += 1

        # update window
        if count > 1:
            self.status_label.set_text(f'{count:d} extensions were uninstalled')

        self.update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle extension->install/update event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_extension_update(self, widget, *args):
        model, pathlist = self.listview.get_selection().get_selected_rows()
        count = 0

        # update extensions
        for path in pathlist:
            treeiter = model.get_iter(path)
            extension = model.get_value(treeiter, TREEITEM_OBJECT)

            if extension.is_updateable() and self.install_extension(extension) and self.start_extension(extension):
                self.update_listview_row(treeiter)
                count += 1

        # update window
        if count > 1:
            self.status_label.set_text(f'{count:d} extensions were updated')

        self.update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle extension->start event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_extension_start(self, widget, *args):
        model, pathlist = self.listview.get_selection().get_selected_rows()
        count = 0

        for path in pathlist:
            treeiter = model.get_iter(path)
            extension = model.get_value(treeiter, TREEITEM_OBJECT)

            if extension.is_installed() and not extension.is_running() and self.start_extension(extension):
                self.update_listview_row(treeiter)
                count += 1

        # update window
        if count > 1:
            self.status_label.set_text(f'{count:d} extensions were started')

        self.update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle extension->stop event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_extension_stop(self, widget, *args):
        model, pathlist = self.listview.get_selection().get_selected_rows()
        count = 0

        for path in pathlist:
            treeiter = model.get_iter(path)
            extension = model.get_value(treeiter, TREEITEM_OBJECT)

            if extension.id != EXTENSION_ID and extension.is_running() and self.stop_extension(extension):
                self.update_listview_row(treeiter)
                count += 1

        # update window
        if count > 1:
            self.status_label.set_text(f'{count:d} extensions were stopped')

        self.update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Install extension
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def install_extension(self, extension):

        if extension.is_running() and not self.stop_extension(extension):
            installed_ok = False

        else:
            try:
                self.status_label.set_text(
                    f'Installing extension "{extension.name} v{extension.available_version}"')
                extension.install()
                self.status_label.set_text(
                    f'Extension "{extension.name} v{extension.available_version}" installed')
                installed_ok = True

            except Exception as e:
                self.status_label.set_text(
                    f'Error installing extension "{extension.name} v{extension.available_version}": {e}')
                installed_ok = False

        return installed_ok

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Uninstall extension
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def uninstall_extension(self, extension):

        if extension.is_running() and not self.stop_extension(extension):
            uninstalled_ok = False

        else:
            try:
                self.status_label.set_text(f'Uninstalling extension "{extension.name}"')
                extension.uninstall()
                self.status_label.set_text(f'Extension "{extension.name}" uninstalled')
                uninstalled_ok = True

            except Exception as e:
                self.status_label.set_text(f'Error uninstalling extension "{extension.name}": {e}')
                uninstalled_ok = False

        return uninstalled_ok

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Start extension
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def start_extension(self, extension):

        try:
            extension.start()
            self.status_label.set_text(f'Extension "{extension.name}" started')
            started_ok = True

        except Exception as e:
            self.status_label.set_text(f'Error starting extension "{extension.name}": {e}')
            started_ok = False

        return started_ok

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Stop extension
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def stop_extension(self, extension):

        try:
            extension.stop()
            self.status_label.set_text(f"Extension '{extension.name}' stopped")
            stopped_ok = True

        except Exception as e:
            self.status_label.set_text(f"Error stopping extension '{extension.name}': {e}")
            stopped_ok = False

        return stopped_ok

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update listview row
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def update_listview_row(self, treeiter):
        model = self.listview.get_model()
        extension = model.get_value(treeiter, TREEITEM_OBJECT)

        # running icon
        if extension.is_installed():
            if extension.is_running():
                icon_name = ICON_NAME_RUNNING
            else:
                icon_name = ICON_NAME_NOT_RUNNING
        else:
            icon_name = ''

        model.set_value(treeiter, TREEITEM_ICON_NAME, icon_name)

        # text
        model.set_value(treeiter, TREEITEM_NAME, extension.name)
        model.set_value(treeiter, TREEITEM_INSTALLED_VERSION, extension.installed_version)
        model.set_value(treeiter, TREEITEM_AVAILABLE_VERSION, extension.available_version)

        # updateable?
        if extension.is_updateable():
            color = COLOR_UPDATE
        else:
            color = None
        model.set_value(treeiter, TREEITEM_COLOR, color)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief retrieve extensions data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def retrieve_extensions(self):
        datastore = self.listview.get_model()
        datastore.clear()

        model = DataModel()
        model.retrieve()

        for extension in model:

            if extension.is_installed():
                if extension.is_running():
                    icon_name = ICON_NAME_RUNNING
                else:
                    icon_name = ICON_NAME_NOT_RUNNING
            else:
                icon_name = ''

            if extension.is_updateable():
                color = COLOR_UPDATE
            else:
                color = None

            datastore.append((icon_name, extension.id, extension.name, extension.installed_version,
                              extension.available_version, color, extension))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set options according to items selected
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def update_options(self):
        selection = self.listview.get_selection()
        model, pathlist = selection.get_selected_rows()

        # check available options
        can_uninstall = False
        can_start = False
        can_stop = False
        can_update = False

        for path in pathlist:
            treeiter = model.get_iter(path)
            extension = model.get_value(treeiter, TREEITEM_OBJECT)

            # extension-manager cannot be uninstalled, started or stopped
            if extension.id != EXTENSION_ID and extension.is_installed():
                can_uninstall = True

                if extension.is_running():
                    can_stop = True
                else:
                    can_start = True

            if extension.is_updateable():
                can_update = True

        # set available options
        self.uninstall_menuitem.set_sensitive(can_uninstall)
        self.update_menuitem.set_sensitive(can_update)
        self.start_menuitem.set_sensitive(can_start)
        self.stop_menuitem.set_sensitive(can_stop)
        self.start_toolitem.set_sensitive(can_start)
        self.stop_toolitem.set_sensitive(can_stop)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    icon = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)
    mobius.core.add_resource('menu.tools.' + EXTENSION_ID, 'Menu Tools: Extension Manager',
                             (icon, EXTENSION_NAME, on_activate))


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('menu.tools.' + EXTENSION_ID)
    pymobius.mediator.call('ui.working-area.del', EXTENSION_ID)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief on_activate
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def on_activate(extension_id):
    working_area = pymobius.mediator.call('ui.working-area.get', EXTENSION_ID)

    if not working_area:
        widget = Widget(pymobius.mediator.copy())
        icon_path = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)

        working_area = pymobius.mediator.call('ui.working-area.new', EXTENSION_ID)
        working_area.set_default_size(400, 500)
        working_area.set_title(EXTENSION_NAME)
        working_area.set_icon(icon_path)
        working_area.set_widget(widget)

    working_area.show()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Handle <app.starting> event
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def cb_app_starting():
    # check new version
    app = mobius.core.application()
    current_version = mobius.framework.get_config('general.current-version')

    if current_version and current_version != app.version:

        # show dialog
        dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
        dialog.text = f"Welcome to Mobius v{app.version}. Would you like to check for new extensions?"
        dialog.add_button(mobius.core.ui.message_dialog.button_yes)
        dialog.add_button(mobius.core.ui.message_dialog.button_no)
        dialog.add_button('Remind me later')
        rc = dialog.run()

        # if response = YES, set current version and show extension manager
        if rc == mobius.core.ui.message_dialog.button_yes:
            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('general.current-version', app.version)
            transaction.commit()
            on_activate(EXTENSION_ID)

        # else if response = NO, just set current version
        elif rc == mobius.core.ui.message_dialog.button_no:
            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('general.current-version', app.version)
            transaction.commit()

        # otherwise, do nothing, and this dialog will be shown next time


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief API initialization
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start_api():
    pymobius.mediator.connect('app.starting', cb_app_starting)
