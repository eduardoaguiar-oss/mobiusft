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
from gi.repository import Gtk
from gi.repository import Gdk

import os
import mobius


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief NoContent list widget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class NoContentListWidget(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, options):
        self.__options = options

        # Main container
        self.widget = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
        self.widget.set_visible(True)

        # === Input row: Entry + Add button ===
        input_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=6)
        input_box.set_visible(True)
        self.widget.pack_start(input_box, False, False, 0)

        self.__entry = Gtk.Entry()
        self.__entry.set_placeholder_text("Enter value to exclude (e.g. /tmp, *.tmp, ...)")
        self.__entry.set_hexpand(True)
        self.__entry.set_visible(True)
        self.__entry.connect("activate", self.__on_add_clicked)  # Enter key adds
        input_box.pack_start(self.__entry, True, True, 0)

        add_button = Gtk.Button.new_with_label("Add")
        add_button.set_visible(True)
        add_button.connect("clicked", self.__on_add_clicked)
        input_box.pack_start(add_button, False, False, 0)

        # === List ===
        scrolled = Gtk.ScrolledWindow()
        scrolled.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        scrolled.set_min_content_height(120)   # adjust as needed
        scrolled.set_visible(True)
        self.widget.pack_start(scrolled, True, True, 0)

        self.__listbox = Gtk.ListBox()
        self.__listbox.set_selection_mode(Gtk.SelectionMode.SINGLE)
        self.__listbox.set_visible(True)
        scrolled.add(self.__listbox)

        # Add last saved items to the listbox
        last_no_content_items = sorted(mobius.framework.get_config('iped.last_no_content_items') or [])
        self.set_items(last_no_content_items)

        # Optional: allow removing with Delete key
        self.__listbox.connect("key-press-event", self.__on_key_press)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get UI widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set visibility of the widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_visible(self, visible):
        self.widget.set_visible(visible)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get current list of items
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_items(self):
        return self.__options.iped_no_content_items.copy()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Replace the current list with new items
    # @param items List of strings to set
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_items(self, items):
        self.clear()
        for item in items:
            self.__options.iped_no_content_items.append(item)
            self.__add_row(item)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Clear the list
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def clear(self):
        self.__options.iped_no_content_items.clear()
        for row in self.__listbox.get_children():
            self.__listbox.remove(row)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Add item to the list
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_add_clicked(self, widget):
        text = self.__entry.get_text().strip()
        if not text:
            return

        if text in self.__options.iped_no_content_items:
            # Optional: prevent duplicates
            self.__entry.set_text("")
            return

        self.__options.iped_no_content_items.append(text)
        self.__add_row(text)
        self.__entry.set_text("")
        self.__entry.grab_focus()

        # Save the updated list to config
        transaction = mobius.framework.new_config_transaction()
        mobius.framework.set_config('iped.last_no_content_items', self.__options.iped_no_content_items)
        transaction.commit()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Add a row to the listbox
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __add_row(self, text):
        row = Gtk.ListBoxRow()
        row.set_visible(True)

        hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=6)
        hbox.set_visible(True)
        hbox.set_margin_start(6)
        hbox.set_margin_end(6)
        hbox.set_margin_top(4)
        hbox.set_margin_bottom(4)

        label = Gtk.Label(label=text)
        label.set_xalign(0)
        label.set_hexpand(True)
        label.set_visible(True)
        hbox.pack_start(label, True, True, 0)

        remove_btn = Gtk.Button.new_from_icon_name("edit-delete-symbolic", Gtk.IconSize.BUTTON)
        remove_btn.set_tooltip_text("Remove this entry")
        remove_btn.set_visible(True)
        remove_btn.connect("clicked", self.__on_remove_clicked, row, text)
        hbox.pack_start(remove_btn, False, False, 0)

        row.add(hbox)
        self.__listbox.add(row)
        row.show_all()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Remove item from the list
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_remove_clicked(self, button, row, text):
        if text in self.__options.iped_no_content_items:
            self.__options.iped_no_content_items.remove(text)
        self.__listbox.remove(row)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle key press events for the listbox
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_key_press(self, widget, event):

        # Only handle Delete key
        if event.keyval != Gdk.KEY_Delete:
            return False

        # Get the selected row
        row = self.__listbox.get_selected_row()
        if not row:
            return False

        # Find the text from the label
        hbox = row.get_child()
        for child in hbox.get_children():
            if isinstance(child, Gtk.Label):
                text = child.get_text()
                if text in self.__options.iped_no_content_items:
                    self.__options.iped_no_content_items.remove(text)
                break
        self.__listbox.remove(row)

        return True


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief IPED report generator action
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class IPEDReportGeneratorAction(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize options
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, options):
        options.iped_generate_report = False
        options.iped_wordlist_path = None
        options.iped_no_content_items = []

        self.__output_folder = options.output_folder
        self.__options = options

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Build options widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_widget(self, grid, row):

        self.__placeholder = mobius.core.ui.label()
        self.__placeholder.set_visible(True)
        grid.attach(self.__placeholder.get_ui_widget(), 0, row, 3, 1)

        # Main option: Generate IPED report
        self.__action_label = mobius.core.ui.label()
        self.__action_label.set_markup('<b>Generate IPED report</b>')
        self.__action_label.set_halign(mobius.core.ui.label.align_left)
        self.__action_label.set_visible(True)
        grid.attach(self.__action_label.get_ui_widget(), 0, row + 1, 1, 1)

        self.__action_hbox = mobius.core
        self.__action_hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        self.__action_hbox.set_spacing(5)
        self.__action_hbox.set_visible(True)
        grid.attach(self.__action_hbox.get_ui_widget(), 1, row + 1, 2, 1)

        self.__action_switch = Gtk.Switch.new()
        self.__action_switch.set_visible(True)
        self.__action_switch.set_active(True)
        self.__action_switch.set_hexpand(False)
        self.__action_switch.connect('notify::active', self.__on_action_switch_modified)
        self.__action_hbox.add_child(self.__action_switch, mobius.core.ui.box.fill_none)
        self.__action_hbox.add_filler()

        # Option: Wordlist file path
        self.__wordlist_label = mobius.core.ui.label()
        self.__wordlist_label.set_markup('<b>Wordlist (optional):</b>')
        self.__wordlist_label.set_halign(mobius.core.ui.label.align_right)
        self.__wordlist_label.set_visible(True)
        grid.attach(self.__wordlist_label.get_ui_widget(), 0, row + 2, 1, 1)

        self.__wordlist_hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        self.__wordlist_hbox.set_spacing(5)
        self.__wordlist_hbox.set_visible(True)
        grid.attach(self.__wordlist_hbox.get_ui_widget(), 1, row + 2, 2, 1)

        self.__wordlist_file_button = mobius.core.ui.button()
        self.__wordlist_file_button.set_icon_by_name('folder')
        self.__wordlist_file_button.set_visible(True)
        self.__wordlist_file_button.get_ui_widget().set_hexpand(True)
        self.__wordlist_file_button.set_callback('clicked', self.__on_click_wordlist_file)
        self.__wordlist_hbox.add_child(self.__wordlist_file_button, mobius.core.ui.box.fill_with_widget)

        self.__wordlist_clear_button = mobius.core.ui.button()
        self.__wordlist_clear_button.set_icon_by_name('edit-clear')
        self.__wordlist_clear_button.set_visible(True)
        self.__wordlist_clear_button.set_sensitive(False)
        self.__wordlist_clear_button.set_callback('clicked', self.__on_click_wordlist_clear_file)
        self.__wordlist_hbox.add_child(self.__wordlist_clear_button, mobius.core.ui.box.fill_none)

        # Option: No content
        self.__no_content_label = mobius.core.ui.label()
        self.__no_content_label.set_markup('\n<b>No content:</b>')
        self.__no_content_label.set_halign(mobius.core.ui.label.align_right)
        self.__no_content_label.set_valign(mobius.core.ui.label.align_top)
        self.__no_content_label.set_visible(True)
        grid.attach(self.__no_content_label.get_ui_widget(), 0, row + 3, 1, 1)

        # No content widget
        self.__no_content_widget = NoContentListWidget(self.__options)
        self.__no_content_widget.set_visible(True)
        grid.attach(self.__no_content_widget.get_ui_widget(), 1, row + 3, 2, 1)

        # Update options based on current state
        last_wordlist_path = mobius.framework.get_config('iped.last_wordlist_file')
        if last_wordlist_path:
            self.__options.iped_wordlist_path = last_wordlist_path

        self.set_output_folder(self.__output_folder)
        self.update_options()

        return row + 4

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set output folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_output_folder(self, output_folder):
        self.__output_folder = output_folder

        # Enable IPED report if has processed items, but not an IPED report on output folder
        processed_items = self.__get_processed_items(self.__options.itemlist)
        has_iped_report = False

        if self.__output_folder:
            search_path = os.path.join(self.__output_folder, "iped", "lib", "iped-search-app.jar")
            has_iped_report = os.path.exists(search_path)

        if processed_items and not has_iped_report:
            self.__action_switch.set_active(True)
        else:
            self.__action_switch.set_active(False)

        # Update options based on current state
        self.update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update options widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def update_options(self):
        processed_items = self.__get_processed_items(self.__options.itemlist)
        is_available = self.__options.template_type == 'media' and bool(self.__output_folder) and bool(processed_items)
        is_enabled = is_available and self.__action_switch.get_active()
        print(f"IPED report options updated: is_available={is_available}, is_enabled={is_enabled}")
        print(f"IPED report options: output_folder={self.__output_folder}, template_type={self.__options.template_type}, processed_items={len(processed_items)}")

        # Show action, if action can run
        self.__placeholder.set_visible(is_available)
        self.__action_label.set_visible(is_available)
        self.__action_hbox.set_visible(is_available)

        # Option: Wordlist
        self.__wordlist_label.set_visible(is_enabled)
        self.__wordlist_hbox.set_visible(is_enabled)
        self.__wordlist_file_button.set_text(self.__options.iped_wordlist_path or 'Select a wordlist file...')
        self.__wordlist_clear_button.set_sensitive(is_enabled and bool(self.__options.iped_wordlist_path))

        # Option: No content
        self.__no_content_label.set_visible(is_enabled)
        self.__no_content_widget.set_visible(is_enabled)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run action
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        print("IPED report generation requested.")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get processed items from item list, including subitems
    # @param itemlist Case item list
    # @return List of processed items
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_processed_items(self, itemlist):
        processed_items = []

        if not itemlist:
            return processed_items

        case = itemlist[0].case

        for item in itemlist:
            search_path = case.get_path(f"work/{item.uid:04d}/iped/lib/iped-search-app.jar")

            if os.path.exists(search_path):
                processed_items.append(item)

            processed_items.extend(self.__get_processed_items(item.get_children()))

        return processed_items

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_action_switch_modified callback
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_action_switch_modified(self, switch, param):
        self.__options.iped_generate_report = switch.get_active()
        self.update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_wordlist_file button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_wordlist_file(self):
        dialog = Gtk.FileChooserDialog(title='Select wordlist file', action=Gtk.FileChooserAction.OPEN)
        dialog.add_buttons(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OPEN, Gtk.ResponseType.OK)

        filefilter = Gtk.FileFilter()
        filefilter.set_name("Text files")
        filefilter.add_mime_type("text/plain")
        dialog.add_filter(filefilter)

        response = dialog.run()

        if response == Gtk.ResponseType.OK:
            self.__options.iped_wordlist_path = dialog.get_filename()

            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('iped.last_wordlist_file', self.__options.iped_wordlist_path)
            transaction.commit()

        dialog.destroy()

        self.update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_wordlist_clear_file button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_wordlist_clear_file(self):
        self.__options.iped_wordlist_path = None

        transaction = mobius.framework.new_config_transaction()
        mobius.framework.remove_config('iped.last_wordlist_file')
        transaction.commit()

        self.update_options()
