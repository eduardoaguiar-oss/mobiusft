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
import shutil
import threading

import mobius
import pymobius
import pymobius.evidence
from gi.repository import GLib
from gi.repository import Gdk
from gi.repository import Gtk
from gi.repository import GdkPixbuf

from metadata import *

class Model(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.output_folder = None
        self.asap_file = None
        self.items = []
        self.template_id = None
        self.actions = []
        self.generate_hashes_txt = False
        self.update_hashes_txt = False
        self.update_asap_file = False
        self.hashes_txt_hash_type = "sha2-256"

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Clone object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def clone(self):
        new_model = Model()
        new_model.output_folder = self.output_folder
        new_model.asap_file = self.asap_file
        new_model.items = self.items[:]
        new_model.template_id = self.template_id
        new_model.generate_hashes_txt = self.generate_hashes_txt
        new_model.update_hashes_txt = self.update_hashes_txt
        new_model.update_asap_file = self.update_asap_file
        new_model.hashes_txt_hash_type = self.hashes_txt_hash_type
        new_model.actions = self.actions[:]

        return new_model
    
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Add action
    # @param action Action
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_action(self, action):
        self.actions.append(action)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def update(self):
        flag_update = True

        while(flag_update):
            flag_update = False

            for action in self.actions:
                flag_update |= action.update()

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Action to generate hashes.txt file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class HashesTxtAction(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize action
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, model):
        self.__name = "Generate hashes.txt"
        self.__model = model
        self.__model.generate_hashes_txt = False
        self.__model.update_hashes_txt = False
        self.__model.update_asap_file = False
        self.__model.hashes_txt_hash_type = "sha2-256"

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Build widget
    # @param grid Grid
    # @param row Row
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_widget(self, grid, row):
        self.__switch = Gtk.Switch(active=self.is_enabled())
        self.__switch.set_visible(True)
        self.__switch.connect('state-set', self.__on_switch_action)
        grid.attach(self.__switch, 0, row, 1, 1)

        label = mobius.core.ui.label()
        label.set_markup(f"<b>{self.get_name()}</b>")
        label.set_halign(mobius.core.ui.label.align_left)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 1, row, 1, 1)

        label = mobius.core.ui.label()
        label.set_markup(f"<i>{self.get_options_text()}</i>")
        label.set_halign(mobius.core.ui.label.align_left)
        label.set_visible(True)
        label.get_ui_widget().set_hexpand(True)
        grid.attach(label.get_ui_widget(), 2, row, 1, 1)

        self.__options_button = mobius.core.ui.button()
        self.__options_button.set_icon_by_name('preferences-system')
        self.__options_button.set_visible(True)
        self.__options_button.set_sensitive(False)
        self.__options_button.set_callback('clicked', self.__on_click_action_options)
        grid.attach(self.__options_button.get_ui_widget(), 3, row, 1, 1)

        self.__revealer = Gtk.Revealer.new()
        self.__revealer.set_transition_type(Gtk.RevealerTransitionType.SLIDE_DOWN)
        self.__revealer.set_reveal_child(False)
        self.__revealer.set_visible(True)
        grid.attach(self.__revealer, 0, row + 1, 3, 1)

        # Options widget
        self.__options_widget = Gtk.Grid.new()
        self.__options_widget.set_row_spacing(10)
        self.__options_widget.set_column_spacing(5)
        self.__options_widget.set_column_homogeneous(False)
        self.__options_widget.show()

        label = mobius.core.ui.label()
        label.set_markup('<b>Update hashes.txt file:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        self.__options_widget.attach(label.get_ui_widget(), 0, 0, 1, 1)

        self.__update_hashes_switch = Gtk.Switch.new()
        self.__update_hashes_switch.set_visible(True)
        self.__update_hashes_switch.set_active(False)
        #self.__update_hashes_switch.connect('state-set', self.__on_update_hashes_switch_state_set)
        self.__options_widget.attach(self.__update_hashes_switch, 1, 0, 1, 1)

        self.__revealer.add(self.__options_widget)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Check if action is enabled
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def is_enabled(self):
        return self.__model.generate_hashes_txt

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get action name
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_name(self):
        return self.__name

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_widget(self):
        return self.__widget
    
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get options text
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_options_text(self):
        if not self.__model.generate_hashes_txt:
            return ""
        
        text = f"Hash type: {self.__model.hashes_txt_hash_type.upper()}"

        if self.__model.update_hashes_txt:
            text += ", update hashes.txt"
        else:
            text += ", overwrite hashes.txt"

        if self.__model.update_asap_file:
            text += ", update .ASAP file"

        return text

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set enabled option
    # @param enabled Enabled
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_enabled(self, enabled):
        self.__switch.set_active(enabled)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update options
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def update(self):
        flag_updated = False

        if not self.__model.output_folder and self.__model.generate_hashes_txt:
            self.__model.generate_hashes_txt = False
            flag_updated = True

        if not self.__model.asap_file and self.__model.update_asap_file:
            self.__model.update_asap_file = False
            flag_updated = True

        is_enabled = self.is_enabled()
        self.__options_button.set_sensitive(is_enabled)
        if not is_enabled:
            self.__revealer.set_reveal_child(False)

        return flag_updated

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run action
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self, model):
        self.__view.__generate_hashes_txt(model)
        GLib.idle_add(self.__view.__update_hashes_txt_label)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_switch_action switch toggled
    # @param switch Switch
    # @param action Action
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_switch_action(self, switch, active):
        self.__model.generate_hashes_txt = active
        self.__model.update()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_action_options button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_action_options(self):
        is_revealed = self.__revealer.get_reveal_child()
        self.__revealer.set_reveal_child(not is_revealed)



