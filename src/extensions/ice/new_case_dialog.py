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

import mobius
import mobius.framework
from gi.repository import Gdk
from gi.repository import Gtk


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief New case dialog
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class NewCaseDialog(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__widget = Gtk.Dialog(title='New Case', modal=True)
        self.__widget.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        self.__widget.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        self.__widget.set_position(Gtk.WindowPosition.CENTER)
        self.__widget.set_default_size(500, 200)
        self.__widget.set_type_hint(Gdk.WindowTypeHint.DIALOG)
        self.__widget.set_border_width(5)
        self.case_id = None
        self.case_name = None
        self.case_folder = None

        # grid
        grid = Gtk.Grid.new()
        grid.set_border_width(5)
        grid.set_column_spacing(5)
        grid.set_row_spacing(10)
        grid.show()
        self.__widget.vbox.pack_start(grid, True, True, 0)

        label = mobius.core.ui.label('Case folder')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 0, 1, 1)

        case_repository_folder = mobius.framework.get_config('ice.case-repository-folder') or os.getcwd()

        self.__case_folder_button = Gtk.FileChooserButton(title="Select folder")
        self.__case_folder_button.set_hexpand(True)
        self.__case_folder_button.set_action(Gtk.FileChooserAction.SELECT_FOLDER)
        self.__case_folder_button.set_current_folder(case_repository_folder)
        self.__case_folder_button.show()
        grid.attach(self.__case_folder_button, 1, 0, 2, 1)

        label = mobius.core.ui.label('ID')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 1, 1, 1)

        self.__entry_id = Gtk.Entry()
        self.__entry_id.set_hexpand(True)
        self.__entry_id.set_text("untitled")
        self.__entry_id.show()
        grid.attach(self.__entry_id, 1, 1, 2, 1)

        label = mobius.core.ui.label('Name')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 2, 1, 1)

        self.__entry_name = Gtk.Entry()
        self.__entry_name.set_text("Untitled Case")
        self.__entry_name.show()
        grid.attach(self.__entry_name, 1, 2, 2, 1)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run dialog
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        rc = None

        while not rc:
            rc = self.__widget.run()

            if rc == Gtk.ResponseType.OK:
                self.case_id = self.__entry_id.get_text().strip()
                self.case_name = self.__entry_name.get_text().strip()
                self.case_folder = self.__case_folder_button.get_filename()

                mobius.framework.set_config('ice.case-repository-folder', os.path.dirname(self.case_folder))

        return rc

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Destroy dialog
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def destroy(self):
        self.__widget.destroy()
