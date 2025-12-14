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
import json
import os
import os.path

import mobius
import mobius.core.decoder
import mobius.core.encoder
import mobius.core.io
import pymobius
from gi.repository import GLib
from gi.repository import Gdk
from gi.repository import GdkPixbuf
from gi.repository import Gtk

from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Windows constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
(CATEGORY_ICON, CATEGORY_NAME, CATEGORY_OBJ) = range(3)
(ATTR_ID, ATTR_NAME, ATTR_VALUE_MASK) = range(3)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief resize icon data
# @param icon_data icon data
# @return new icon data
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def resize_icon_data(icon_data):
    loader = GdkPixbuf.PixbufLoader()
    loader.set_size(64, 64)
    loader.write(icon_data)
    loader.close()

    pixbuf = loader.get_pixbuf()
    result, data = pixbuf.save_to_bufferv("png", [None], [])

    if result:
        return data

    return icon_data


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Import category database from .json file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def import_json(f):
    # read file
    fp = f.new_reader()
    categories = json.load(fp)

    # insert data
    transaction = mobius.framework.new_category_transaction()
    mobius.framework.remove_categories()

    for d_category in categories:
        category = mobius.framework.new_category(d_category.get('id'))
        category.name = d_category.get('name')
        category.description = d_category.get('description')
        category.icon_data = mobius.core.decoder.base64(d_category.get('icon'))

        # attributes
        for d_attr in d_category.get('attributes'):
            attr = category.new_attribute(d_attr.get('id'))
            attr.name = d_attr.get('name')
            attr.description = d_attr.get('description')
            attr.datatype = d_attr.get('datatype')
            attr.value_mask = d_attr.get('value_mask')

    transaction.commit()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Export category database as .json file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def export_json(f):
    # generate data
    categories = []

    for category in mobius.framework.get_categories():
        d_category = {
            'id': category.id,
            'name': category.name,
            'description': category.description,
            'icon': mobius.core.encoder.base64(category.icon_data)
        }

        attrlist = []
        for attr in category.get_attributes():
            d_attr = {
                'id': attr.id,
                'name': attr.name,
                'description': attr.description,
                'datatype': attr.datatype,
                'value_mask': attr.value_mask,
                'index': attr.index
            }
            attrlist.append(d_attr)

        d_category['attributes'] = attrlist
        categories.append(d_category)

    # write file
    fp = mobius.core.io.text_writer(f.new_writer())
    json.dump(categories, fp)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Widget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Widget(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__widget = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        self.__mediator = pymobius.mediator.copy()
        self.__widget.set_border_width(5)
        self.__widget.set_spacing(5)
        self.__widget.set_visible(True)

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
        item.connect("activate", self.on_extension_close)
        item.show()
        menu.append(item)

        # hpaned
        hpaned = Gtk.HPaned()
        hpaned.show()
        self.__widget.add_child(hpaned, mobius.core.ui.box.fill_with_widget)

        # category listview
        vbox1 = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbox1.set_spacing(5)
        vbox1.set_visible(True)
        hpaned.pack1(vbox1.get_ui_widget(), True, True)

        frame = Gtk.Frame()
        frame.show()
        vbox1.add_child(frame, mobius.core.ui.box.fill_with_widget)

        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        frame.add(sw)

        model = Gtk.ListStore.new([GdkPixbuf.Pixbuf, str, object])
        model.set_sort_column_id(CATEGORY_NAME, Gtk.SortType.ASCENDING)

        self.category_listview = Gtk.TreeView.new_with_model(model)
        self.category_listview.set_headers_visible(False)
        self.category_listview.show()
        sw.add(self.category_listview)

        selection = self.category_listview.get_selection()
        selection.connect('changed', self.on_category_selection_changed)

        renderer = Gtk.CellRendererPixbuf()
        tvcolumn = Gtk.TreeViewColumn()
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'pixbuf', CATEGORY_ICON)
        self.category_listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn()
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', CATEGORY_NAME)
        self.category_listview.append_column(tvcolumn)

        # buttons
        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_visible(True)
        vbox1.add_child(hbox, mobius.core.ui.box.fill_none)

        button = mobius.core.ui.button()
        button.set_icon_by_name('list-add')
        button.set_text("_Add")
        button.set_visible(True)
        button.set_callback('clicked', self.__on_category_add)
        hbox.add_child(button, mobius.core.ui.box.fill_none)

        hbox.add_filler()

        self.__remove_button = mobius.core.ui.button()
        self.__remove_button.set_icon_by_name('list-remove')
        self.__remove_button.set_text("_Remove")
        self.__remove_button.set_sensitive(False)
        self.__remove_button.set_visible(True)
        self.__remove_button.set_callback('clicked', self.__on_category_remove)
        hbox.add_child(self.__remove_button, mobius.core.ui.box.fill_none)

        # notebook
        notebook = Gtk.Notebook()
        notebook.show()
        hpaned.pack2(notebook, True, True)

        # page: general
        vbox2 = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbox2.set_border_width(5)
        vbox2.set_visible(True)
        notebook.append_page(vbox2.get_ui_widget(), mobius.core.ui.label('General').get_ui_widget())

        grid = Gtk.Grid.new()
        grid.set_row_spacing(10)
        grid.set_column_spacing(5)
        grid.show()
        vbox2.add_child(grid, mobius.core.ui.box.fill_with_widget)

        label = mobius.core.ui.label()
        label.set_markup('<b>ID</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 0, 1, 1)

        self.category_id_entry = Gtk.Entry()
        self.category_id_entry.set_editable(False)
        self.category_id_entry.set_sensitive(False)
        self.category_id_entry.set_hexpand(True)
        self.category_id_entry.show()
        grid.attach(self.category_id_entry, 1, 0, 2, 1)

        label = mobius.core.ui.label()
        label.set_markup('<b>Name</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 1, 1, 1)

        self.category_name_entry = Gtk.Entry()
        self.category_name_entry.connect('changed', self.on_category_name_changed)
        self.category_name_entry.show()
        grid.attach(self.category_name_entry, 1, 1, 2, 1)

        label = mobius.core.ui.label()
        label.set_markup('<b>Icon</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 2, 1, 1)

        icon = mobius.core.ui.new_icon_by_name('image-missing', mobius.core.ui.icon.size_dnd)
        self.__category_icon_button = mobius.core.ui.button()
        self.__category_icon_button.set_icon(icon)
        self.__category_icon_button.set_visible(True)
        self.__category_icon_button.set_callback('clicked', self.__on_category_icon_edit)

        grid.attach(self.__category_icon_button.get_ui_widget(), 1, 2, 1, 1)

        placeholder = mobius.core.ui.label(' ')
        placeholder.set_visible(True)
        grid.attach(placeholder.get_ui_widget(), 2, 0, 1, 1)

        # page: attributes
        vbox_page = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbox_page.set_spacing(5)
        vbox_page.set_visible(True)
        notebook.append_page(vbox_page.get_ui_widget(), mobius.core.ui.label('Attributes').get_ui_widget())

        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        vbox_page.add_child(sw, mobius.core.ui.box.fill_with_widget)

        # attributes listview
        datastore = Gtk.ListStore.new([str, str, str])
        self.attribute_listview = Gtk.TreeView.new_with_model(datastore)
        self.attribute_listview.set_enable_search(False)

        selection = self.attribute_listview.get_selection()
        selection.connect('changed', self.on_attribute_selection_changed)

        renderer = Gtk.CellRendererText()
        renderer.set_property('editable', True)
        renderer.connect('edited', self.on_attribute_id_edited)

        tvcolumn = Gtk.TreeViewColumn('ID')
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', ATTR_ID)
        tvcolumn.set_resizable(True)
        self.attribute_listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        renderer.set_property('editable', True)
        renderer.connect('edited', self.on_attribute_name_edited)

        tvcolumn = Gtk.TreeViewColumn('Name')
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', ATTR_NAME)
        self.attribute_listview.append_column(tvcolumn)
        self.attribute_listview.show()
        sw.add(self.attribute_listview)

        # value_mask
        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(5)
        hbox.set_visible(True)
        vbox_page.add_child(hbox, mobius.core.ui.box.fill_none)

        label = mobius.core.ui.label()
        label.set_markup('<b>Value mask</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        self.value_mask_entry = Gtk.Entry()
        self.value_mask_entry.connect('changed', self.on_value_mask_changed)
        self.value_mask_entry.show()
        hbox.add_child(self.value_mask_entry, mobius.core.ui.box.fill_with_widget)

        # attribute buttons
        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_visible(True)
        vbox_page.add_child(hbox, mobius.core.ui.box.fill_none)

        self.__add_attr_button = mobius.core.ui.button()
        self.__add_attr_button.set_icon_by_name('list-add')
        self.__add_attr_button.set_visible(True)
        self.__add_attr_button.set_callback('clicked', self.__on_attribute_add)
        hbox.add_child(self.__add_attr_button, mobius.core.ui.box.fill_none)

        self.__remove_attr_button = mobius.core.ui.button()
        self.__remove_attr_button.set_icon_by_name('list-remove')
        self.__remove_attr_button.set_sensitive(False)
        self.__remove_attr_button.set_visible(True)
        self.__remove_attr_button.set_callback('clicked', self.__on_attribute_remove)
        hbox.add_child(self.__remove_attr_button, mobius.core.ui.box.fill_none)

        self.__up_attr_button = mobius.core.ui.button()
        self.__up_attr_button.set_icon_by_name('go-up')
        self.__up_attr_button.set_sensitive(False)
        self.__up_attr_button.set_visible(True)
        self.__up_attr_button.set_callback('clicked', self.__on_attribute_up)
        hbox.add_child(self.__up_attr_button, mobius.core.ui.box.fill_none)

        self.__down_attr_button = mobius.core.ui.button()
        self.__down_attr_button.set_icon_by_name('go-down')
        self.__down_attr_button.set_sensitive(False)
        self.__down_attr_button.set_visible(True)
        self.__down_attr_button.set_callback('clicked', self.__on_attribute_down)
        hbox.add_child(self.__down_attr_button, mobius.core.ui.box.fill_none)

        # flags
        self.is_selecting = False  # selecting new category
        self.is_setting_value_mask = False  # setting value_mask text
        self.category = None  # current selected category

        self.populate_category_listview()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get internal widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_widget(self):
        return self.__widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate category listview
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def populate_category_listview(self):
        model = self.category_listview.get_model()
        model.clear()

        for category in mobius.framework.get_categories():
            icon = self.__render_icon(category)
            model.append((icon, category.name, category))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate attribute listview
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def populate_attribute_listview(self, category):
        model = self.attribute_listview.get_model()
        model.clear()

        for a in category.get_attributes():
            if a.name and a.name[0].islower():
                name = a.name.capitalize()
            else:
                name = a.name
            model.append((a.id, name, a.value_mask))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle close button
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_extension_close(self, widget, *args):
        self.__mediator.call('ui.working-area.close', self.working_area.id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief show save/ignore/cancel dialog if there are modified items
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_widget_stopped(self):  # test
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle model-export event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_model_export(self, widget, *args):

        # choose file
        fs = Gtk.FileChooserDialog(title='Export category database', action=Gtk.FileChooserAction.SAVE)
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        fs.set_do_overwrite_confirmation(True)
        fs.set_current_name('category.json')

        fc_filter = Gtk.FileFilter()
        fc_filter.set_name('Category Manager')
        fc_filter.add_pattern('*.json')
        fs.set_filter(fc_filter)

        rc = fs.run()
        uri = fs.get_uri()
        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        root, ext = os.path.splitext(uri)
        if ext != '.json':
            uri += '.json'

        # export database
        f = mobius.core.io.new_file_by_url(uri)
        export_json(f)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle model-import event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_model_import(self, widget, *args):

        # build filechooser
        fs = Gtk.FileChooserDialog(title='Import category database')
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)

        fc_filter = Gtk.FileFilter()
        fc_filter.add_pattern('*.json')
        fs.set_filter(fc_filter)

        # run dialog
        rc = fs.run()
        uri = fs.get_uri()
        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        # import database
        f = mobius.core.io.new_file_by_url(uri)
        import_json(f)
        self.populate_category_listview()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle category selection
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_category_selection_changed(self, selection, *args):
        model, treeiter = selection.get_selected()
        self.is_selecting = True

        # fill info
        if treeiter:
            category = model.get_value(treeiter, CATEGORY_OBJ)

            self.category_id_entry.set_text(category.id)
            self.category_name_entry.set_text(category.name or '')

            icon = mobius.core.ui.new_icon_from_data(category.icon_data, mobius.core.ui.icon.size_dnd)
            self.__category_icon_button.set_icon(icon)

            self.populate_attribute_listview(category)
            self.category = category

        else:
            self.category_id_entry.set_text('')
            self.category_name_entry.set_text('')
            icon = mobius.core.ui.new_icon_by_name('image-missing', mobius.core.ui.icon.size_dnd)
            self.__category_icon_button.set_icon(icon)

            attr_model = self.attribute_listview.get_model()
            attr_model.clear()
            self.category = None

        self.is_selecting = False

        # enable/disable widgets
        enabled = treeiter is not None
        self.__remove_button.set_sensitive(enabled)
        self.category_name_entry.set_sensitive(enabled)
        self.__category_icon_button.set_sensitive(enabled)
        self.__add_attr_button.set_sensitive(enabled)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle category add
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_category_add(self):

        # show dialog to enter category ID and Name
        dialog = Gtk.Dialog(title='Creating category', modal=True)
        dialog.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        dialog.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        dialog.set_position(Gtk.WindowPosition.CENTER)
        dialog.set_type_hint(Gdk.WindowTypeHint.DIALOG)
        dialog.set_border_width(10)

        grid = Gtk.Grid.new()
        grid.set_row_spacing(5)
        grid.set_column_spacing(5)
        grid.show()
        dialog.vbox.pack_start(grid, True, True, 0)

        label = mobius.core.ui.label('ID')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label, 0, 0, 1, 1)

        dialog.id_entry = Gtk.Entry()
        dialog.id_entry.show()
        grid.attach(dialog.id_entry, 1, 0, 1, 1)

        label = mobius.core.ui.label('Name')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label, 0, 1, 1, 1)

        dialog.name_entry = Gtk.Entry()
        dialog.name_entry.show()
        grid.attach(dialog.name_entry, 1, 1, 1, 1)

        rc = dialog.run()
        cat_id = dialog.id_entry.get_text()
        cat_name = dialog.name_entry.get_text()
        dialog.destroy()

        # if user pressed OK, create category
        if rc == Gtk.ResponseType.OK and cat_id:
            # create category
            category = mobius.framework.new_category(cat_id)
            category.name = cat_name

            # update window
            icon = self.__render_icon(category)
            model = self.category_listview.get_model()
            treeiter = model.append((icon, category.name, category))

            selection = self.category_listview.get_selection()
            selection.select_iter(treeiter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle category remove
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_category_remove(self):
        selection = self.category_listview.get_selection()
        model, treeiter = selection.get_selected()

        if treeiter:
            category = model.get_value(treeiter, CATEGORY_OBJ)

            # show confirmation dialog
            dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
            dialog.text = f"You are about to remove category '{category.name}'. Are you sure?"
            dialog.add_button(mobius.core.ui.message_dialog.button_yes)
            dialog.add_button(mobius.core.ui.message_dialog.button_no)
            dialog.set_default_response(mobius.core.ui.message_dialog.button_no)
            rc = dialog.run()

            if rc != mobius.core.ui.message_dialog.button_yes:
                return

            # remove category
            mobius.framework.remove_category(category.id)

            # update window
            has_next = model.remove(treeiter)

            if has_next:
                selection.select_iter(treeiter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle category name edition
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_category_name_changed(self, widget, *args):
        model, treeiter = self.category_listview.get_selection().get_selected()

        if not self.is_selecting and treeiter:
            value = self.category_name_entry.get_text()

            category = model.get_value(treeiter, CATEGORY_OBJ)
            category.name = value
            model.set_value(treeiter, CATEGORY_NAME, value)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle category icon edition
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_category_icon_edit(self):

        # choose file
        fs = Gtk.FileChooserDialog(title='Choose 64x64 icon filename')
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)

        fc_filter = Gtk.FileFilter()
        fc_filter.add_pattern('*.png')
        fc_filter.add_pattern('*.jpg')
        fc_filter.add_pattern('*.gif')
        fc_filter.add_pattern('*.svg')
        fs.set_filter(fc_filter)

        rc = fs.run()
        filename = fs.get_filename()
        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        # read file
        fp = open(filename, 'rb')
        data = fp.read()
        fp.close()

        # set new icon
        model, treeiter = self.category_listview.get_selection().get_selected()

        if treeiter:
            category = model.get_value(treeiter, CATEGORY_OBJ)
            category.icon_data = resize_icon_data(data)
            icon = mobius.core.ui.new_icon_from_data(category.icon_data, mobius.core.ui.icon.size_dnd)
            self.__category_icon_button.set_icon(icon)
            model.set_value(treeiter, CATEGORY_ICON, icon)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief render category icon
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __render_icon(self, category):
        icon = None

        if category.icon_data:
            image = mobius.core.ui.new_icon_from_data(category.icon_data, mobius.core.ui.icon.size_dnd)
            icon = image.get_ui_widget().get_pixbuf()

        return icon

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle attribute selection
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_attribute_selection_changed(self, selection, *args):
        model, treeiter = selection.get_selected()

        # get value mask
        if treeiter:
            value_mask = model.get_value(treeiter, ATTR_VALUE_MASK) or ''
            row = model.get_path(treeiter)[0]
        else:
            value_mask = ''
            row = 0

        # set entry text
        self.is_setting_value_mask = True
        self.value_mask_entry.set_text(value_mask)
        self.is_setting_value_mask = False

        # enable/disable widgets
        enabled = treeiter is not None
        up_enabled = enabled and row > 0
        down_enabled = enabled and model.iter_next(treeiter) is not None

        self.value_mask_entry.set_sensitive(enabled)
        self.__remove_attr_button.set_sensitive(enabled)
        self.__up_attr_button.set_sensitive(up_enabled)
        self.__down_attr_button.set_sensitive(down_enabled)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle attribute add
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_attribute_add(self):
        model = self.attribute_listview.get_model()
        treeiter = model.append(('<ID>', '<NAME>', ''))
        path = model.get_path(treeiter)
        column = self.attribute_listview.get_column(ATTR_ID)
        self.attribute_listview.set_cursor(path, column, True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle attribute remotion
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_attribute_remove(self):
        selection = self.attribute_listview.get_selection()
        model, treeiter = selection.get_selected()

        if treeiter:
            # remove attribute
            attr_id = model.get_value(treeiter, ATTR_ID)
            self.category.remove_attribute(attr_id)

            # update window
            has_next = model.remove(treeiter)

            if has_next:
                selection.select_iter(treeiter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle move up
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_attribute_up(self):
        model, treeiter = self.attribute_listview.get_selection().get_selected()

        if treeiter:
            # update attribute
            dest_row = model.get_path(treeiter)[0] - 1
            attr_id = model.get_value(treeiter, ATTR_ID)
            attr = self.category.get_attribute(attr_id)
            attr.index = dest_row + 1

            # update window
            dest_iter = model.get_iter(dest_row)
            model.move_before(treeiter, dest_iter)
            self.attribute_listview.scroll_to_cell(dest_row)

            if dest_row == 0:
                self.__up_attr_button.set_sensitive(False)

            self.__down_attr_button.set_sensitive(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle move down
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_attribute_down(self):
        model, treeiter = self.attribute_listview.get_selection().get_selected()

        if treeiter:
            # update attribute
            dest_row = model.get_path(treeiter)[0] + 1
            attr_id = model.get_value(treeiter, ATTR_ID)
            attr = self.category.get_attribute(attr_id)
            attr.index = dest_row + 1

            # update window
            dest_iter = model.get_iter(dest_row)
            model.move_after(treeiter, dest_iter)
            self.attribute_listview.scroll_to_cell(dest_row)

            if dest_row + 1 >= len(model):
                self.__down_attr_button.set_sensitive(False)

            self.__up_attr_button.set_sensitive(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle attribute ID edition
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_attribute_id_edited(self, cell, path, new_text, *args):
        model = self.attribute_listview.get_model()
        treeiter = model.get_iter_from_string(path)
        text = model.get_value(treeiter, ATTR_ID)

        if text == '<ID>' and new_text.strip() != '' and new_text != '<ID>':
            model.set_value(treeiter, ATTR_ID, new_text)
            attr = self.category.new_attribute(new_text)

        elif text == '<ID>':
            dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_error)
            dialog.text = 'Attribute ID is mandatory'
            dialog.add_button(mobius.core.ui.message_dialog.button_ok)
            rc = dialog.run()

            column = self.attribute_listview.get_column(ATTR_ID)
            GLib.idle_add(self.attribute_listview.set_cursor, path, column, True)

        elif text != new_text:
            dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_error)
            dialog.text = 'Cannot changed attribute ID'
            dialog.add_button(mobius.core.ui.message_dialog.button_ok)
            rc = dialog.run()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle attribute name edition
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_attribute_name_edited(self, cell, path, new_text, *args):
        model = self.attribute_listview.get_model()
        treeiter = model.get_iter_from_string(path)
        text = model.get_value(treeiter, ATTR_NAME)

        if text != new_text:
            attr_id = model.get_value(treeiter, ATTR_ID)
            model.set_value(treeiter, ATTR_NAME, new_text)
            attr = self.category.get_attribute(attr_id)
            attr.name = new_text

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle value_mask edition
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_value_mask_changed(self, widget, *args):
        selection = self.attribute_listview.get_selection()
        model, treeiter = selection.get_selected()

        if not self.is_setting_value_mask and treeiter:
            attr_id = model.get_value(treeiter, ATTR_ID)
            value = self.value_mask_entry.get_text()
            model.set_value(treeiter, ATTR_VALUE_MASK, value)

            attr = self.category.get_attribute(attr_id)
            attr.value_mask = value


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    icon = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)
    mobius.core.add_resource('menu.tools.' + EXTENSION_ID, 'Menu Tool: Category Manager',
                             (icon, EXTENSION_NAME, on_activate))

    # fill database, if necessary
    app = mobius.core.application()

    if not os.path.exists(app.get_config_path("category.sqlite")):
        path = app.get_data_path('data/category.json')
        f = mobius.core.io.new_file_by_path(path)
        import_json(f)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('menu.tools.' + EXTENSION_ID)
    pymobius.mediator.call('ui.working-area.del', EXTENSION_ID)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief on_activate
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def on_activate(item_id):
    working_area = pymobius.mediator.call('ui.working-area.get', EXTENSION_ID)

    if not working_area:
        widget = Widget()
        icon_path = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)

        working_area = pymobius.mediator.call('ui.working-area.new', EXTENSION_ID)
        working_area.set_default_size(600, 400)
        working_area.set_title(EXTENSION_NAME)
        working_area.set_icon(icon_path)
        working_area.set_widget(widget)

    working_area.show()