TEMPLATE_ICON, TEMPLATE_ID, TEMPLATE_TYPE, TEMPLATE_NAME, GENERATOR_OBJ = range(5)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Report generator view
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ReportGeneratorView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()

        # Initialize model
        self.__model = Model()
        self.__model.add_action(HashesTxtAction(self.__model))

        # Control variables
        self.__template_id = None
        self.__asap_file = None
        self.__generate_hashes = True
        self.__is_running = False
        self.__hashes_txt_value = None
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

        # Actions
        label = mobius.core.ui.label()
        label.set_markup('<b>Actions:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 3, 1, 1)

        actions_grid = Gtk.Grid.new()
        actions_grid.set_row_spacing(10)
        actions_grid.set_column_spacing(5)
        actions_grid.set_column_homogeneous(False)
        actions_grid.set_visible(True)
        grid.attach(actions_grid, 1, 3, 2, 1)

        # Each action has a switch, a label, an option list, an options button
        # and an options revealer
        row = 0

        for action in self.__model.actions:
            action.build_widget(actions_grid, row)
            row += 2

        # Media options revealer
        self.__media_options_revealer = Gtk.Revealer.new()
        self.__media_options_revealer.set_transition_type(Gtk.RevealerTransitionType.SLIDE_DOWN)
        self.__media_options_revealer.set_reveal_child(False)
        self.__media_options_revealer.set_visible(True)
        grid.attach(self.__media_options_revealer, 1, 4, 2, 1)

        media_vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        media_vbox.set_spacing(10)
        media_vbox.set_border_width(10)
        media_vbox.set_visible(True)
        self.__media_options_revealer.add(media_vbox.get_ui_widget())

        label = mobius.core.ui.label()
        label.set_markup("<b>Media options:</b>")
        label.set_halign(mobius.core.ui.label.align_left)
        label.set_visible(True)
        media_vbox.add_child(label.get_ui_widget(), mobius.core.ui.box.fill_none)

        # Hashes.txt value and copy button
        hashes_hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hashes_hbox.set_spacing(10)
        hashes_hbox.set_visible(True)
        media_vbox.add_child(hashes_hbox, mobius.core.ui.box.fill_none)

        label = mobius.core.ui.label()
        label.set_markup("<b>Hashes.txt (SHA2-256):</b>")
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        hashes_hbox.add_child(label.get_ui_widget(), mobius.core.ui.box.fill_none)

        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.IN)
        frame.set_name("text-frame")
        frame.show()
        hashes_hbox.add_child(frame, mobius.core.ui.box.fill_with_widget)

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
        hashes_hbox.add_child(self.__hashes_txt_copy_button, mobius.core.ui.box.fill_none)

        # Status bar and Buttons
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
            self.__model.output_folder = last_output_folder

        last_asap_file = mobius.framework.get_config('last_asap_file')
        if last_asap_file:
            self.__asap_file = last_asap_file

        self.__update_hashes_txt_label()
        self.__update_options()

        # Subscribe to events
        self.__event_uid1 = mobius.core.subscribe('config-set', self.__on_config_set)
        self.__event_uid2 = mobius.core.subscribe('config-remove', self.__on_config_remove)

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
        mobius.core.unsubscribe(self.__event_uid1)
        mobius.core.unsubscribe(self.__event_uid2)
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
    # @brief Update panel options
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __update_options(self):

        if self.__itemlist:
            self.__widget.show_content()
        else:
            self.__widget.set_message('Select item(s) to generate report')

        if self.__model.output_folder:
            self.__output_folder_button.set_text(self.__model.output_folder)
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

        # Media releaver
        treemodel = self.__template_combobox.get_model()
        treeiter = self.__template_combobox.get_active_iter()

        if treeiter:
            template_type = treemodel[treeiter][TEMPLATE_TYPE]
            is_media = (template_type == 'media')
        else:
            is_media = False

        self.__media_options_revealer.set_reveal_child(is_media)

        # Execute button
        can_generate = not self.__is_running and bool(self.__model. template_id) and bool(self.__model.output_folder) and bool(self.__itemlist)
        self.__generate_button.set_sensitive(can_generate)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_template_changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_template_changed(self, combobox, *args):
        treemodel = combobox.get_model()
        treeiter = combobox.get_active_iter()

        if treeiter:
            self.__model.template_id = treemodel[treeiter][TEMPLATE_ID]

            if self.__model.template_id:
                transaction = mobius.framework.new_config_transaction()
                mobius.framework.set_config('last_report_template', self.__model.template_id)
                transaction.commit()

            self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_output_folder button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_output_folder(self):
        dialog = Gtk.FileChooserDialog(title='Select output folder', action=Gtk.FileChooserAction.SELECT_FOLDER)
        dialog.add_buttons(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OPEN, Gtk.ResponseType.OK)

        if self.__model.output_folder:
            dialog.set_current_folder(self.__model.output_folder)

        response = dialog.run()

        if response == Gtk.ResponseType.OK:
            self.__model.output_folder = dialog.get_filename()

            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('last_report_folder', self.__model.output_folder)
            transaction.commit()

        dialog.destroy()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_asap_file button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_asap_file(self):
        dialog = Gtk.FileChooserDialog(title='Select .ASAP file', action=Gtk.FileChooserAction.OPEN)
        dialog.add_buttons(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OPEN, Gtk.ResponseType.OK)

        if self.__asap_file:
            dialog.set_filename(self.__asap_file)

        filefilter = Gtk.FileFilter()
        filefilter.set_name("ASAP files (*.asap)")
        filefilter.add_pattern("*.asap")
        dialog.add_filter(filefilter)

        response = dialog.run()

        if response == Gtk.ResponseType.OK:
            self.__asap_file = dialog.get_filename()
            self.__asap_file_button.set_text(self.__asap_file)

            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('last_asap_file', self.__asap_file)
            transaction.commit()

        dialog.destroy()
        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_asap_clear_file button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_asap_clear_file(self):
        self.__asap_file = None
        self.__asap_file_button.set_text('Select a .ASAP file from the Federal Police of Brazil...')
        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_hashes_txt_copy button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_hashes_txt_copy(self):
        if self.__hashes_txt_value:
            clipboard = Gtk.Clipboard.get(Gdk.SELECTION_CLIPBOARD)
            clipboard.set_text(self.__hashes_txt_value, -1)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_generate_hashes_toggled checkbutton toggled
    # @param checkbutton Checkbutton
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_generate_hashes_toggled(self, checkbutton):
        self.__generate_hashes = checkbutton.get_active()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_config_set event handler
    # @param name Config name
    # @param value Config value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_config_set(self, name, value):
        if name == 'last_report_folder':
            self.__model.output_folder = value
            self.__update_hashes_txt_label()
            self.__update_options()

        elif name == 'last_asap_file':
            self.__asap_file = value
            self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_config_remove event handler
    # @param name Config name
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_config_remove(self, name):
        if name == 'last_report_folder':
            self.__model.output_folder = None
            self.__update_hashes_txt_label()
            self.__update_options()

        elif name == 'last_asap_file':
            self.__asap_file = None
            self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_generate_report button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_generate_report(self):

        # check if there is an older report
        if os.path.exists(os.path.join(self.__model.output_folder, "index.html")):
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

        model = self.__model.clone()
        model.generator = treemodel[treeiter][GENERATOR_OBJ]
        model.case = self.__itemlist[0].case
        model.items = self.__itemlist[:]
        model.template_type = treemodel[treeiter][TEMPLATE_TYPE]
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
            guard = mobius.core.thread_guard()
            connection = model.case.new_connection()

            # Generate report using generator
            self.__set_status("Generating report...")
            generator = model.generator
            generator.run(model)

            # Generate hashes.txt if template is media and option is enabled
            print("model.template_type, model.generate_hashes_txt", model.template_type, model.generate_hashes_txt)
            if model.template_type == 'media' and model.generate_hashes_txt:
                self.__generate_hashes_txt(model)
                GLib.idle_add(self.__update_hashes_txt_label)

            # Update UI
            self.__set_status("Report generation completed.")

        except Exception as e:
            self.__set_status(f"Error generating report: {e}")

        # Set running state
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

        # remove old hashes.txt, if any
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
                hash_value = self.__get_hash(path)
                filename = path[pos:]
                writer.write(f"{hash_value} ?SHA256*{filename}\n")

        writer.flush()

        # move file to output_path
        shutil.copyfile(f.path, hashes_txt_path)
        os.remove(f.path)

        # calculate hash of hashes.txt
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

        if not self.__model.output_folder:
            return
        
        hashes_txt_path = os.path.join(self.__model.output_folder, "hashes.txt")
        if not os.path.exists(hashes_txt_path):
            return
                
        self.__set_status("Calculating <b>hashes.txt</b> hash...")
     
        self.__hashes_txt_value = self.__get_hash(hashes_txt_path)
        self.__hashes_txt_hash_label.set_text(self.__hashes_txt_value)

        self.__set_status("<b>hashes.txt</b> file hash value calculated.")
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
