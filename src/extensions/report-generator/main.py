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
import os
import os.path
import threading
import traceback

import mobius
import pymobius
import pymobius.evidence
from gi.repository import GLib
from gi.repository import Gdk
from gi.repository import Gtk
from gi.repository import GdkPixbuf

from metadata import *
from action.generate_hashes_txt import GenerateHashesTxtAction

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Report actions
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ACTIONS = [GenerateHashesTxtAction
           ]

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Report generator view
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
TEMPLATE_ICON, TEMPLATE_ID, TEMPLATE_TYPE, TEMPLATE_NAME, GENERATOR_OBJ = range(5)

class ReportGeneratorView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()

        # Control variables
        self.__options = pymobius.Data()
        self.__options.template_id = None
        self.__options.template_type = None
        self.__options.output_folder = None
        self.__options.asap_file = None
        self.__options.hashes_txt_value = None
        self.__options.itemlist = []
        self.__actions = []
        self.__is_running = False

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
        vbox.add_child(grid, mobius.core.ui.box.fill_none)

        self.__general_options_hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        self.__general_options_hbox.set_spacing(5)
        self.__general_options_hbox.set_visible(True)
        grid.attach(self.__general_options_hbox.get_ui_widget(), 0, 0, 3, 1)

        label = mobius.core.ui.label()
        label.set_markup('<b>General options:</b>')
        label.set_halign(mobius.core.ui.label.align_left)
        label.set_visible(True)
        self.__general_options_hbox.add_child(label, mobius.core.ui.box.fill_none)
        self.__general_options_hbox.add_filler()

        label = mobius.core.ui.label()
        label.set_markup('<b>Template:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 1, 1, 1)

        # Template combobox model
        icon_path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'media.png')
        media_icon = GdkPixbuf.Pixbuf.new_from_file_at_size(icon_path, 24, 24)

        icon_path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'report.png')
        report_icon = GdkPixbuf.Pixbuf.new_from_file_at_size(icon_path, 24, 24)

        self.__template_model = Gtk.ListStore.new([GdkPixbuf.Pixbuf, str, str, str, object])

        for r in mobius.core.get_resources('report-template'):
            generator = r.value()

            for template in generator.templates:
                t_id = template['id']
                t_type = template['type']
                t_icon = media_icon if t_type == 'media' else report_icon
                t_description = template['description']
                self.__template_model.append([t_icon, t_id, t_type, t_description, generator])

        # Template combobox widget
        self.__template_combobox = Gtk.ComboBox.new_with_model(self.__template_model)
        self.__template_combobox.set_hexpand(True)
        self.__template_combobox.set_halign(Gtk.Align.FILL)

        spacer = Gtk.CellRendererText()
        self.__template_combobox.pack_start(spacer, True)
        spacer.set_property("xalign", 1.0)

        renderer = Gtk.CellRendererPixbuf()
        self.__template_combobox.pack_start(renderer, False)
        self.__template_combobox.add_attribute(renderer, 'pixbuf', TEMPLATE_ICON)

        renderer = Gtk.CellRendererText()
        self.__template_combobox.pack_start(renderer, False)
        self.__template_combobox.add_attribute(renderer, 'text', TEMPLATE_NAME)
        self.__template_combobox.set_id_column(TEMPLATE_ID)
        self.__template_combobox.connect('changed', self.__on_template_changed)
        self.__template_combobox.show()

        spacer = Gtk.CellRendererText()
        self.__template_combobox.pack_start(spacer, True)
        spacer.set_property("xalign", 0)

        grid.attach(self.__template_combobox, 1, 1, 2, 1)

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

        # Output folder
        label = mobius.core.ui.label()
        label.set_markup('<b>Output folder:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 3, 1, 1)

        # Output folder chooser button
        self.__output_folder_button = mobius.core.ui.button()
        self.__output_folder_button.set_icon_by_name('folder')
        self.__output_folder_button.set_text('Select output folder...')
        self.__output_folder_button.set_visible(True)
        self.__output_folder_button.set_callback('clicked', self.__on_click_output_folder)
        grid.attach(self.__output_folder_button.get_ui_widget(), 1, 3, 2, 1)

        # Add actions
        row = 4

        for r in mobius.core.get_resources('report.action'):
            action = r.value(self.__options)
            row = action.build_widget(grid, row)
            self.__actions.append(action)

        for a in ACTIONS:
            action = a(self.__options)
            row = action.build_widget(grid, row)
            self.__actions.append(action)

        # Status bar and Buttons
        vbox.add_filler()

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(10)
        hbox.set_visible(True)
        vbox.add_child(hbox, mobius.core.ui.box.fill_none)

        # status bar
        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.IN)
        frame.show()
        hbox.add_child(frame, mobius.core.ui.box.fill_with_widget)

        self.__status_label = mobius.core.ui.label("Testing")
        self.__status_label.set_halign(mobius.core.ui.label.align_left)
        self.__status_label.set_elide_mode(mobius.core.ui.label.elide_end)
        self.__status_label.set_visible(True)
        frame.add(self.__status_label.get_ui_widget())

        # Buttons
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

        last_output_folder = mobius.framework.get_config('last_report_folder')
        if last_output_folder:
            self.__set_output_folder(last_output_folder)

        last_asap_file = mobius.framework.get_config('last_asap_file')
        if last_asap_file:
            self.__options.asap_file = last_asap_file

        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
       if self.__is_running:
           return
       
       self.__options.itemlist = itemlist
       self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_destroy view
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set status text
    # @param text Text
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __set_status(self, text):
        if text:
            t = str(datetime.datetime.now())[:19]
            text = t + ' ' + text

        GLib.idle_add(self.__status_label.set_markup, text or '')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set output folder
    # @param path Output folder path
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __set_output_folder(self, path):
        if self.__options.output_folder != path:
            self.__options.output_folder = path

            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('last_report_folder', self.__options.output_folder)
            transaction.commit()

        for action in self.__actions:
            action.set_output_folder(path)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update output folder based on .ASAP file path and on template type selected
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __update_output_folder(self):
        if not self.__options.asap_file:
            return

        if self.__options.template_type == 'media':
            if self.__options.itemlist:
                case = self.__options.itemlist[0].case
                filename = os.path.basename(self.__options.asap_file)

                if filename.startswith("AsAP_Laudo_") and filename.endswith(".asap"):
                    parts = filename[len("AsAP_Laudo_"):-len(".asap")].split("-")
                    if len(parts) == 2:
                        number, year = parts
                        self.__set_output_folder(case.get_path(f"report/{year}-{number}"))

        elif self.__options.template_type == 'report':
            self.__set_output_folder(os.path.dirname(self.__options.asap_file))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update panel options
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __update_options(self):

        if self.__options.itemlist:
            self.__widget.show_content()
        else:
            self.__widget.set_message('Select item(s) to generate report')

        if self.__options.output_folder:
            self.__output_folder_button.set_text(self.__options.output_folder)
        else:
            self.__output_folder_button.set_text('Select output folder...')

        if self.__options.asap_file:
            self.__asap_file_button.set_text(self.__options.asap_file)
            self.__asap_clear_button.set_sensitive(True)
        else:
            self.__asap_file_button.set_text('Select a .ASAP file from the Federal Police of Brazil...')
            self.__asap_clear_button.set_sensitive(False)

        # Actions
        for action in self.__actions:
            action.update_options()

        # Execute button
        can_generate = not self.__is_running and bool(self.__options.template_id) and bool(self.__options.output_folder) and bool(self.__options.itemlist)
        self.__generate_button.set_sensitive(can_generate)

        # Status bar
        if self.__is_running:
            self.__set_status("Generating report...")
        else:
            self.__set_status("")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_template_changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_template_changed(self, combobox, *args):
        treemodel = combobox.get_model()
        treeiter = combobox.get_active_iter()

        if treeiter:
            self.__options.template_id = treemodel[treeiter][TEMPLATE_ID]
            self.__options.template_type = treemodel[treeiter][TEMPLATE_TYPE]

            if self.__options.template_id:
                transaction = mobius.framework.new_config_transaction()
                mobius.framework.set_config('last_report_template', self.__options.template_id)
                transaction.commit()

                self.__update_output_folder()

        else:
            self.__options.template_id = None
            self.__options.template_type = None

        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_output_folder button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_output_folder(self):
        dialog = Gtk.FileChooserDialog(title='Select output folder', action=Gtk.FileChooserAction.SELECT_FOLDER)
        dialog.add_buttons(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OPEN, Gtk.ResponseType.OK)

        if self.__options.output_folder:
            dialog.set_current_folder(self.__options.output_folder)

        response = dialog.run()

        if response == Gtk.ResponseType.OK:
            self.__set_output_folder(dialog.get_filename())

        dialog.destroy()

        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_asap_file button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_asap_file(self):

        # Run dialog
        dialog = Gtk.FileChooserDialog(title='Select .ASAP file', action=Gtk.FileChooserAction.OPEN)
        dialog.add_buttons(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OPEN, Gtk.ResponseType.OK)

        if self.__options.asap_file:
            dialog.set_filename(self.__options.asap_file)

        filefilter = Gtk.FileFilter()
        filefilter.set_name("ASAP files (*.asap)")
        filefilter.add_pattern("*.asap")
        dialog.add_filter(filefilter)

        response = dialog.run()
        selected_path = dialog.get_filename()
        dialog.destroy()

        if response != Gtk.ResponseType.OK:
            return

        # Set selected .ASAP file
        self.__options.asap_file = selected_path

        transaction = mobius.framework.new_config_transaction()
        mobius.framework.set_config('last_asap_file', self.__options.asap_file)
        transaction.commit()

        # Update output folder based on .ASAP file if available
        self.__update_output_folder()

        # Update panel options
        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_asap_clear_file button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_asap_clear_file(self):
        self.__options.asap_file = None
        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_generate_report button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_generate_report(self):

        # check if there is an older report
        if os.path.exists(os.path.join(self.__options.output_folder, "index.html")):
            dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
            dialog.text = f"Output folder already contains a report. Do you want to overwrite it?"
            dialog.add_button(mobius.core.ui.message_dialog.button_yes)
            dialog.add_button(mobius.core.ui.message_dialog.button_no)
            dialog.set_default_response(mobius.core.ui.message_dialog.button_no)
            rc = dialog.run()

            if rc != mobius.core.ui.message_dialog.button_yes:
                return

        # create model
        treemodel = self.__template_combobox.get_model()
        treeiter = self.__template_combobox.get_active_iter()

        model = pymobius.Data()
        model.template_id = self.__options.template_id
        model.template_type = treemodel[treeiter][TEMPLATE_TYPE]
        model.generator = treemodel[treeiter][GENERATOR_OBJ]
        model.output_folder = self.__options.output_folder
        model.update_hashes_txt = self.__update_hashes_txt_option_switch.get_active()
        model.case = self.__options.itemlist[0].case
        model.items = self.__options.itemlist[:]
        model.evidence_types = self.__get_evidence_types()
        model.extra_pages = []
        model.set_status = self.__set_status
        model.report_title = None

        # Add .ASAP data if available
        if self.__options.asap_file:
            model.asap = self.__parse_asap_file(self.__options.asap_file)
            model.asap_path = self.__options.asap_file
        else:
            model.asap = None
            model.asap_path = None

        # create thread
        t = threading.Thread(
            target=self.__generate_report_thread, args=(model,), daemon=True
        )
        t.start()

        # Set running state
        self.__is_running = True
        self.__generate_button.set_sensitive(False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Report generation thread
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_report_thread(self, model):
        try:
            # Multithreading guard to ensure thread safety when accessing Mobius core resources
            guard = mobius.core.thread_guard()
            connection = model.case.new_connection()

            # Generate report using generator
            self.__set_status("Generating report...")
            generator = model.generator
            generator.run(model)

            # Generate hashes.txt if template is media and option is enabled
            if model.template_type == 'media':
                self.__generate_hashes_txt(model)
                GLib.idle_add(self.__update_hashes_txt_label)

            # Update status
            self.__set_status("Report generation completed.")

        except Exception as e:
            self.__set_status(f"Error generating report: {e}")
            mobius.core.logf(f"ERR Error generating report: {str(e)}\n{traceback.format_exc()}")

        # Update UI
        self.__is_running = False
        GLib.idle_add(self.__update_options)

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
        
        ini = mobius.core.decoder.inifile(f.new_reader(), "ISO-8859-1")
        data = {}

        for k, v in ini.get_values('laudo').items():
            data[f'laudo.{k.lower()}'] = v

        for k, v in ini.get_values('solicitacao').items():
            data[f'solicitacao.{k.lower()}'] = v

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
