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
import os.path

import mobius
import mobius.core.ui
import pymobius.xml
from gi.repository import GdkPixbuf
from gi.repository import Gtk
from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Windows constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
METHOD_NEW_PART, METHOD_MERGE = range(2)
CATEGORY_ICON, CATEGORY_NAME, CATEGORY_OBJECT = range(3)
PART_ID, PART_OBJECT = range(2)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Extension variables
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
event_uid = -1


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Widget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Widget(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__widget = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        self.__widget.set_border_width(5)
        self.__widget.set_spacing(5)
        self.__widget.set_visible(True)
        self.__mediator = pymobius.mediator.copy()

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

        item = Gtk.MenuItem.new_with_mnemonic('_Export...')
        item.connect("activate", self.on_model_export)
        item.show()
        menu.append(item)

        item = Gtk.MenuItem.new_with_mnemonic('_Import...')
        item.connect("activate", self.on_model_import)
        item.show()
        menu.append(item)

        item = Gtk.SeparatorMenuItem.new()
        item.show()
        menu.append(item)

        item = Gtk.MenuItem.new_with_mnemonic('_Quit')
        item.connect("activate", self.__on_extension_quit)
        item.show()
        menu.append(item)

        item = Gtk.MenuItem.new_with_mnemonic('_Edit')
        item.show()
        menubar.append(item)

        menu = Gtk.Menu()
        menu.show()
        item.set_submenu(menu)

        self.__copy_menuitem = Gtk.MenuItem.new_with_mnemonic('_Copy')
        self.__copy_menuitem.connect("activate", self.__on_part_copy)
        self.__copy_menuitem.set_sensitive(False)
        self.__copy_menuitem.show()
        menu.append(self.__copy_menuitem)

        self.__paste_menuitem = Gtk.MenuItem.new_with_mnemonic('_Paste')
        self.__paste_menuitem.connect("activate", self.__on_part_paste)
        self.__paste_menuitem.set_sensitive(False)
        self.__paste_menuitem.show()
        menu.append(self.__paste_menuitem)

        # working area
        hpaned = Gtk.HPaned()
        hpaned.show()
        self.__widget.add_child(hpaned, mobius.core.ui.box.fill_with_widget)

        # part vbox
        part_vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        part_vbox.set_spacing(4)
        part_vbox.set_visible(True)
        hpaned.pack1(part_vbox.get_ui_widget(), False, True)

        # category combobox
        datastore = Gtk.ListStore.new([GdkPixbuf.Pixbuf, str, object])
        datastore.set_sort_column_id(CATEGORY_NAME, Gtk.SortType.ASCENDING)

        for category in mobius.framework.get_categories():
            image = mobius.core.ui.new_icon_from_data(category.icon_data, mobius.core.ui.icon.size_dnd)
            icon = image.get_ui_widget().get_pixbuf()
            datastore.append((icon, category.name, category))

        self.__category_combobox = Gtk.ComboBox.new_with_model(datastore)
        self.__category_combobox.connect('changed', self.__on_category_selection_changed)
        self.__category_combobox.show()

        renderer = Gtk.CellRendererPixbuf()
        self.__category_combobox.pack_start(renderer, False)
        self.__category_combobox.add_attribute(renderer, 'pixbuf', CATEGORY_ICON)

        renderer = Gtk.CellRendererText()
        self.__category_combobox.pack_start(renderer, True)
        self.__category_combobox.add_attribute(renderer, 'text', CATEGORY_NAME)

        part_vbox.add_child(self.__category_combobox, mobius.core.ui.box.fill_none)

        # part listview
        frame = Gtk.Frame()
        frame.show()
        part_vbox.add_child(frame, mobius.core.ui.box.fill_with_widget)

        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        frame.add(sw)

        datastore = Gtk.ListStore.new([str, object])
        datastore.set_sort_column_id(PART_ID, Gtk.SortType.ASCENDING)

        self.__part_listview = Gtk.TreeView.new_with_model(datastore)
        self.__part_listview.set_headers_visible(False)
        self.__part_listview.show()
        sw.add(self.__part_listview)

        selection = self.__part_listview.get_selection()
        selection.connect("changed", self.__on_part_selection_changed)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('attribute')
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', PART_ID)
        self.__part_listview.append_column(tvcolumn)

        # buttons
        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_visible(True)
        part_vbox.add_child(hbox, mobius.core.ui.box.fill_none)

        self.__remove_button = mobius.core.ui.button()
        self.__remove_button.set_icon_by_name('list-remove')
        self.__remove_button.set_text("_Remove")
        self.__remove_button.set_sensitive(False)
        self.__remove_button.set_callback('clicked', self.__on_part_remove)
        self.__remove_button.set_visible(True)
        hbox.add_child(self.__remove_button, mobius.core.ui.box.fill_none)

        # attribute listview
        self.__listview = self.__mediator.call('ui.new-widget', 'attribute-list')
        self.__listview.set_is_editable(True)
        self.__listview.set_sensitive(False)
        self.__listview.set_report_id('part.attributes')
        self.__listview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__listview.set_control(self, 'listview')
        self.__listview.show()
        hpaned.pack2(self.__listview.get_ui_widget(), True, True)

        # select last used category, if any
        last_category_selected = mobius.framework.get_config('part-catalogue.last-category')
        last_category_idx = -1

        model = self.__category_combobox.get_model()

        for idx, (icon, name, category) in enumerate(model):
            if category.id == last_category_selected:
                last_category_idx = idx

        if last_category_idx != -1:
            self.__category_combobox.set_active(last_category_idx)

        # extension data
        self.__category = None
        self.__part = None
        self.__copy_and_paste_data = {}

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Return widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_widget(self):
        return self.__widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle category selection changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_category_selection_changed(self, combobox, *args):
        model = combobox.get_model()
        treeiter = combobox.get_active_iter()
        category = None

        part_model = self.__part_listview.get_model()
        part_model.clear()

        if treeiter:
            category = model.get_value(treeiter, CATEGORY_OBJECT)

            for part in self.__mediator.call('part.get_by_category', category.id):
                part_model.append((part.id, part))

            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('part-catalogue.last-category', category.id)
            transaction.commit()

        self.__category = category

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle part selection changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_part_selection_changed(self, selection, *args):
        can_copy = False
        can_paste = False

        # clear attribute listview
        self.__listview.clear()

        # get selected part
        part = None
        model, treeiter = selection.get_selected()

        if treeiter:
            part = model.get_value(treeiter, PART_OBJECT)
            category = mobius.framework.get_category(part.category)

            for attr in category.get_attributes():
                if attr.id != 'part_id' and attr.is_editable:
                    attr_value = part.attributes.get(attr.id)
                    self.__listview.add_value(attr.id, attr.name, attr_value, attr.is_editable)

            can_copy = True

            if self.__copy_and_paste_data:
                can_paste = True

        # enable/disable widgets
        item_enabled = part is not None
        self.__listview.set_sensitive(item_enabled)
        self.__remove_button.set_sensitive(item_enabled)
        self.__copy_menuitem.set_sensitive(can_copy)
        self.__paste_menuitem.set_sensitive(can_paste)
        self.__part = part

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle menu->copy
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_part_copy(self, widget, *args):
        self.__copy_and_paste_data = {}

        for attr_id, attr_name, attr_value in self.__listview:
            if attr_value is not None and attr_value.strip() != '':
                self.__copy_and_paste_data[attr_id] = attr_value

        self.__paste_menuitem.set_sensitive(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle menu->paste
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_part_paste(self, widget, *args):
        if not self.__part:
            return

        for attr_id, attr_name, attr_value in self.__listview:
            attr_value = self.__copy_and_paste_data.get(attr_id)
            self.__listview.set_value(attr_id, attr_value)

            if attr_value is not None:
                self.__part.attributes[attr_id] = attr_value

        self.__mediator.call('part.set', self.__part)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle part->remove event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_part_remove(self):
        if not self.__part:
            return

        # show confirmation dialog
        dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
        dialog.text = f"You are about to remove part '{self.__part.id}'. Are you sure?"
        dialog.add_button(mobius.core.ui.message_dialog.button_yes)
        dialog.add_button(mobius.core.ui.message_dialog.button_no)
        dialog.set_default_response(mobius.core.ui.message_dialog.button_no)
        rc = dialog.run()

        if rc != mobius.core.ui.message_dialog.button_yes:
            return

        # remove part from model
        self.__mediator.call('part.remove', self.__part)

        # remove part from part_listview
        selection = self.__part_listview.get_selection()
        model, treeiter = selection.get_selected()
        has_next = model.remove(treeiter)

        if has_next:
            selection.select_iter(treeiter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add new part to treeview
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_new_part(self, part):

        # set category_combobox
        model = self.__category_combobox.get_model()
        treeiter = model.get_iter_first()
        found = False

        while treeiter and not found:
            category = model.get_value(treeiter, CATEGORY_OBJECT)

            if category.id == part.category:
                self.__category_combobox.set_active_iter(treeiter)
                found = True

            else:
                treeiter = model.iter_next(treeiter)

        # if part does not exist, add to part_listview
        model = self.__part_listview.get_model()
        treeiter = model.get_iter_first()
        found = False

        while treeiter and not found:
            part_id = model.get_value(treeiter, PART_ID)

            if part_id == part.id:
                found = True
            else:
                treeiter = model.iter_next(treeiter)

        if not found:
            treeiter = model.append((part.id, part))

        # select part
        treepath = model.get_path(treeiter)
        self.__part_listview.scroll_to_cell(treepath)

        selection = self.__part_listview.get_selection()
        selection.select_iter(treeiter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief stop widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_widget_stopped(self):
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle quit option
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_extension_quit(self, widget, *args):
        self.__mediator.call('ui.working-area.close', self.__widget.working_area.id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle model-export event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_model_export(self, widget, *args):

        # choose file
        fs = Gtk.FileChooserDialog(title='Export part catalogue', action=Gtk.FileChooserAction.SAVE)
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        fs.set_do_overwrite_confirmation(True)

        treefilter = Gtk.FileFilter()
        treefilter.set_name('Part catalogue')
        treefilter.add_pattern('*.xml')
        fs.set_filter(treefilter)

        rc = fs.run()
        path = fs.get_filename()
        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        # export catalogue
        root, ext = os.path.splitext(path)
        if ext != '.xml':
            path += '.xml'

        parts = self.__mediator.call('part.get_all')
        pickle = pymobius.xml.Pickle()
        pickle.save(path, parts)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle model-import event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_model_import(self, widget, *args):

        # build filechooser
        fs = Gtk.FileChooserDialog(title='Import catalogue')
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)

        treefilter = Gtk.FileFilter()
        treefilter.add_pattern('*.xml')
        fs.set_filter(treefilter)

        # run dialog
        rc = fs.run()
        path = fs.get_filename()
        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        # import catalogue
        pickle = pymobius.xml.Pickle()
        parts = pickle.load(path)
        self.__mediator.call('part.set_all', parts)

        # reselect category
        self.__on_category_selection_changed(self.__category_combobox)

        # show result
        dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_info)
        dialog.text = f'Parts processed: {len(parts):d}'
        dialog.add_button(mobius.core.ui.message_dialog.button_ok)
        rc = dialog.run()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle listview event: value edition
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def listview_after_editing_value(self, attr_id, old_value, new_value):

        if old_value != new_value:  # and self.__part:
            if new_value.strip() != '':
                self.__part.attributes[attr_id] = new_value

            else:
                self.__part.attributes.pop(attr_id, None)

            self.__mediator.call('part.set', self.__part)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Open working area
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def open_working_area():
    working_area = pymobius.mediator.call('ui.working-area.get', EXTENSION_ID)

    if not working_area:
        widget = Widget()
        icon_path = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)

        working_area = pymobius.mediator.call('ui.working-area.new', EXTENSION_ID)
        working_area.set_default_size(600, 400)
        working_area.set_title(EXTENSION_NAME)
        working_area.set_icon(icon_path)
        working_area.set_widget(widget)
        working_area.app_widget = widget

    working_area.show()

    return working_area


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    icon = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)
    mobius.core.add_resource('menu.tools.' + EXTENSION_ID, 'Menu Tool: Part Catalogue',
                             (icon, EXTENSION_NAME, on_activate))

    global event_uid
    event_uid = mobius.core.subscribe('attribute-modified', on_attribute_modified)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('menu.tools.' + EXTENSION_ID)
    pymobius.mediator.call('ui.working-area.del', EXTENSION_ID)

    global event_uid
    mobius.core.unsubscribe(event_uid)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief on_activate
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def on_activate(item_id):
    working_area = open_working_area()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Handle attribute-modified event
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def on_attribute_modified(item, attr_id, old_value, new_value):
    if attr_id != 'part_id' or new_value.strip() == '':
        return

    part = pymobius.mediator.call('part.get', item.category, new_value)

    # unknown part ID. Open up part catalogue dialog
    if not part:
        dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
        dialog.text = f"Would you like to include part '{new_value}'?"
        dialog.add_button(mobius.core.ui.message_dialog.button_yes)
        dialog.add_button(mobius.core.ui.message_dialog.button_no)
        rc = dialog.run()

        if rc == mobius.core.ui.message_dialog.button_yes:
            working_area = open_working_area()
            widget = working_area.app_widget
            part = pymobius.mediator.call('part.new', item.category, new_value.strip())
            widget.add_new_part(part)

    # otherwise, update item attributes
    else:
        for attr_id, value in part.attributes.items():
            item.set_attribute(attr_id, value)
        item.expand_masks()
