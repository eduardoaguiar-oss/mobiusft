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
import os
import os.path

import mobius
import mobius.framework
import pymobius
from gi.repository import Gtk

from about_dialog import AboutDialog
from case_view import CaseView
from metadata import *
from new_case_dialog import NewCaseDialog

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
VIEW_CASE_ITEMS = range(1)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief ICE window
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ICEWindow(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__case = None

        # window
        self.__widget = mobius.core.ui.window()
        self.__widget.set_visible(True)
        self.__widget.set_callback('closing', self.__on_window_closing)

        # set window size
        if mobius.framework.has_config('ice.window.width'):
            width = mobius.framework.get_config('ice.window.width')
            height = mobius.framework.get_config('ice.window.height')
            self.__widget.set_size(width, height)

        else:
            self.__widget.set_size(800, 600)

        # set window icon
        path = self.__mediator.call('extension.get-icon-path', EXTENSION_ID)
        icon = mobius.core.ui.new_icon_by_path(path, mobius.core.ui.icon.size_toolbar)
        self.__widget.set_icon(icon)

        # vbox
        vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbox.set_border_width(5)
        vbox.set_spacing(5)
        vbox.show()
        self.__widget.set_content(vbox)

        # menubar
        menubar = Gtk.MenuBar()
        menubar.show()
        vbox.add_child(menubar, mobius.core.ui.box.fill_none)

        item = Gtk.MenuItem.new_with_mnemonic('_File')
        item.show()
        menubar.append(item)

        menu = Gtk.Menu()
        menu.show()
        item.set_submenu(menu)

        item = Gtk.MenuItem.new_with_mnemonic('_New')
        item.connect("activate", self.__on_file_new)
        item.show()
        menu.append(item)

        item = Gtk.MenuItem.new_with_mnemonic('_Open')
        item.connect("activate", self.__on_file_open)
        item.show()
        menu.append(item)

        item = Gtk.SeparatorMenuItem.new()
        item.show()
        menu.append(item)

        self.close_file_menuitem = Gtk.MenuItem.new_with_mnemonic("_Close")
        self.close_file_menuitem.set_sensitive(False)
        self.close_file_menuitem.connect("activate", self.__on_file_close)
        self.close_file_menuitem.show()
        menu.append(self.close_file_menuitem)

        item = Gtk.MenuItem.new_with_mnemonic("_Quit")
        item.connect("activate", self.__on_file_quit)
        item.show()
        menu.append(item)

        item = Gtk.MenuItem.new_with_mnemonic('_Tools')
        item.show()
        menubar.append(item)

        self.__tools_menu = Gtk.Menu()
        self.__tools_menu.show()
        item.set_submenu(self.__tools_menu)

        menu_tools = [(r.id, r.value) for r in mobius.core.get_resources('menu.tools')]
        menu_tools = list((title, item_id, icon_path, callback) for item_id, (icon_path, title, callback) in menu_tools)

        for (title, item_id, icon_path, callback) in sorted(menu_tools, key=lambda x: x[0]):
            self.__add_tools_menu_item(item_id, title, icon_path, callback)

        item = Gtk.MenuItem.new_with_mnemonic('_Help')
        item.show()
        menubar.append(item)

        menu = Gtk.Menu()
        menu.show()
        item.set_submenu(menu)

        item = Gtk.MenuItem.new_with_mnemonic('_About')
        item.connect("activate", self.__on_help_about)
        item.show()
        menu.append(item)

        # toolbar
        toolbar = Gtk.Toolbar()
        toolbar.set_style(Gtk.ToolbarStyle.ICONS)
        toolbar.show()
        vbox.add_child(toolbar, mobius.core.ui.box.fill_none)

        toolitem = Gtk.ToolButton.new()
        toolitem.set_icon_name('document-new')
        toolitem.connect("clicked", self.__on_file_new)
        toolitem.show()
        toolitem.set_tooltip_text("New case")
        toolbar.insert(toolitem, -1)

        toolitem = Gtk.ToolButton.new()
        toolitem.set_icon_name('document-open')
        toolitem.connect("clicked", self.__on_file_open)
        toolitem.show()
        toolitem.set_tooltip_text("Open case")
        toolbar.insert(toolitem, -1)

        # main container
        self.__case_view = CaseView()
        self.__case_view.show()
        vbox.add_child(self.__case_view.get_ui_widget(), mobius.core.ui.box.fill_with_widget)

        # update window
        self.__update_window()

        # connect to events
        self.__event_uid_1 = mobius.core.subscribe('attribute-modified', self.__on_attribute_modified)
        self.__event_uid_2 = mobius.core.subscribe('attribute-removed', self.__on_attribute_removed)
        self.__event_uid_3 = mobius.core.subscribe('resource-added', self.__on_resource_added)
        self.__event_uid_4 = mobius.core.subscribe('resource-removed', self.__on_resource_removed)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Show window
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def show(self):
        self.__widget.set_visible(True)

        if mobius.framework.has_config('ice.window.x'):
            x = mobius.framework.get_config('ice.window.x')
            y = mobius.framework.get_config('ice.window.y')
            self.__widget.set_position(x, y)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle window->closing event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_window_closing(self):

        # if there is at most one case opened, performs like a file->quit
        if mobius.framework.model.get_case_count() < 2:
            return self.__on_file_quit(self.__widget)

        # otherwise, performs like a file->close
        else:
            return self.__on_file_close(self.__widget)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set case object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __set_case(self, case):
        self.__case = case
        self.__case_view.set_case(case)
        self.__update_window()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get window for a case
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_window_case(self):

        # create new working area if necessary
        if not self.__case:
            window = self

        else:
            window = ICEWindow()
            window.show()

        # return window
        return window

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save configuration and destroy window
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __destroy(self):

        # get window position and size
        x, y = self.__widget.get_position()
        width, height = self.__widget.get_size()

        # save configuration
        transaction = mobius.framework.new_config_transaction()

        mobius.framework.set_config('ice.window.x', x)
        mobius.framework.set_config('ice.window.y', y)
        mobius.framework.set_config('ice.window.width', width)
        mobius.framework.set_config('ice.window.height', height)

        self.__case_view.on_destroy()

        transaction.commit()

        # destroy widget
        mobius.core.unsubscribe(self.__event_uid_1)
        mobius.core.unsubscribe(self.__event_uid_2)
        mobius.core.unsubscribe(self.__event_uid_3)
        mobius.core.unsubscribe(self.__event_uid_4)

        self.__mediator.clear()
        self.__widget.destroy()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update window controls and appearance
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __update_window(self):

        # enable/disable widgets
        if self.__case:
            self.close_file_menuitem.set_sensitive(True)
            self.__case_view.show()
        else:
            self.close_file_menuitem.set_sensitive(False)
            self.__case_view.hide()

        # update window's title
        app = mobius.core.application()
        title = f'{app.title} - {EXTENSION_NAME}'

        if self.__case:
            root = self.__case.root_item
            title += ' - ' + root.name

        self.__widget.set_title(title)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle file->new
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_file_new(self, widget, *args):

        # run new case dialog
        dialog = NewCaseDialog()
        response = dialog.run()

        case_id = dialog.case_id
        case_name = dialog.case_name
        case_folder = dialog.case_folder

        dialog.destroy()

        # if user hit OK, create a new case
        if response == Gtk.ResponseType.OK:
            case = mobius.framework.model.new_case(case_folder)
            case.root_item.id = case_id
            case.root_item.name = case_name

            window = self.__get_window_case()
            window.__set_case(case)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle file->open
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_file_open(self, widget, *args):

        # choose file
        fs = Gtk.FileChooserDialog(title='Choose case folder to open')
        fs.set_action(Gtk.FileChooserAction.SELECT_FOLDER)
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)

        rc = fs.run()
        folder = fs.get_filename()
        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        # check if case.sqlite exists
        if not os.path.exists(os.path.join(folder, 'case.sqlite')):
            dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_error)
            dialog.text = 'Could not find "case.sqlite" file inside folder'
            dialog.add_button(mobius.core.ui.message_dialog.button_ok)
            rc = dialog.run()
            return

        # open case
        case = mobius.framework.model.open_case(folder)
        window = self.__get_window_case()
        window.__set_case(case)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle file->close
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_file_close(self, widget, *args):
        root_item = self.__case.root_item
        case_name = root_item.name or root_item.id

        # show 'would you like to close case' dialog
        dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
        dialog.text = f"Do you want to close '{case_name}'?"
        dialog.add_button(mobius.core.ui.message_dialog.button_yes)
        dialog.add_button(mobius.core.ui.message_dialog.button_no)
        dialog.set_default_response(mobius.core.ui.message_dialog.button_no)
        rc = dialog.run()

        # if response = YES, set current version and show extension manager
        if rc != mobius.core.ui.message_dialog.button_yes:
            return True

        # close case
        mobius.framework.model.close_case(self.__case)

        # close working area
        if mobius.framework.model.get_case_count() > 0:
            self.__destroy()

        else:
            self.__set_case(None)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle file->quit. quit application
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_file_quit(self, widget, *args):

        # show confirmation dialog
        dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
        dialog.text = "Do you really want to quit from Mobius Forensic Toolkit?"
        dialog.add_button(mobius.core.ui.message_dialog.button_yes)
        dialog.add_button(mobius.core.ui.message_dialog.button_no)
        dialog.set_default_response(mobius.core.ui.message_dialog.button_no)
        rc = dialog.run()

        if rc != mobius.core.ui.message_dialog.button_yes:
            return True

        # stop UI
        self.__destroy()
        mobius.core.ui.stop()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle help->about
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_help_about(self, widget, *args):
        dialog = AboutDialog()
        dialog.run()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle resource-added event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_resource_added(self, report_id, resource):
        if report_id.startswith('menu.tools.'):
            icon_path, title, callback = resource.value
            item_id = report_id.rsplit('.', 1)[1]
            self.__add_tools_menu_item(item_id, title, icon_path, callback)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle resource-removed event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_resource_removed(self, report_id):
        if report_id.startswith('menu.tools.'):
            item_id = report_id.rsplit('.', 1)[1]
            self.__remove_tools_menu_item(item_id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle attribute-modified event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_attribute_modified(self, obj, attr_id, old_value, new_value):
        self.__on_attribute_changed(obj, attr_id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle attribute-removed event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_attribute_removed(self, obj, attr_id, old_value):
        self.__on_attribute_changed(obj, attr_id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle attribute changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_attribute_changed(self, obj, attr_id):
        if self.__case and obj.case and obj.case.uid == self.__case.uid:

            if attr_id == 'name' and obj.uid == 1:  # root item - change window title
                self.__update_window()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Add item to tools menus
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __add_tools_menu_item(self, item_id, title, icon_path, callback):
        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(3)
        hbox.set_visible(True)

        icon = mobius.core.ui.new_icon_by_path(icon_path, mobius.core.ui.icon.size_toolbar)
        icon.set_visible(True)
        hbox.add_child(icon, mobius.core.ui.box.fill_none)

        label = mobius.core.ui.label(title)
        label.set_halign(mobius.core.ui.label.align_left)
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_with_widget)

        item = Gtk.MenuItem.new()
        item.add(hbox.get_ui_widget())
        item.item_id = item_id
        item.title = title
        item.connect('activate', callback)
        item.show()

        # find item position
        pos = 0
        children = self.__tools_menu.get_children()

        while pos < len(children) and children[pos].title < title:
            pos += 1

        # insert item
        self.__tools_menu.insert(item, pos)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Remove item from tools menus
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __remove_tools_menu_item(self, item_id):
        for child in self.__tools_menu.get_children():
            if child.item_id == item_id:
                self.__tools_menu.remove(child)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Service <app.start> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_app_start():

    # create working area
    window = ICEWindow()
    window.show()

    # start graphical interface
    # mobius.core.ui.set_ui_implementation ("gtk3")

    print("app start *** 1 ***")
    try:
      mobius.core.ui.start()
    except Exception as e:
      print("app start *** 2 ***", e)
    print("app start *** 3 ***")


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief API initialization
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start_api():
    pymobius.mediator.advertise('app.start', svc_app_start)
