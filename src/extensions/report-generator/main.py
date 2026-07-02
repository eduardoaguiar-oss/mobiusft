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
import datetime
import os.path

import mobius
import pymobius
from gi.repository import GLib
from gi.repository import Gtk

from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Report Generator view
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ReportGeneratorView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__template_id = None
        self.__output_folder = None
        self.__itemlist = []

        self.name = f'{EXTENSION_NAME} v{EXTENSION_VERSION}'
        icon_path = self.__mediator.call('extension.get-icon-path', EXTENSION_ID)
        self.icon_data = open(icon_path, 'rb').read()

        # widget
        self.__widget = mobius.core.ui.container()
        self.__widget.show()

        vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbox.set_border_width(5)
        vbox.set_spacing(10)
        vbox.set_visible(True)
        self.__widget.set_content(vbox)

        grid = Gtk.Grid.new()
        grid.set_row_spacing(10)
        grid.set_column_spacing(5)
        grid.show()
        vbox.add_child(grid, mobius.core.ui.box.fill_with_widget)

        label = mobius.core.ui.label()
        label.set_markup('<b>Template:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 0, 1, 1)

        # Template combobox
        self.__template_model = Gtk.ListStore.new([str, str, object])

        for r in mobius.core.get_resources('report-template'):
            generator = r.value()

            for template in generator.templates:
                t_id = template['id']
                t_type = template['type']   # @todo select icon according to type
                t_description = template['description']
                self.__template_model.append([t_id, t_description, generator])

        self.__template_combobox = Gtk.ComboBox.new_with_model(self.__template_model)
        self.__template_combobox.set_hexpand(True)

        renderer = Gtk.CellRendererText()
        self.__template_combobox.pack_start(renderer, True)
        self.__template_combobox.add_attribute(renderer, 'text', 1)
        self.__template_combobox.set_id_column(0)
        self.__template_combobox.connect('changed', self.__on_template_changed)
        self.__template_combobox.show()
        grid.attach(self.__template_combobox, 1, 0, 2, 1)

        # Output directory
        label = mobius.core.ui.label()
        label.set_markup('<b>Output folder:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 1, 1, 2)

        # Output folder chooser button
        self.__output_folder_button = mobius.core.ui.button()
        self.__output_folder_button.set_icon_by_name('folder')
        self.__output_folder_button.set_text('Select output folder...')
        self.__output_folder_button.set_visible(True)
        self.__output_folder_button.set_callback('clicked', self.__on_click_output_folder)
        grid.attach(self.__output_folder_button.get_ui_widget(), 1, 1, 2, 2)

        # Buttons
        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(5)
        hbox.set_visible(True)
        hbox.add_filler ()
        vbox.add_child(hbox, mobius.core.ui.box.fill_none)

        self.__generate_button = mobius.core.ui.button()
        self.__generate_button.set_icon_by_name('system-run')
        self.__generate_button.set_text("_Execute")
        self.__generate_button.set_visible(True)
        self.__generate_button.set_sensitive(False)
        self.__generate_button.set_callback('clicked', self.__on_generate_report)
        hbox.add_child(self.__generate_button, mobius.core.ui.box.fill_none)

        # Set panel state
        last_template = mobius.framework.get_config('report-generator.last_template')
        if last_template:
            self.__template_combobox.set_active_id(last_template)

        last_output_dir = mobius.framework.get_config('report-generator.last_output_dir')
        if last_output_dir:
            self.__output_folder_button.set_text(last_output_dir)
            self.__output_folder = last_output_dir

        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set status text
    # @param text Text
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_status(self, text):
        if text:
            t = str(datetime.datetime.now())[:19]
            text = t + ' ' + text

        GLib.idle_add(self.__status_label.set_text, text or '')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
       self.__itemlist = itemlist
       self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_destroy view
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update panel options
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __update_options(self):

        if self.__itemlist:
            self.__widget.show_content()
        else:
            self.__widget.set_message('Select item(s) to generate report')

        can_generate = bool(self.__template_id) and bool(self.__output_folder) and bool(self.__itemlist)
        self.__generate_button.set_sensitive(can_generate)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_template_changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_template_changed(self, combobox, *args):
        self.__template_id = combobox.get_active_id()

        if self.__template_id:
            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('report-generator.last_template', self.__template_id)
            transaction.commit()

            self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_output_folder button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_output_folder(self):
        dialog = Gtk.FileChooserDialog(title='Select output folder', action=Gtk.FileChooserAction.SELECT_FOLDER)
        dialog.add_buttons(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OPEN, Gtk.ResponseType.OK)

        if self.__output_folder:
            dialog.set_current_folder(self.__output_folder)

        response = dialog.run()

        if response == Gtk.ResponseType.OK:
            self.__output_folder = dialog.get_filename()
            self.__output_folder_button.set_text(self.__output_folder)

            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('report-generator.last_output_dir', self.__output_folder)
            transaction.commit()

        dialog.destroy()
        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_generate_report button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_generate_report(self):
        treemodel = self.__template_combobox.get_model()
        treeiter = self.__template_combobox.get_active_iter()
        generator = treemodel[treeiter][2]

        model = {
            'template_id': self.__template_id,
            'output_dir': self.__output_folder,
            'items': self.__itemlist
        }

        generator.run(model)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    mobius.core.add_resource('view.report-generator', 'Report Generator view', ReportGeneratorView)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('view.report-generator')
