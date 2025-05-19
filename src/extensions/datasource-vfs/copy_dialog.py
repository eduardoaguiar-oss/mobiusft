# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
import threading

import mobius
from gi.repository import GLib
from gi.repository import Gdk
from gi.repository import Gtk


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Dialog: CopyDialog
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CopyDialog(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Init widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, reader, writer, title='Copying data...'):
        self.__reader = reader
        self.__writer = writer

        # dialog
        self.__dialog = Gtk.Dialog(title=title, modal=False)

        self.__cancel_button = self.__dialog.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        self.__ok_button = self.__dialog.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        self.__ok_button.set_sensitive(False)
        self.__dialog.set_position(Gtk.WindowPosition.CENTER)
        self.__dialog.set_type_hint(Gdk.WindowTypeHint.DIALOG)
        self.__dialog.set_border_width(5)
        self.__dialog.set_default_size(400, 200)

        # grid
        grid = Gtk.Grid.new()
        grid.set_row_spacing(5)
        grid.set_column_spacing(5)
        grid.show()
        self.__dialog.vbox.pack_start(grid, True, True, 0)

        label = mobius.core.ui.label()
        label.set_markup('<b>Total bytes:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 0, 1, 1)

        self.__total_bytes_label = mobius.core.ui.label(f'{reader.size:15d}')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(self.__total_bytes_label.get_ui_widget(), 1, 0, 1, 1)

        label = mobius.core.ui.label()
        label.set_markup('<b>Bytes copied:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 1, 1, 1)

        self.__bytes_copied_label = mobius.core.ui.label(f'{0:15d}')
        self.__bytes_copied_label.set_halign(mobius.core.ui.label.align_right)
        self.__bytes_copied_label.set_visible(True)
        grid.attach(self.__bytes_copied_label.get_ui_widget(), 1, 1, 1, 1)

        label = mobius.core.ui.label()
        label.set_markup('<b>Bytes to go:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 2, 1, 1)

        self.__bytes_to_go_label = mobius.core.ui.label(f'{reader.size:15d}')
        self.__bytes_to_go_label.set_halign(mobius.core.ui.label.align_right)
        self.__bytes_to_go_label.set_visible(True)
        grid.attach(self.__bytes_to_go_label.get_ui_widget(), 1, 2, 1, 1)

        # progress bar
        self.__progress_bar = Gtk.ProgressBar()
        self.__progress_bar.set_hexpand(True)
        self.__progress_bar.show()
        grid.attach(self.__progress_bar, 0, 4, 3, 1)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run dialog
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__thread = threading.Thread(target=self.__export_thread, args=(self.__reader, self.__writer))
        self.__stop_event = threading.Event()
        self.__thread.start()

        rc = self.__dialog.run()

        if rc == Gtk.ResponseType.CANCEL:
            self.__stop_event.set()

        self.__dialog.destroy()

        return rc

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Export thread
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __export_thread(self, reader, writer):
        guard = mobius.core.thread_guard()

        BLOCK_SIZE = 1048576
        pct = 0.0
        size = 0
        total_size = reader.size

        # read sectors from 'reader' and write them into 'writer'
        data = reader.read(BLOCK_SIZE)

        while not self.__stop_event.is_set() and data:
            writer.write(data)
            size += len(data)
            newpct = float(size) / total_size

            GLib.idle_add(self.__bytes_copied_label.set_text, f'{size:15d}')
            GLib.idle_add(self.__bytes_to_go_label.set_text, f'{total_size - size:15d}')

            if newpct - pct > 0.01:
                pct = newpct
                GLib.idle_add(self.__progress_bar.set_fraction, pct)
                GLib.idle_add(self.__progress_bar.set_text, f'{pct * 100.0:.2f} %')

            data = reader.read(BLOCK_SIZE)

        # finish
        writer.flush()
        GLib.idle_add(self.__progress_bar.set_fraction, 1.0)
        GLib.idle_add(self.__ok_button.set_sensitive, True)
        GLib.idle_add(self.__cancel_button.set_sensitive, False)
