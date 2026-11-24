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
from datetime import datetime
import threading

import mobius
import mobius.core.crypt
import mobius.core.kff
import mobius.core.io
import pymobius
from gi.repository import GLib
from gi.repository import Gtk

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Extension metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
EXTENSION_ID = "kff-manager"
EXTENSION_NAME = "KFF Manager"
EXTENSION_AUTHOR = "Eduardo Aguiar"
EXTENSION_VERSION = "2.0"
EXTENSION_DESCRIPTION = "Known File Filter (KFF) Manager"

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief KFF view class
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
HASHSET_ID, HASHSET_DESCRIPTION, HASHSET_IS_ALERT, HASHSET_SIZE, HASHSET_OBJ = range(5)

class KFFView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__data = []

        self.name = f"{EXTENSION_NAME} v{EXTENSION_VERSION} (KFF Manager)"
        self.icon_path = pymobius.mediator.call("extension.get-icon-path", EXTENSION_ID)
        self.icon_data = open(self.icon_path, "rb").read()

        # build widget
        self.__widget = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        self.__widget.set_border_width(5)
        self.__widget.set_visible(True)

        # toolbar
        toolbar = Gtk.Toolbar()
        toolbar.show()
        self.__widget.add_child(toolbar, mobius.core.ui.box.fill_none)

        self.__import_hashlist_toolitem = Gtk.ToolButton.new()
        self.__import_hashlist_toolitem.set_icon_name("document-import")
        self.__import_hashlist_toolitem.connect("clicked", self.__on_import)
        self.__import_hashlist_toolitem.show()
        self.__import_hashlist_toolitem.set_tooltip_text("Import data into KFF")
        self.__import_hashlist_toolitem.set_sensitive(True)
        toolbar.insert(self.__import_hashlist_toolitem, -1)

        self.__export_hashlist_toolitem = Gtk.ToolButton.new()
        self.__export_hashlist_toolitem.set_icon_name("document-export")
        self.__export_hashlist_toolitem.connect("clicked", self.__on_export)
        self.__export_hashlist_toolitem.show()
        self.__export_hashlist_toolitem.set_tooltip_text("Export data from KFF")
        self.__export_hashlist_toolitem.set_sensitive(True)
        toolbar.insert(self.__export_hashlist_toolitem, -1)

        # status listview
        self.__status_listview = self.__mediator.call("ui.new-widget", "tableview")
        self.__status_listview.set_report_id("kff.status")
        self.__status_listview.set_report_name("KFF Status")
        self.__status_listview.set_report_app(f"{EXTENSION_NAME} v{EXTENSION_VERSION}")
        self.__status_listview.show()
        self.__status_listview.set_drag_and_drop(False)
        self.__status_listview.set_control(self)

        self.__status_listview.add_column("name")
        self.__status_listview.add_column("count", "Count", column_type="int")

        self.__widget.add_child(
            self.__status_listview.get_ui_widget(), mobius.core.ui.box.fill_with_widget
        )

        # status bar
        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.IN)
        frame.show()
        self.__widget.add_child(frame, mobius.core.ui.box.fill_none)

        self.__status_label = mobius.core.ui.label()
        self.__status_label.set_selectable(True)
        self.__status_label.set_halign(mobius.core.ui.label.align_left)
        self.__status_label.set_elide_mode(mobius.core.ui.label.elide_middle)
        self.__status_label.set_visible(True)
        frame.add(self.__status_label.get_ui_widget())

        # populate listview
        self.__populate()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Reload and populate listview
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_reload_data(self, tableview):
        self.__populate()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate listview
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate(self):
        self.__status_listview.clear()
        self.__status_listview.add_row(("Files", mobius.core.kff.get_file_count()))
        self.__status_listview.add_row(
            ("Alert files", mobius.core.kff.get_alert_count())
        )
        self.__status_listview.add_row(("Hashes", mobius.core.kff.get_hash_count()))
        self.__status_listview.add_row(
            ("Thumbnails", mobius.core.kff.get_thumbnail_count())
        )

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set current status
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __set_status(self, status):
        now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        GLib.idle_add(self.__status_label.set_markup, f"<b>{now}</b> {status}")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle close button
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_extension_close(self, widget, *args):
        self.__mediator.call("ui.working-area.close", self.working_area.id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Stop widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_widget_stopped(self):
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle import
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_import(self, widget, *args):

        # build dialog
        fs = Gtk.FileChooserDialog(title=f'Importing KFF data')
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)

        # add import handlers
        filefilter = Gtk.FileFilter()
        filefilter.handler = mobius.core.kff.populate_from_database
        filefilter.extension = "db"
        filefilter.set_name("Populate from database file (*.db)")
        filefilter.add_pattern("*.db")
        fs.add_filter(filefilter)

        filefilter = Gtk.FileFilter()
        filefilter.handler = mobius.core.kff.populate_from_iped_hashes
        filefilter.extension = "db"
        filefilter.set_name("Populate from IPED hashes file (*.db)")
        filefilter.add_pattern("*.db")
        fs.add_filter(filefilter)

        filefilter = Gtk.FileFilter()
        filefilter.handler = mobius.core.kff.merge_collected_data
        filefilter.extension = "db"
        filefilter.set_name("Merge collected data file (*.db)")
        filefilter.add_pattern("*.db")
        fs.add_filter(filefilter)

        filefilter = Gtk.FileFilter()
        filefilter.handler = mobius.core.kff.merge_file_sizes
        filefilter.extension = "txt"
        filefilter.set_name("Merge file sizes file (*.txt)")
        filefilter.add_pattern("*.txt")
        fs.add_filter(filefilter)

        filefilter = Gtk.FileFilter()
        filefilter.handler = mobius.core.kff.merge_passwords
        filefilter.extension = "txt"
        filefilter.set_name("Merge passwords file (*.txt)")
        filefilter.add_pattern("*.txt")
        fs.add_filter(filefilter)

        # run dialog
        rc = fs.run()
        path = fs.get_filename()
        folder = fs.get_current_folder()

        filefilter = fs.get_filter()
        handler = filefilter.handler
        fileext = filefilter.extension

        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        # import file
        self.__run_thread(handler, path)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle export
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export(self, widget, *args):

        # build dialog
        fs = Gtk.FileChooserDialog(
            title=f"Exporting KFF data", action=Gtk.FileChooserAction.SAVE
        )
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        fs.set_do_overwrite_confirmation(True)
        fs.set_current_name("kff.db")

        # set current folder
        last_folder = mobius.framework.get_config("general.save-folder")
        if last_folder:
            fs.set_current_folder(last_folder)

        # add export handlers
        filefilter = Gtk.FileFilter()
        filefilter.handler = mobius.core.kff.export_collected_data
        filefilter.extension = "db"
        filefilter.set_name("Collected data (*.db)")
        filefilter.add_pattern("*.db")
        fs.add_filter(filefilter)

        filefilter = Gtk.FileFilter()
        filefilter.handler = mobius.core.kff.export_database
        filefilter.extension = "db"
        filefilter.set_name("Full KFF database file (*.db)")
        filefilter.add_pattern("*.db")
        fs.add_filter(filefilter)

        # run dialog
        rc = fs.run()
        path = fs.get_filename()
        folder = fs.get_current_folder()

        filefilter = fs.get_filter()
        handler = filefilter.handler
        fileext = filefilter.extension

        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        mobius.framework.set_config("general.save-folder", folder)

        # add extension to name
        if not path.endswith("." + fileext):
            path = path + "." + fileext

        # export
        self.__run_thread(handler, path)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run export in a separate thread
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __run_thread(self, handler, path):
        try:
            thread = threading.Thread(target=handler, args=(path, self.__set_status))
            thread.start()
        except Exception as e:
            self.__set_status(f"Error: {e}")


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    icon = pymobius.mediator.call("extension.get-icon-path", EXTENSION_ID)
    mobius.core.add_resource(
        "menu.tools." + EXTENSION_ID,
        "Menu Tools: KFF Manager",
        (icon, EXTENSION_NAME, on_activate),
    )


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource("menu.tools." + EXTENSION_ID)
    pymobius.mediator.call("ui.working-area.del", EXTENSION_ID)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief on_activate
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def on_activate(item_id):
    widget = KFFView()
    icon_path = pymobius.mediator.call("extension.get-icon-path", EXTENSION_ID)

    working_area = pymobius.mediator.call("ui.working-area.new", EXTENSION_ID)
    working_area.set_default_size(500, 700)
    working_area.set_title(EXTENSION_NAME)
    working_area.set_icon(icon_path)
    working_area.set_widget(widget.get_ui_widget())
    working_area.show()
