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
import shutil
import subprocess
import threading

import mobius
import mobius.core.io
import pymobius
from gi.repository import Gdk
from gi.repository import Gtk

from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief NoContent list widget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class NoContentListWidget(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__items = []  # internal list of strings

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
        return self.__items.copy()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Replace the current list with new items
    # @param items List of strings to set
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_items(self, items):
        self.clear()
        for item in items:
            self.__items.append(item)
            self.__add_row(item)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Clear the list
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def clear(self):
        self.__items.clear()
        for row in self.__listbox.get_children():
            self.__listbox.remove(row)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Add item to the list
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_add_clicked(self, widget):
        text = self.__entry.get_text().strip()
        if not text:
            return

        if text in self.__items:
            # Optional: prevent duplicates
            self.__entry.set_text("")
            return

        self.__items.append(text)
        self.__add_row(text)
        self.__entry.set_text("")
        self.__entry.grab_focus()

        # Save the updated list to config
        transaction = mobius.framework.new_config_transaction()
        mobius.framework.set_config('iped.last_no_content_items', self.__items)
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
        if text in self.__items:
            self.__items.remove(text)
        self.__listbox.remove(row)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle key press events for the listbox
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_key_press(self, widget, event):
        if event.keyval == Gdk.KEY_Delete:
            row = self.__listbox.get_selected_row()
            if row:
                # Find the text from the label
                hbox = row.get_child()
                for child in hbox.get_children():
                    if isinstance(child, Gtk.Label):
                        text = child.get_text()
                        if text in self.__items:
                            self.__items.remove(text)
                        break
                self.__listbox.remove(row)
                return True
        return False


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: Report
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ReportView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__mediator = pymobius.mediator.copy()
        self.__control = control
        self.__iped_path = None
        self.__output_folder = None
        self.__asap_file = None
        self.__wordlist_file = None
        self.__is_running = False
        self.__itemlist = None
        self.__processed_items = []
        self.__laudo = None
        self.name = "Generate Report"

        path = self.__mediator.call("extension.get-resource-path", EXTENSION_ID, "report.png")
        self.icon_data = open(path, "rb").read()

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
        grid.set_column_homogeneous(False)
        grid.show()
        vbox.add_child(grid, mobius.core.ui.box.fill_with_widget)

        # Output folder
        label = mobius.core.ui.label()
        label.set_markup('<b>Output folder:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 0, 1, 1)

        # Output folder chooser button
        self.__output_folder_button = mobius.core.ui.button()
        self.__output_folder_button.set_icon_by_name('folder')
        self.__output_folder_button.set_text('Select output folder...')
        self.__output_folder_button.set_visible(True)
        self.__output_folder_button.set_callback('clicked', self.__on_click_output_folder)
        self.__output_folder_button.get_ui_widget().set_hexpand(True)
        grid.attach(self.__output_folder_button.get_ui_widget(), 1, 0, 2, 1)

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

        # Wordlist file path
        label = mobius.core.ui.label()
        label.set_markup('<b>Wordlist (optional):</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 2, 1, 1)

        wordlist_hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        wordlist_hbox.set_spacing(5)
        wordlist_hbox.set_visible(True)
        grid.attach(wordlist_hbox.get_ui_widget(), 1, 2, 2, 1)

        # Wordlist file chooser button
        self.__wordlist_file_button = mobius.core.ui.button()
        self.__wordlist_file_button.set_icon_by_name('folder')
        self.__wordlist_file_button.set_text('Select a wordlist file...')
        self.__wordlist_file_button.set_visible(True)
        self.__wordlist_file_button.set_callback('clicked', self.__on_click_wordlist_file)
        wordlist_hbox.add_child(self.__wordlist_file_button, mobius.core.ui.box.fill_with_widget)

        # Wordlist clear button
        self.__wordlist_clear_button = mobius.core.ui.button()
        self.__wordlist_clear_button.set_icon_by_name('edit-clear')
        self.__wordlist_clear_button.set_visible(True)
        self.__wordlist_clear_button.set_sensitive(False)
        self.__wordlist_clear_button.set_callback('clicked', self.__on_click_wordlist_clear_file)
        wordlist_hbox.add_child(self.__wordlist_clear_button, mobius.core.ui.box.fill_none)

        # No content label
        self.__no_content_label = mobius.core.ui.label()
        self.__no_content_label.set_markup('\n<b>No content options:</b>')
        self.__no_content_label.set_halign(mobius.core.ui.label.align_right)
        self.__no_content_label.set_valign(mobius.core.ui.label.align_top)
        self.__no_content_label.set_visible(True)
        grid.attach(self.__no_content_label.get_ui_widget(), 0, 3, 1, 1)

        # No content widget
        self.__no_content_widget = NoContentListWidget()
        self.__no_content_widget.set_visible(True)
        grid.attach(self.__no_content_widget.get_ui_widget(), 1, 3, 2, 1)

        # Buttons
        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_visible(True)
        hbox.set_spacing(10)
        hbox.add_filler()
        vbox.add_child(hbox, mobius.core.ui.box.fill_none)

        self.__generate_report_button = mobius.core.ui.button()
        self.__generate_report_button.set_icon_by_name("system-run")
        self.__generate_report_button.set_text("_Execute")
        self.__generate_report_button.set_sensitive(False)
        self.__generate_report_button.set_visible(True)
        self.__generate_report_button.set_callback("clicked", self.__on_generate_report)
        hbox.add_child(self.__generate_report_button, mobius.core.ui.box.fill_none)

        self.__open_report_button = mobius.core.ui.button()
        self.__open_report_button.set_icon_by_name("folder")
        self.__open_report_button.set_text("_Open report")
        self.__open_report_button.set_sensitive(False)
        self.__open_report_button.set_visible(True)
        self.__open_report_button.set_callback("clicked", self.__on_click_open_report_button)
        hbox.add_child(self.__open_report_button, mobius.core.ui.box.fill_none)
       
        # Set panel state
        last_output_folder = mobius.framework.get_config('last_report_folder')
        if last_output_folder:
            self.__output_folder = last_output_folder

        last_asap_file = mobius.framework.get_config('last_asap_file')
        if last_asap_file:
            self.__asap_file = last_asap_file

        self.__update_options()

        # Subscribe to config events
        self.__event_uid1 = mobius.core.subscribe("config-set", self.__on_config_set)
        self.__event_uid2 = mobius.core.subscribe("config-remove", self.__on_config_remove)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save current state
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        mobius.core.unsubscribe(self.__event_uid1)
        mobius.core.unsubscribe(self.__event_uid2)
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
        self.__itemlist = itemlist
        self.__processed_items = []
        self.__laudo = None

        if self.__itemlist:
            self.__set_selected_items(self.__itemlist)

        else:
            self.__widget.set_message("Select item(s) to generate report")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set IPED path
    # @param path Full path
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_iped_path(self, path):
        self.__iped_path = path

        if path:
            self.__widget.show_content()
        else:
            self.__widget.set_message(
                "You must set IPED path before generating reports"
            )

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set selected items
    # @param itemlist Case item list
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __set_selected_items(self, itemlist):
        self.__processed_items = self.__get_processed_items(itemlist)

        if self.__processed_items:
            self.__widget.show_content()
            self.__update_options()
        else:
            self.__widget.set_message("No processed item(s) selected")

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

        if self.__wordlist_file:
            self.__wordlist_file_button.set_text(self.__wordlist_file)
            self.__wordlist_clear_button.set_sensitive(True)
        else:
            self.__wordlist_file_button.set_text('Select a wordlist file...')
            self.__wordlist_clear_button.set_sensitive(False)

        can_generate = not self.__is_running and bool(self.__output_folder) and bool(self.__itemlist)
        self.__generate_report_button.set_sensitive(can_generate)

        can_open = self.__has_report()
        self.__open_report_button.set_sensitive(can_open)

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
            mobius.framework.set_config('last_report_folder', self.__output_folder)
            transaction.commit()

        dialog.destroy()

        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_asap_file button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_asap_file(self):
        dialog = Gtk.FileChooserDialog(title='Select .ASAP file', action=Gtk.FileChooserAction.OPEN)
        dialog.add_buttons(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OPEN, Gtk.ResponseType.OK)

        if self.__asap_file:
            dialog.set_current_folder(self.__asap_file)

        filefilter = Gtk.FileFilter()
        filefilter.set_name("ASAP files (*.asap)")
        filefilter.add_pattern("*.asap")
        dialog.add_filter(filefilter)

        response = dialog.run()

        if response == Gtk.ResponseType.OK:
            self.__asap_file = dialog.get_filename()
        
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
        self.__update_options()

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
            self.__wordlist_file = dialog.get_filename()

            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('last_wordlist_file', self.__wordlist_file)
            transaction.commit()

        dialog.destroy()

        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_wordlist_clear_file button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_wordlist_clear_file(self):
        self.__wordlist_file = None
        self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_config_set event handler
    # @param name Config name
    # @param value Config value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_config_set(self, name, value):
        if name == 'last_report_folder':
            self.__output_folder = value
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
            self.__output_folder = None
            self.__update_options()

        elif name == 'last_asap_file':
            self.__asap_file = None
            self.__update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get processed items from item list, including subitems
    # @param itemlist Case item list
    # @return List of processed items
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_processed_items(self, itemlist):
        processed_items = []

        for item in itemlist:
            if self.__is_processed_item(item):
                processed_items.append(item)

            processed_items.extend(self.__get_processed_items(item.get_children()))

        return processed_items

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Check if item is already processed by IPED
    # @param item Case item
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __is_processed_item(self, item):
        case = item.case
        search_path = case.get_path(f"work/{item.uid:04d}/iped/lib/iped-search-app.jar")

        return os.path.exists(search_path)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Check if report already exists in the output folder
    # @return True if report exists, False otherwise
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __has_report(self):
        if not self.__output_folder:
            return False
        
        search_path = os.path.join(self.__output_folder, "iped", "lib", "iped-search-app.jar")
        return os.path.exists(search_path)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_generate_report event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_generate_report(self):

        # check if there is an older report
        if self.__has_report():
            dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
            dialog.text = f"Report found on '{self.__output_folder}'. Do you want to overwrite?"
            dialog.add_button(mobius.core.ui.message_dialog.button_yes)
            dialog.add_button(mobius.core.ui.message_dialog.button_no)
            dialog.set_default_response(mobius.core.ui.message_dialog.button_no)
            rc = dialog.run()

            if rc != mobius.core.ui.message_dialog.button_yes:
                return

        option = pymobius.Data()
        option.report_path = self.__output_folder
        option.report_log_path = self.__output_folder + "/report.log" # @todo report.log path
        option.case = self.__itemlist[0].case
        option.itemlist = self.__itemlist[:]
        option.processed_items = self.__processed_items[:]
        option.control = self.__control
        option.iped_path = self.__iped_path
        option.asap_file = self.__asap_file
        option.wordlist_file = self.__wordlist_file
        option.no_content_items = self.__no_content_widget.get_items()
        option.xmx = mobius.framework.get_config("iped.xmx") or 8

        # create thread
        t = threading.Thread(target=self.__generate_report_thread, args=(option,), daemon=True)
        t.start()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Report generation thread
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_report_thread(self, option):
        guard = mobius.core.thread_guard()
        connection = option.case.new_connection()

        # remove previous report folder and .log
        if os.path.exists(option.report_log_path):
            os.remove(option.report_log_path)

        if os.path.exists(option.report_path):
            option.control.set_status("Removing old report...")
            shutil.rmtree(option.report_path)

        os.makedirs(option.report_path)

        # build command line
        # create one xxxx.iped file for each 'bookmarks.iped' file, because each file name must have a unique name
        cmd = [
            'java', '-jar', f'{option.iped_path}/iped.jar',
            '-log', option.report_log_path,
            f'-Xmx{option.xmx}g',
            f'-Xms{option.xmx}g',
            '-o', option.report_path
        ]

        for item in option.processed_items:
            case = item.case
            indexer_path = case.get_path(os.path.join("work", f"{item.uid:04d}", "iped"))
            bookmarks_default_path = os.path.join(indexer_path, "bookmarks.iped")

            if os.path.exists(bookmarks_default_path):
                bookmarks_path = os.path.join(indexer_path, f"{item.uid:04d}.iped")
                shutil.copyfile(bookmarks_default_path, bookmarks_path)
                cmd += ['-d', bookmarks_path]

        # add .asap file, if available
        if option.asap_file:
            cmd += ['-asap', option.asap_file]

        # add wordlist file, if available
        if option.wordlist_file:
            cmd += ['-keywordlist', option.wordlist_file]

        # add -nocontent for each no content item
        for arg in option.no_content_items:
            cmd += ['-nocontent', arg]

        # run report
        mobius.core.logf("INF " + ' '.join(cmd))
        option.control.set_status("Running IPED report...")

        rc = subprocess.call(cmd)

        # final message
        option.control.set_status(f"Report generated. RC={rc}.")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_open_report_button event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_open_report_button(self):
        if not self.__output_folder or not os.path.exists(self.__output_folder):
            return
        
        iped_search_app_path = os.path.join(self.__output_folder, "iped", "lib", "iped-search-app.jar")
        if not os.path.exists(iped_search_app_path):
            return
        
        subprocess.call(['java', '-jar', iped_search_app_path])
