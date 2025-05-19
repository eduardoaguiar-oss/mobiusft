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
import traceback

import mobius
import pymobius
from gi.repository import Gtk
from gi.repository import GtkSource
from gi.repository import Pango

from metadata import *
from navigation_bar import NavigationBar

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
(FILE_FINDER_VIEW_ICON,
 FILE_FINDER_VIEW_PATH,
 FILE_FINDER_VIEW_OBJ
 ) = range(3)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Adhoc expression filter
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class AdhocFilter(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'adhoc'
        self.name = 'Ad-hoc filter'
        self.__code = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set expression
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_expression(self, expr):
        self.__code = compile(expr, 'ad-hoc expression', 'eval')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Test file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def test(self, f):
        l = {
            'name': f.name,
            'short_name': f.short_name,
            'path': f.path,
            'size': f.size,
            'access_time': f.access_time,
            'backup_time': f.backup_time,
            'creation_time': f.creation_time,
            'deletion_time': f.deletion_time,
            'modification_time': f.modification_time,
            'metadata_time': f.metadata_time,
            'extension': f.get_extension(),
            'is_file': f.is_file(),
            'is_folder': f.is_folder(),
            'is_reallocated': f.is_reallocated(),
            'is_deleted': f.is_deleted(),
            'permissions': f.permissions,
            'user_id': f.user_id,
            'group_id': f.group_id,
        }

        return eval(self.__code, {}, l)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: File Finder
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class FileFinderView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__control = control
        self.__mediator = pymobius.mediator.copy()
        self.__category_icons = {}
        self.__filter = None
        self.name = 'File Finder'

        self.__icons = {}
        icon_path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'view_file_finder.png')
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.core.ui.container()
        self.__widget.show()

        vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbox.set_spacing(5)
        vbox.set_visible(True)
        self.__widget.set_content(vbox)

        # navigation bar
        self.__navigation_bar = NavigationBar(control)
        self.__navigation_bar.show()
        vbox.add_child(self.__navigation_bar.get_ui_widget(), mobius.core.ui.box.fill_none)

        # vpaned
        self.__vpaned = Gtk.VPaned()
        self.__vpaned.set_border_width(5)
        self.__vpaned.show()
        vbox.add_child(self.__vpaned, mobius.core.ui.box.fill_with_widget)

        position = mobius.framework.get_config('file-explorer.file-finder-vpaned-position')

        if position:
            self.__vpaned.set_position(position)

        # search area
        search_hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        search_hbox.set_spacing(5)
        search_hbox.set_visible(True)
        self.__vpaned.pack1(search_hbox.get_ui_widget(), True, True)

        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        search_hbox.add_child(sw, mobius.core.ui.box.fill_with_widget)

        language_manager = GtkSource.LanguageManager.get_default()
        language = language_manager.get_language('python')

        sourcebuffer = GtkSource.Buffer()
        sourcebuffer.set_language(language)

        self.__sourceview = GtkSource.View.new_with_buffer(sourcebuffer)
        self.__sourceview.set_show_line_numbers(False)
        self.__sourceview.set_auto_indent(True)
        self.__sourceview.set_indent_width(4)
        self.__sourceview.set_insert_spaces_instead_of_tabs(True)
        self.__sourceview.show()

        font = Pango.FontDescription("Monospace 10")
        self.__sourceview.override_font(font)
        sw.add(self.__sourceview)

        search_vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        search_vbox.set_spacing(5)
        search_vbox.set_visible(True)
        search_hbox.add_child(search_vbox, mobius.core.ui.box.fill_none)

        # combobox
        datastore = Gtk.ListStore.new([str, object])

        self.__filter_combobox = Gtk.ComboBox.new_with_model(datastore)
        self.__populate_combobox()
        self.__filter_combobox.connect('changed', self.__on_combobox_selection_changed)
        self.__filter_combobox.set_active(0)
        self.__filter_combobox.show()
        search_vbox.add_child(self.__filter_combobox, mobius.core.ui.box.fill_with_widget)

        renderer = Gtk.CellRendererText()
        renderer.set_padding(5, 0)
        self.__filter_combobox.pack_start(renderer, True)
        self.__filter_combobox.add_attribute(renderer, 'text', 0)

        self.__search_button = mobius.core.ui.button()
        self.__search_button.set_icon_by_name('edit-find')
        self.__search_button.set_text('_Find')
        self.__search_button.set_visible(True)
        self.__search_button.set_callback('clicked', self.__on_search_button_clicked)
        search_vbox.add_child(self.__search_button, mobius.core.ui.box.fill_with_widget)

        # view
        self.__listview = self.__mediator.call('ui.new-widget', 'tableview')
        self.__listview.set_control(self, 'listview')
        self.__listview.set_multiple_selection(True)
        self.__listview.set_report_id('filelist')
        self.__listview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')

        column = self.__listview.add_column('icon', 'Icon', column_type='icon')

        column = self.__listview.add_column('path', 'Path')
        column.is_sortable = True

        column = self.__listview.add_column('obj', 'object', column_type='object')
        column.is_visible = False

        self.__listview.show()
        self.__vpaned.pack2(self.__listview.get_ui_widget(), True, True)

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'file.svg')
        image = mobius.core.ui.new_icon_by_path(path, mobius.core.ui.icon.size_toolbar)
        icon = image.get_ui_widget().get_pixbuf()
        self.__listview.set_icon('file', icon)

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'folder.svg')
        image = mobius.core.ui.new_icon_by_path(path, mobius.core.ui.icon.size_toolbar)
        icon = image.get_ui_widget().get_pixbuf()
        self.__listview.set_icon('folder', icon)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save current state
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        self.__mediator.clear()
        mobius.framework.set_config('file-explorer.file-finder-vpaned-position', self.__vpaned.get_position())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, folder):

        try:
            self.__folder = folder
            self.__listview.clear()
            self.__navigation_bar.set_folder(folder)
            self.__widget.show_content()

        except Exception as e:
            self.__widget.set_message(str(e))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate combobox
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate_combobox(self):
        model = self.__filter_combobox.get_model()
        model.clear()
        model.append(('Ad-hoc filter', AdhocFilter()))

        # Get filters
        filters = [(r.value.name, r.value) for r in mobius.core.get_resources('file.filter')]

        for name, f in sorted(filters):
            model.append((name, f))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle listview selection changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def listview_on_selection_changed(self, selection_list):
        entries = []

        for row_id, row_data in selection_list:
            entries.append(row_data[2])

        self.__control.on_entry_selection_changed(entries)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle selection changed event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_selection_changed(self, selection, *args):
        entries = []
        model, pathlist = selection.get_selected_rows()

        for treepath in pathlist:
            e = model[treepath][FILE_FINDER_VIEW_OBJ]
            entries.append(e)

        self.__control.on_entry_selection_changed(entries)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle row activated event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_row_activated(self, widget, row, col, *args):
        model = widget.get_model()
        entry = model[row][FILE_FINDER_VIEW_OBJ]

        if entry.is_folder():
            self.__control.on_folder_activated(entry)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle filter combobox selection changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_combobox_selection_changed(self, combobox, *args):
        combobox_iter = combobox.get_active_iter()

        if combobox_iter:
            model = combobox.get_model()
            self.__filter = model.get_value(combobox_iter, 1)

            is_adhoc = self.__filter.id == 'adhoc'
            self.__sourceview.set_sensitive(is_adhoc)

        else:
            self.__filter = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle search button clicked event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_search_button_clicked(self):
        try:

            # set expression for adhoc filter
            if self.__filter.id == 'adhoc':
                text_buffer = self.__sourceview.get_buffer()
                start, end = text_buffer.get_bounds()
                text = text_buffer.get_text(start, end, True).strip()
                self.__filter.set_expression(text)

            # search through folder and sub-folders
            self.__search()

        except Exception as e:
            mobius.core.logf(f'ERR {str(e)}\n{traceback.format_exc()}')

            dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_error)
            dialog.text = f'Error: {str(e)}'
            dialog.add_button(mobius.core.ui.message_dialog.BUTTON_OK)
            rc = dialog.run()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Search recursively files matching criteria
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __search(self):

        # clear old results
        self.__listview.clear()
        self.__listview.set_report_id(f"filelist-{self.__filter.id}")
        self.__listview.set_report_name(f'{self.__filter.name} files')

        # search
        for entry in self.__walk(self.__folder):
            if self.__filter.test(entry):
                icon_id = 'file' if entry.is_file() else 'folder'
                self.__listview.add_row((icon_id, entry.path, entry))
                mobius.core.ui.flush()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Walk through folder entries recursively
    # @param folder Root folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __walk(self, folder):

        for child in folder.get_children():
            yield child

            if child.is_folder():
                for e in self.__walk(child):
                    yield e
