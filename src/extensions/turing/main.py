# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023 Eduardo Aguiar
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
import json

import mobius
import mobius.core.turing
import pymobius
from gi.repository import Gtk

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Extension metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
EXTENSION_ID = 'turing'
EXTENSION_NAME = 'Turing'
EXTENSION_AUTHOR = 'Eduardo Aguiar'
EXTENSION_VERSION = '1.15'
EXTENSION_DESCRIPTION = 'Turing cryptographic extension'

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
JSON_VERSION = 1


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: database view
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class DatabaseView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__data = None
        self.__turing = mobius.core.turing.turing()

        self.name = 'Turing Database'
        icon_path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'database.png')
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.ui.container()
        self.__widget.set_visible(True)

        # vbox
        vbox = mobius.ui.box(mobius.ui.box.orientation_vertical)
        vbox.set_border_width(5)
        vbox.set_spacing(10)
        vbox.set_visible(True)
        self.__widget.set_content(vbox)

        # tableview
        self.__tableview = self.__mediator.call('ui.new-widget', 'tableview')
        self.__tableview.set_report_id('turing.hashes')
        self.__tableview.set_report_name('Turing: hash database')
        self.__tableview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__tableview.set_control(self, 'main_tableview')
        self.__tableview.show()
        vbox.add_child(self.__tableview.get_ui_widget(), mobius.ui.box.fill_with_widget)

        column = self.__tableview.add_column('type', 'Type')
        column.is_sortable = True

        column = self.__tableview.add_column('value', 'Value')
        column.is_sortable = True

        self.__tableview.add_column('password', 'Password')

        self.__tableview.add_export_handler('json', '.JSON file', 'json', self.__on_export_json)
        self.__tableview.add_export_handler('wordlist', 'Word list', 'txt', self.__on_export_wordlist)

        # toolbar
        toolbar = self.__tableview.get_toolbar()

        toolitem = Gtk.SeparatorToolItem()
        toolitem.show()
        toolbar.insert(toolitem, -1)

        path = pymobius.mediator.call('extension.get-resource-path', EXTENSION_ID, 'import.png')
        image = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_dnd)
        image.show()

        self.__test_keywords_toolitem = Gtk.ToolButton.new()
        self.__test_keywords_toolitem.set_icon_widget(image.get_ui_widget())
        self.__test_keywords_toolitem.show()
        self.__test_keywords_toolitem.set_tooltip_text("Import data from file")
        self.__test_keywords_toolitem.connect("clicked", self.__on_import_file)
        toolbar.insert(self.__test_keywords_toolitem, -1)

        # status bar
        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.IN)
        frame.show()
        vbox.add_child(frame, mobius.ui.box.fill_none)

        self.__status_label = mobius.ui.label()
        self.__status_label.set_halign(mobius.ui.label.align_left)
        self.__status_label.set_elide_mode(mobius.ui.label.elide_end)
        self.__status_label.set_visible(True)
        frame.add(self.__status_label.get_ui_widget())

        # populate table
        self.__populate_tableview()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_widget(self):
        return self.__widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Show widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def show(self):
        self.__widget.show()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
        pass

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate tableview
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate_tableview(self):
        self.__tableview.clear()

        count = 0

        for hash_type, value, password in self.__turing.get_hashes():
            self.__tableview.add_row((hash_type, value, password))
            count += 1

        if count == 1:
            text = '1 hash found.'

        else:
            text = f'{count:d} hashes found.'

        self.__status_label.set_text(f'Database loaded. {text}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save current state
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle tableview->reload
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def main_tableview_on_reload_data(self, widget, *args):
        self.__populate_tableview()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle import file option
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_import_file(self, widget, *args):

        # choose file
        fs = Gtk.FileChooserDialog(title='Import from...', action=Gtk.FileChooserAction.OPEN)
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)

        # fill filefilters
        importers = [('json', '.json data file', ['json']), ('pot', 'John the Ripper .pot file', ['pot'])]

        for filetype, name, extensions in sorted(importers):
            filter_name = f'{name} ({", ".join(f"*.{e}" for e in extensions)})'

            filefilter = Gtk.FileFilter()
            filefilter.set_name(filter_name)
            filefilter.filetype = filetype

            for i in extensions:
                filefilter.add_pattern(f'*.{i}')

            fs.add_filter(filefilter)

        # run dialog
        rc = fs.run()

        filename = fs.get_filename()
        filefilter = fs.get_filter()
        filter_type = filefilter.filetype

        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        # import file according to type
        count = 0

        if filter_type == 'json':
            count = self.__on_import_json(filename)

        elif filter_type == 'pot':
            count = self.__on_import_pot(filename)

        # show number of hashes imported
        if count == 1:
            text = '1 hash imported.'
        else:
            text = f'{count:d} hashes imported.'

        self.__status_label.set_text(text)

        # populate tableview, if necessary
        if count > 0:
            self.__populate_tableview()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Import .json file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_import_json(self, filename):

        # read data
        fp = open(filename, encoding='utf-8')
        model = json.load(fp)
        fp.close()

        # store data
        transaction = self.__turing.new_transaction()
        count = 0

        for t, v, p in model.get('hashes', []):
            if not self.__turing.has_hash(t, v):
                self.__turing.set_hash(t, v, p)
                count += 1

        transaction.commit()

        return count

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Import .pot file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_import_pot(self, filename):
        f = mobius.io.new_file_by_path(filename)
        fp = mobius.io.line_reader(f.new_reader(), 'utf-8', '\n')

        # store data
        transaction = self.__turing.new_transaction()
        count = 0

        for line in fp:

            # parse line
            line = line.rstrip('\n\r')
            hash_text, password = line.split(':', 1)
            username = None
            iterations = None

            if hash_text.startswith('$NT$'):
                hash_type = 'nt'
                hash_value = hash_text[4:]

            elif hash_text.startswith('$LM$'):
                hash_type = 'lm'
                hash_value = hash_text[4:]

            elif hash_text.startswith('M$'):
                hash_type = 'msdcc1'
                hash_text = hash_text[2:]
                username, hash_value = hash_text.split('#')

            elif hash_text.startswith('$DCC2$'):
                hash_type = 'msdcc2'
                hash_text = hash_text[6:]
                iterations, username, hash_value = hash_text.split('#')

            else:
                hash_type = None
                hash_value = None

            # set hash
            if hash_type and not self.__turing.has_hash(hash_type, hash_value):
                self.__turing.set_hash(hash_type, hash_value, password)
                count += 1

        transaction.commit()
        fp.close()

        return count

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Export wordlist file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_wordlist(self, uri, data):
        wordlist = set(p for (t, v, p) in self.__tableview)

        # create writer
        f = mobius.io.new_file_by_url(uri)
        fp = mobius.io.text_writer(f.new_writer())

        # export data
        for word in sorted(wordlist):
            fp.write(f'{word}\n')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Export .json file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_json(self, url, data):
        model = {'version': JSON_VERSION, 'hashes': [(t, v, p) for (t, v, p) in self.__tableview]}

        f = mobius.io.new_file_by_url(url)
        fp = mobius.io.text_writer(f.new_writer())
        json.dump(model, fp)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Turing view
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class TuringView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()

        self.name = f'{EXTENSION_NAME} v{EXTENSION_VERSION} (Cryptography Viewer)'
        icon_path = self.__mediator.call('extension.get-icon-path', EXTENSION_ID)
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__database_view = DatabaseView()
        self.__widget = self.__database_view
        self.__widget.show()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_widget(self):
        return self.__widget.get_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_destroy view
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        self.__database_view.on_destroy()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    icon = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)
    mobius.core.add_resource('menu.tools.' + EXTENSION_ID, 'Menu Tool: Turing', (icon, EXTENSION_NAME, on_activate))


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('menu.tools.' + EXTENSION_ID)
    pymobius.mediator.call('ui.working-area.del', EXTENSION_ID)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief on_activate
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def on_activate(item_id):
    widget = TuringView()
    icon_path = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)

    working_area = pymobius.mediator.call('ui.working-area.new', EXTENSION_ID)
    working_area.set_default_size(900, 700)
    working_area.set_title(EXTENSION_NAME)
    working_area.set_icon(icon_path)
    working_area.set_widget(widget)
    working_area.show()
