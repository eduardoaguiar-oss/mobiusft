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
import mobius.framework
from gi.repository import Gdk
from gi.repository import GdkPixbuf
from gi.repository import Gtk

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
(ADD_ITEM_COMBO_ICON, ADD_ITEM_COMBO_NAME, ADD_ITEM_COMBO_OBJECT) = range(3)
(ADD_ITEM_ATTR_ID, ADD_ITEM_ATTR_NAME, ADD_ITEM_ATTR_VALUE) = range(3)
(CATEGORY_ICON, CATEGORY_NAME, CATEGORY_OBJECT) = range(3)


class AddItemDialog():

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Build widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__widget = Gtk.Dialog(title='Add item', modal=True)
        self.__widget.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        self.__widget.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)

        self.__category = None
        self.__mediator = pymobius.mediator.copy()

        # configure dialog
        self.__widget.set_position(Gtk.WindowPosition.CENTER)
        self.__widget.set_default_size(500, 400)
        self.__widget.set_type_hint(Gdk.WindowTypeHint.DIALOG)
        self.__widget.set_border_width(10)
        self.__widget.vbox.set_spacing(5)

        # datastore
        datastore = Gtk.ListStore.new([GdkPixbuf.Pixbuf, str, object])
        datastore.set_sort_column_id(ADD_ITEM_COMBO_NAME, Gtk.SortType.ASCENDING)

        for category in mobius.framework.get_categories():
            image = mobius.core.ui.new_icon_from_data(category.icon_data, mobius.core.ui.icon.size_dnd)
            image.show()
            icon = image.get_ui_widget().get_pixbuf()
            datastore.append((icon, category.name, category))

        # combobox
        self.combobox = Gtk.ComboBox.new_with_model(datastore)
        self.combobox.connect('changed', self.on_category_selected)
        self.combobox.show()

        renderer = Gtk.CellRendererPixbuf()
        self.combobox.pack_start(renderer, False)
        self.combobox.add_attribute(renderer, 'pixbuf', ADD_ITEM_COMBO_ICON)

        renderer = Gtk.CellRendererText()
        self.combobox.pack_start(renderer, True)
        self.combobox.add_attribute(renderer, 'text', ADD_ITEM_COMBO_NAME)
        self.__widget.vbox.pack_start(self.combobox, False, True, 0)

        # spinbutton
        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(5)
        hbox.set_visible(True)
        self.__widget.vbox.pack_start(hbox.get_ui_widget(), False, True, 0)

        hbox.add_filler()

        label = mobius.core.ui.label('Amount')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        adj = Gtk.Adjustment.new(1, 1, 9999, 1, 1, 0)
        self.spin = Gtk.SpinButton.new(adj, 0, 0)
        self.spin.show()
        hbox.add_child(self.spin, mobius.core.ui.box.fill_none)

        # attribute listview
        frame = Gtk.Frame()
        frame.show()
        self.__widget.vbox.pack_start(frame, True, True, 0)

        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        frame.add(sw)

        datastore = Gtk.ListStore.new([str, str, str])
        self.attribute_listview = Gtk.TreeView.new_with_model(datastore)
        self.attribute_listview.show()
        sw.add(self.attribute_listview)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('attribute')
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', ADD_ITEM_ATTR_NAME)
        self.attribute_listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        renderer.set_property('editable', True)
        renderer.connect('edited', self.on_attribute_edited)

        tvcolumn = Gtk.TreeViewColumn('value')
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', ADD_ITEM_ATTR_VALUE)
        self.attribute_listview.append_column(tvcolumn)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Select category
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_category(self, cat_id):
        model = self.combobox.get_model()
        treeiter = model.get_iter_first()

        while treeiter:
            category = model.get_value(treeiter, ADD_ITEM_COMBO_OBJECT)

            if category.id == cat_id:
                self.combobox.set_active_iter(treeiter)

            treeiter = model.iter_next(treeiter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Return amount of items
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_amount(self):
        return int(self.spin.get_value())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Return select category
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_category(self):
        return self.__category

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Return attribute values
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_attribute_values(self):
        model = self.attribute_listview.get_model()
        values = []

        for attr_id, attr_name, attr_value in model:
            if attr_value.strip() != '':
                values.append((attr_id, attr_value))

        return values

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle category selection
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_category_selected(self, combobox, *args):
        model = combobox.get_model()
        treeiter = combobox.get_active_iter()

        attribute_model = self.attribute_listview.get_model()
        attribute_model.clear()

        if treeiter:
            self.__category = model.get_value(treeiter, ADD_ITEM_COMBO_OBJECT)

            for attr in self.__category.get_attributes():
                if attr.is_editable:
                    attribute_model.append((attr.id, attr.name, ''))
        else:
            self.__category = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle value edition
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_attribute_edited(self, cell, path_string, new_text, *args):
        model = self.attribute_listview.get_model()
        treeiter = model.get_iter_from_string(path_string)
        model.set_value(treeiter, ADD_ITEM_ATTR_VALUE, new_text)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run Dialog
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        return self.__widget.run()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Destroy Dialog
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def destroy(self):
        self.__widget.destroy()
