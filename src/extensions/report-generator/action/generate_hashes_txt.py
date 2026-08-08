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
import shutil

from gi.repository import Gtk
from gi.repository import Gdk

import os
import mobius

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief hashes.txt file generator action
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class GenerateHashesTxtAction(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize options
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, options):
        options.generate_hashes_txt = True

        self.__options = options
        self.__hashes_txt_value = None
        self.__hashes_txt_path = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Build options widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_widget(self, grid, row):

        self.__placeholder = mobius.core.ui.label()
        self.__placeholder.set_visible(True)
        grid.attach(self.__placeholder.get_ui_widget(), 0, row, 3, 1)

        # Main option
        self.__action_label = mobius.core.ui.label()
        self.__action_label.set_markup('<b>Generate hashes.txt file</b>')
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

        # Media option: Update hashes.txt
        self.__update_label = mobius.core.ui.label()
        self.__update_label.set_markup('<b>Update hashes.txt file:</b>')
        self.__update_label.set_halign(mobius.core.ui.label.align_right)
        self.__update_label.set_visible(True)
        grid.attach(self.__update_label.get_ui_widget(), 0, row + 2, 1, 1)

        self.__update_hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        self.__update_hbox.set_spacing(5)
        self.__update_hbox.set_visible(True)
        grid.attach(self.__update_hbox.get_ui_widget(), 1, row + 2, 2, 1)

        self.__update_switch = Gtk.Switch.new()
        self.__update_switch.set_visible(True)
        self.__update_switch.set_active(False)
        self.__update_hbox.add_child(self.__update_switch, mobius.core.ui.box.fill_none)
        self.__update_hbox.add_filler()

        # Hashes.txt value and copy button
        self.__value_label = mobius.core.ui.label()
        self.__value_label.set_markup("<b>Hashes.txt (SHA2-256):</b>")
        self.__value_label.set_halign(mobius.core.ui.label.align_right)
        self.__value_label.set_visible(True)
        grid.attach(self.__value_label.get_ui_widget(), 0, row + 3, 1, 1)

        self.__value_hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        self.__value_hbox.set_spacing(5)
        self.__value_hbox.set_visible(True)
        self.__value_hbox.get_ui_widget().set_hexpand(True)
        grid.attach(self.__value_hbox.get_ui_widget(), 1, row + 3, 2, 1)

        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.IN)
        frame.show()
        self.__value_hbox.add_child(frame, mobius.core.ui.box.fill_with_widget)

        self.__hash_label = mobius.core.ui.label()
        self.__hash_label.set_halign(mobius.core.ui.label.align_left)
        self.__hash_label.set_selectable(True)
        self.__hash_label.show()
        frame.add(self.__hash_label.get_ui_widget())

        self.__copy_button = mobius.core.ui.button()
        self.__copy_button.set_icon_by_name('edit-copy')
        self.__copy_button.set_visible(True)
        self.__copy_button.set_sensitive(False)
        self.__copy_button.set_callback('clicked', self.__on_click_hashes_txt_copy)
        self.__value_hbox.add_child(self.__copy_button, mobius.core.ui.box.fill_none)

        return row + 4

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set output folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_output_folder(self, output_folder):
        self.__options.output_folder = output_folder
        self.update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update options widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def update_options(self):
        is_available = self.__options.template_type == 'media' and bool(self.__options.output_folder)
        is_enabled = is_available and self.__action_switch.get_active()

        # Show action, if action can run
        self.__placeholder.set_visible(is_available)
        self.__action_label.set_visible(is_available)
        self.__action_hbox.set_visible(is_available)

        # Option: Update hashes.txt
        if self.__options.output_folder:
            hashes_txt_path = os.path.join(self.__options.output_folder, "hashes.txt")
            has_hashes_txt = os.path.exists(hashes_txt_path) 
        else:
            has_hashes_txt = False

        self.__update_label.set_visible(is_enabled)
        self.__update_hbox.set_visible(is_enabled)
        return
    
        can_copy_hash_value = bool(self.__options.hashes_txt_value)
        self.__copy_button.set_sensitive(can_copy_hash_value)

        # Calculate hashes.txt value if it exists
        hashes_txt_path = os.path.join(self.__options.output_folder, "hashes.txt")

        if hashes_txt_path != self.__hashes_txt_path:
            self.__hashes_txt_path = hashes_txt_path

            if os.path.exists(hashes_txt_path):
                self.__hash_label.set_text(self.__get_hash(hashes_txt_path))
                has_hashes_txt = True
            else:
                self.__hash_label.set_text('')
                has_hashes_txt = False

            # Update visibility of the hashes.txt value and copy button
            self.__value_label.set_visible(has_hashes_txt)
            self.__value_hbox.set_visible(has_hashes_txt)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate hashes.txt
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__set_status("Generating hashes.txt file...")
        hashes_txt_path = os.path.join(self.__options.output_folder, "hashes.txt")

        # If model.update_hashes_txt is True, read hashes from current hashes.txt
        cached_hashes = {}
        hashes_txt_mtime = None

        if self.__options.update_hashes_txt and os.path.exists(hashes_txt_path):
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
        pos = len(self.__output_folder) + 1

        for root, dirs, files in os.walk(self.__output_folder, topdown=False):
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
        self.__options.hashes_txt_value = self.__get_hash(hashes_txt_path)

        # Write hashes_txt value back to .asap file if available
        if self.__options.asap_path:
            self.__update_asap_file(self.__options.asap_path, self.__options.hashes_txt_value)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_action_switch_modified callback
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_action_switch_modified(self, switch, param):
        self.__options.generate_hashes_txt = switch.get_active()
        self.update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_action_switch_modified callback for generate_iped switch
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_action_switch_modified(self, switch, param):
        self.__options.generate_hashes_txt = switch.get_active()
        self.update_options()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_click_hashes_txt_copy button clicked
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_click_hashes_txt_copy(self):
        if self.__options.hashes_txt_value:
            clipboard = Gtk.Clipboard.get(Gdk.SELECTION_CLIPBOARD)
            clipboard.set_text(self.__options.hashes_txt_value, -1)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update hashes.txt label
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __update_hashes_txt_label(self):
        self.__hash_label.set_text('')
        self.__copy_button.set_sensitive(False)

        if not self.__options.output_folder:
            return
        
        hashes_txt_path = os.path.join(self.__options.output_folder, "hashes.txt")
        if not os.path.exists(hashes_txt_path):
            return
                
        # self.__set_status("Calculating <b>hashes.txt</b> hash...")
     
        self.__hash_value = self.__get_hash(hashes_txt_path)
        self.__hash_label.set_text(self.__hash_value)

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
