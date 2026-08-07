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
import shutil
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
        self.__template_id = None
        self.__template_type = None
        self.__output_folder = None
        self.__asap_file = None
        self.__hashes_txt_value = None
        self.__itemlist = []
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

        label = mobius.core.ui.label()
        label.set_markup('<b>Template:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 0, 1, 1)

        # CSS for frame
        css = b"""
            frame#text-frame {
            border: none;
            border-radius: 0;
            padding: 2px 8px;
            background-color: alpha(@theme_bg_color, 0.8);
            }
            """
        provider = Gtk.CssProvider()
        provider.load_from_data(css)

        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), provider,
            Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
        )

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

        grid.attach(self.__template_combobox, 1, 0, 2, 1)

        # .ASAP file path
        label = mobius.core.ui.label()
        label.set_markup('<b>.ASAP file (optional):</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 1, 1, 1)

        asap_hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        asap_hbox.set_spacing(5)
        asap_hbox.set_visible(True)
        grid.attach(asap_hbox.get_ui_widget(), 1, 1, 2, 1)

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
        grid.attach(label.get_ui_widget(), 0, 2, 1, 1)

        # Output folder chooser button
        self.__output_folder_button = mobius.core.ui.button()
        self.__output_folder_button.set_icon_by_name('folder')
        self.__output_folder_button.set_text('Select output folder...')
        self.__output_folder_button.set_visible(True)
        self.__output_folder_button.set_callback('clicked', self.__on_click_output_folder)
        grid.attach(self.__output_folder_button.get_ui_widget(), 1, 2, 2, 1)

        # Media option: Update hashes.txt
        self.__update_hashes_txt_option_label = mobius.core.ui.label()
        self.__update_hashes_txt_option_label.set_markup('<b>Update hashes.txt file:</b>')
        self.__update_hashes_txt_option_label.set_halign(mobius.core.ui.label.align_right)
        self.__update_hashes_txt_option_label.set_visible(True)
        grid.attach(self.__update_hashes_txt_option_label.get_ui_widget(), 0, 3, 1, 1)

        self.__update_hashes_txt_option_hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        self.__update_hashes_txt_option_hbox.set_spacing(5)
        self.__update_hashes_txt_option_hbox.set_visible(True)
        grid.attach(self.__update_hashes_txt_option_hbox.get_ui_widget(), 1, 3, 2, 1)

        self.__update_hashes_txt_option_switch = Gtk.Switch.new()
        self.__update_hashes_txt_option_switch.set_visible(True)
        self.__update_hashes_txt_option_switch.set_active(False)
        #self.__update_hashes_txt_option_switch.connect('state-set', self.__on_update_hashes_switch_state_set)
        self.__update_hashes_txt_option_hbox.add_child(self.__update_hashes_txt_option_switch, mobius.core.ui.box.fill_none)
        self.__update_hashes_txt_option_hbox.add_filler()

        # Hashes.txt value and copy button
        self.__hashes_txt_value_label = mobius.core.ui.label()
        self.__hashes_txt_value_label.set_markup("<b>Hashes.txt (SHA2-256):</b>")
        self.__hashes_txt_value_label.set_halign(mobius.core.ui.label.align_right)
        self.__hashes_txt_value_label.set_visible(True)
        grid.attach(self.__hashes_txt_value_label.get_ui_widget(), 0, 4, 1, 1)

        self.__hashes_txt_value_hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        self.__hashes_txt_value_hbox.set_spacing(5)
        self.__hashes_txt_value_hbox.set_visible(True)
        self.__hashes_txt_value_hbox.get_ui_widget().set_hexpand(True)
        grid.attach(self.__hashes_txt_value_hbox.get_ui_widget(), 1, 4, 2, 1)

        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.IN)
        frame.set_name("text-frame")
        frame.show()
        self.__hashes_txt_value_hbox.add_child(frame, mobius.core.ui.box.fill_with_widget)

        self.__hashes_txt_hash_label = mobius.core.ui.label()
        self.__hashes_txt_hash_label.set_halign(mobius.core.ui.label.align_left)
        self.__hashes_txt_hash_label.set_selectable(True)
        self.__hashes_txt_hash_label.show()
        frame.add(self.__hashes_txt_hash_label.get_ui_widget())

        self.__hashes_txt_copy_button = mobius.core.ui.button()
        self.__hashes_txt_copy_button.set_icon_by_name('edit-copy')
        self.__hashes_txt_copy_button.set_visible(True)
        self.__hashes_txt_copy_button.set_sensitive(False)
        self.__hashes_txt_copy_button.set_callback('clicked', self.__on_click_hashes_txt_copy)
        self.__hashes_txt_value_hbox.add_child(self.__hashes_txt_copy_button, mobius.core.ui.box.fill_none)

        # Status bar and Buttons
        vbox.add_filler()

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(10)
        hbox.set_visible(True)
        vbox.add_child(hbox, mobius.core.ui.box.fill_none)

        # status bar
        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.IN)
        frame.set_name("text-frame")
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
            self.__output_folder = last_output_folder
            if os.path.exists(os.path.join(self.__output_folder, "hashes.txt")):
                self.__update_hashes_txt_option_switch.set_active(True)

        last_asap_file = mobius.framework.get_config('last_asap_file')
        if last_asap_file:
            self.__asap_file = last_asap_file

        self.__update_hashes_txt_label()
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
       
       self.__itemlist = itemlist
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
        self.__output_folder = path

        transaction = mobius.framework.new_config_transaction()
        mobius.framework.set_config('last_report_folder', self.__output_folder)
        transaction.commit()

        if os.path.exists(os.path.join(path, "hashes.txt")):
            self.__update_hashes_txt_option_switch.set_active(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update output folder based on .ASAP file path and on template type selected
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __update_output_folder(self):
        if not self.__asap_file:
            return

        if self.__template_type == 'media':
            if self.__itemlist:
                case = self.__itemlist[0].case
                filename = os.path.basename(self.__asap_file)

                if filename.startswith("AsAP_Laudo_") and filename.endswith(".asap"):
                    parts = filename[len("AsAP_Laudo_"):-len(".asap")].split("-")
                    if len(parts) == 2:
                        number, year = parts
                        self.__set_output_folder(case.get_path(f"report/{year}-{number}"))

        elif self.__template_type == 'report':
            self.__set_output_folder(os.path.dirname(self.__asap_file))

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

        if self.__asap_file:
            self.__asap_file_button.set_text(self.__asap_file)
            self.__asap_clear_button.set_sensitive(True)
        else:
            self.__asap_file_button.set_text('Select a .ASAP file from the Federal Police of Brazil...')
            self.__asap_clear_button.set_sensitive(False)

        if self.__hashes_txt_value:
            self.__hashes_txt_copy_button.set_sensitive(True)
        else:
            self.__hashes_txt_copy_button.set_sensitive(False)

        # Media options
        if self.__template_type == 'media':
            is_media = True
        else:
            is_media = False

        if is_media and self.__output_folder and os.path.exists(os.path.join(self.__output_folder, "hashes.txt")):
            has_hashes_txt = True
        else:
            has_hashes_txt = False

        self.__update_hashes_txt_option_label.set_visible(has_hashes_txt)
        self.__update_hashes_txt_option_hbox.set_visible(has_hashes_txt)
        self.__hashes_txt_value_label.set_visible(has_hashes_txt)
        self.__hashes_txt_value_hbox.set_visible(has_hashes_txt)

        # Execute button
        can_generate = not self.__is_running and bool(self.__template_id) and bool(self.__output_folder) and bool(self.__itemlist)
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
            self.__template_id = treemodel[treeiter][TEMPLATE_ID]
            self.__template_type = treemodel[treeiter][TEMPLATE_TYPE]

            if self.__template_id:
                transaction = mobius.framework.new_config_transaction()
                mobius.framework.set_config('last_report_template', self.__template_id)
                transaction.commit()

                self.__update_output_folder()

        else:
            self.__template_id = None
            self.__template_type = None

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

        if self.__asap_file:
            dialog.set_filename(self.__asap_file)

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
        self.__asap_file = selected_path

        transaction = mobius.framework.new_config_transaction()
        mobius.framework.set_config('last_asap_file', self.__asap_file)
        transaction.commit()

        # Update output folder based on .ASAP file if available
        self.__update_output_folder()

        # Update panel options
        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_asap_clear_file button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_asap_clear_file(self):
        self.__asap_file = None
        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_hashes_txt_copy button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_hashes_txt_copy(self):
        if self.__hashes_txt_value:
            clipboard = Gtk.Clipboard.get(Gdk.SELECTION_CLIPBOARD)
            clipboard.set_text(self.__hashes_txt_value, -1)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_generate_report button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_generate_report(self):

        # check if there is an older report
        if os.path.exists(os.path.join(self.__output_folder, "index.html")):
            dialog = mobius.core.ui.message_dialog(
                mobius.core.ui.message_dialog.type_question
            )
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
        model.template_id = self.__template_id
        model.template_type = treemodel[treeiter][TEMPLATE_TYPE]
        model.generator = treemodel[treeiter][GENERATOR_OBJ]
        model.output_folder = self.__output_folder
        model.update_hashes_txt = self.__update_hashes_txt_option_switch.get_active()
        model.case = self.__itemlist[0].case
        model.items = self.__itemlist[:]
        model.evidence_types = self.__get_evidence_types()
        model.extra_pages = []
        model.set_status = self.__set_status
        model.report_title = None

        # Add .ASAP data if available
        if self.__asap_file:
            model.asap = self.__parse_asap_file(self.__asap_file)
            model.asap_path = self.__asap_file
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

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate hashes.txt
    # @param model Model data structure
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_hashes_txt(self, model):

        self.__set_status("Generating hashes.txt file...")
        hashes_txt_path = os.path.join(model.output_folder, "hashes.txt")

        # If model.update_hashes_txt is True, read hashes from current hashes.txt
        cached_hashes = {}
        hashes_txt_mtime = None

        if model.update_hashes_txt and os.path.exists(hashes_txt_path):
            hashes_txt_mtime = os.path.getmtime(hashes_txt_path)

            with open(hashes_txt_path, 'r') as hf:
                for line in hf:
                    parts = line.strip().split(' ?SHA256*')
                    if len(parts) == 2:
                        cached_hashes[parts[1]] = parts[0]

        # Remove old hashes.txt, if any
        old_f = mobius.core.io.new_file_by_path(hashes_txt_path)
        if old_f.exists():
            old_f.remove()

        # create temporary file
        f = mobius.core.io.tempfile()
        writer = mobius.core.io.text_writer(f.new_writer())

        # generate hashes.txt
        pos = len(model.output_folder) + 1

        for root, dirs, files in os.walk(model.output_folder, topdown=False):
            for name in files:
                path = os.path.join(root, name)
                filename = path[pos:]

                if cached_hashes and os.path.getmtime(path) < hashes_txt_mtime:
                    hash_value = cached_hashes.get(filename, self.__get_hash(path))
                else:
                    hash_value = self.__get_hash(path)

                writer.write(f"{hash_value} ?SHA256*{filename}\n")

        writer.flush()

        # move file to output_path
        shutil.copyfile(f.path, hashes_txt_path)
        os.remove(f.path)

        # Calculate hash of hashes.txt
        self.__hashes_txt_value = self.__get_hash(hashes_txt_path)

        # Write hashes_txt value back to .asap file if available
        if model.asap_path:
            self.__update_asap_file(model.asap_path, self.__hashes_txt_value)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update hashes.txt label
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __update_hashes_txt_label(self):
        self.__hashes_txt_hash_label.set_text('')
        self.__hashes_txt_copy_button.set_sensitive(False)

        if not self.__output_folder:
            return
        
        hashes_txt_path = os.path.join(self.__output_folder, "hashes.txt")
        if not os.path.exists(hashes_txt_path):
            return
                
        self.__set_status("Calculating <b>hashes.txt</b> hash...")
     
        self.__hashes_txt_value = self.__get_hash(hashes_txt_path)
        self.__hashes_txt_hash_label.set_text(self.__hashes_txt_value)

        self.__set_status("Calculated hash for <b>hashes.txt</b>.")
        self.__hashes_txt_copy_button.set_sensitive(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Calculate file hash
    # @param path File path
    # @return Hash as string
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_hash(self, path):
        h = mobius.core.crypt.hash("sha2-256")
        f = mobius.core.io.new_file_by_path(path)
        reader = f.new_reader()
        block_size = 512 * 1024  # 512 KB

        data = reader.read(block_size)
        while data:
            h.update(data)
            data = reader.read(block_size)

        return h.get_hex_digest()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update .ASAP file with hashes.txt value
    # @param path .ASAP file path
    # @param hashes_txt_value Hashes.txt value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __update_asap_file(self, path, hashes_txt_value):
        f = mobius.core.io.new_file_by_path(path)

        if not f.exists():
            raise Exception(f'File not found: {path}')
        
        fp = mobius.core.io.line_reader(f.new_reader(), "ISO-8859-1")
        tmpf = mobius.core.io.tempfile()
        fw = mobius.core.io.text_writer(tmpf.new_writer(), "ISO-8859-1")

        for line in fp:

            # Ignore old MIDIA_GERADA_HASHES_TXT line
            if not line.startswith("MIDIA_GERADA_HASHES_TXT="):
                fw.write(line + "\n")

            # Create new MIDIA_GERADA_HASHES_TXT line if not present
            if line.startswith("MIDIA_GERADA_DESCRICAO="):
                fw.write(f"MIDIA_GERADA_HASHES_TXT={hashes_txt_value}\n")

        fw.flush()
        shutil.copyfile(tmpf.path, path)
        os.remove(tmpf.path)

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
