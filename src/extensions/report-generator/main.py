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
import json

import mobius
import pymobius
from pymobius import app
import pymobius.evidence
from gi.repository import GLib
from gi.repository import Gtk
from gi.repository import GdkPixbuf

from metadata import *

TEMPLATE_ICON, TEMPLATE_ID, TEMPLATE_NAME, GENERATOR_OBJ = range(4)

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
        self.__asap_path = None
        self.__itemlist = []

        self.name = f'{EXTENSION_NAME} v{EXTENSION_VERSION}'
        icon_path = self.__mediator.call('extension.get-icon-path', EXTENSION_ID)
        self.icon_data = open(icon_path, 'rb').read()

        # Widget
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
        grid.set_column_homogeneous(False)
        grid.show()
        vbox.add_child(grid, mobius.core.ui.box.fill_with_widget)

        label = mobius.core.ui.label()
        label.set_markup('<b>Template:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 0, 1, 1)

        # Template combobox model
        self.__template_model = Gtk.ListStore.new([GdkPixbuf.Pixbuf, str, str, object])

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'media.png')
        media_icon = GdkPixbuf.Pixbuf.new_from_file_at_size(path, 24, 24)

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'report.png')
        report_icon = GdkPixbuf.Pixbuf.new_from_file_at_size(path, 24, 24)

        for r in mobius.core.get_resources('report-template'):
            generator = r.value()

            for template in generator.templates:
                t_id = template['id']
                t_type = template['type']
                t_icon = media_icon if t_type == 'media' else report_icon
                t_description = template['description']
                self.__template_model.append([t_icon, t_id, t_description, generator])

        # Template combobox widget
        self.__template_combobox = Gtk.ComboBox.new_with_model(self.__template_model)
        self.__template_combobox.set_hexpand(True)

        renderer = Gtk.CellRendererPixbuf()
        self.__template_combobox.pack_start(renderer, False)
        self.__template_combobox.add_attribute(renderer, 'pixbuf', TEMPLATE_ICON)

        renderer = Gtk.CellRendererText()
        self.__template_combobox.pack_start(renderer, True)
        self.__template_combobox.add_attribute(renderer, 'text', TEMPLATE_NAME)
        self.__template_combobox.set_id_column(TEMPLATE_ID)
        self.__template_combobox.connect('changed', self.__on_template_changed)
        self.__template_combobox.show()
        grid.attach(self.__template_combobox, 1, 0, 2, 1)

        # Output folder
        label = mobius.core.ui.label()
        label.set_markup('<b>Output folder:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 1, 1, 1)

        # Output folder chooser button
        self.__output_folder_button = mobius.core.ui.button()
        self.__output_folder_button.set_icon_by_name('folder')
        self.__output_folder_button.set_text('Select output folder...')
        self.__output_folder_button.set_visible(True)
        self.__output_folder_button.set_callback('clicked', self.__on_click_output_folder)
        grid.attach(self.__output_folder_button.get_ui_widget(), 1, 1, 2, 1)

        # .ASAP file path
        label = mobius.core.ui.label()
        label.set_markup('<b>.ASAP file (optional):</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 2, 1, 1)

        asap_hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        asap_hbox.set_spacing(5)
        asap_hbox.set_visible(True)
        grid.attach(asap_hbox.get_ui_widget(), 1, 2, 2, 1)

        # .ASAP file chooser button
        self.__asap_file_button = mobius.core.ui.button()
        self.__asap_file_button.set_icon_by_name('folder')
        self.__asap_file_button.set_text('Select a .ASAP file from the Federal Police of Brazil...')
        self.__asap_file_button.set_visible(True)
        self.__asap_file_button.set_callback('clicked', self.__on_click_asap_file)
        asap_hbox.add_child(self.__asap_file_button, mobius.core.ui.box.fill_with_widget)

        # .ASAP clear button
        self.__asap_clear_button = mobius.core.ui.button()
        self.__asap_clear_button.set_icon_by_name('edit-clear')
        self.__asap_clear_button.set_visible(True)
        self.__asap_clear_button.set_sensitive(False)
        self.__asap_clear_button.set_callback('clicked', self.__on_click_asap_clear_file)
        asap_hbox.add_child(self.__asap_clear_button, mobius.core.ui.box.fill_none)

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
        last_template = mobius.framework.get_config('last_report_template')
        if last_template:
            self.__template_combobox.set_active_id(last_template)

        last_output_dir = mobius.framework.get_config('last_report_dir')
        if last_output_dir:
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

        if self.__output_folder:
            self.__output_folder_button.set_text(self.__output_folder)
        else:
            self.__output_folder_button.set_text('Select output folder...')

        if self.__asap_path:
            self.__asap_file_button.set_text(self.__asap_path)
            self.__asap_clear_button.set_sensitive(True)
        else:
            self.__asap_file_button.set_text('Select a .ASAP file from the Federal Police of Brazil...')
            self.__asap_clear_button.set_sensitive(False)

        can_generate = bool(self.__template_id) and bool(self.__output_folder) and bool(self.__itemlist)
        self.__generate_button.set_sensitive(can_generate)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_template_changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_template_changed(self, combobox, *args):
        self.__template_id = combobox.get_active_id()

        if self.__template_id:
            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('last_report_template', self.__template_id)
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
            mobius.framework.set_config('last_report_dir', self.__output_folder)
            transaction.commit()

        dialog.destroy()
        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_asap_file button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_asap_file(self):
        dialog = Gtk.FileChooserDialog(title='Select .ASAP file', action=Gtk.FileChooserAction.OPEN)
        dialog.add_buttons(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OPEN, Gtk.ResponseType.OK)

        if self.__asap_path:
            dialog.set_current_folder(self.__asap_path)

        response = dialog.run()

        if response == Gtk.ResponseType.OK:
            self.__asap_path = dialog.get_filename()
            self.__asap_file_button.set_text(self.__asap_path)

            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('last_asap_file', self.__asap_path)
            transaction.commit()

        dialog.destroy()
        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_asap_clear_file button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_asap_clear_file(self):
        self.__asap_path = None
        self.__asap_file_button.set_text('Select a .ASAP file from the Federal Police of Brazil...')
        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_generate_report button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_generate_report(self):
        treemodel = self.__template_combobox.get_model()
        treeiter = self.__template_combobox.get_active_iter()
        generator = treemodel[treeiter][GENERATOR_OBJ]

        # Build model
        model = {
            'template_id': self.__template_id,
            'output_dir': self.__output_folder,
            'items': self.__itemlist,
            'evidence_types': self.__get_evidence_types(),
        }

        # Add .ASAP data if available
        if self.__asap_path:
            model['asap'] = self.__parse_asap_file(self.__asap_path)

        # Generate report
        generator.run(model)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get evidence types
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_evidence_types(self):
        evidence_types = []

        app = mobius.core.application()
        icons_path = app.get_data_path('icons/evidence')

        for t in pymobius.evidence.MODEL[:]:

            # Add icon to evidence type
            t['icon'] = f"{icons_path}/{t['id']}.png"

            # Clean up master_views.exporters
            for mv in t.get('master_views', []):
                mv.pop('exporters', None)

            evidence_types.append(t)

        return evidence_types
    
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse .ASAP file
    # @param path .ASAP file path
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __parse_asap_file(self, path):
        f = mobius.core.io.new_file_by_path(path)

        if not f.exists():
            raise Exception(f'File not found: {path}')
        
        data = {}

        ini = mobius.core.decoder.inifile(path, "iso-8859-1")
        data['laudo'] = dict((k.lower(), v) for (k, v) in ini.get_values('LAUDO'))
        data['solicitacao'] = dict((k.lower(), v) for (k, v) in ini.get_values('SOLICITACAO'))

        return data


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
