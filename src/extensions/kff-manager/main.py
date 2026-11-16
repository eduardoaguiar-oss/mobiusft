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
import json
import os
import os.path

import mobius
import mobius.core.crypt
import mobius.core.kff
import mobius.core.io
import pymobius
from gi.repository import GObject
from gi.repository import Gdk
from gi.repository import Gtk

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Extension metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
EXTENSION_ID = 'kff-manager'
EXTENSION_NAME = 'KFF Manager'
EXTENSION_AUTHOR = 'Eduardo Aguiar'
EXTENSION_VERSION = '2.0'
EXTENSION_DESCRIPTION = 'Known File Filter (KFF) Manager'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get hashes from json file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def import_json(f, hashset):
    fp = mobius.core.io.text_reader(f.new_reader())
    d = json.load(fp)

    for htype, hashlist in d.items():
        for h in hashlist:
            hashset.add(htype, h)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get hashes from tab separated file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def import_tab_separated(f, hashset):
    fp = mobius.core.io.line_reader(f.new_reader())

    for line in fp:
        htype, hvalue = line.split('\t')
        hashset.add(htype, hvalue)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get hashes from .csv file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def import_csv(f, hashset):
    fp = mobius.core.io.line_reader(f.new_reader())
    first = True

    for line in fp:
        if first:
            first = False

        else:
            htype, hvalue = line.split(',')

            if htype[0] == '"':
                htype = htype[1:-1]

            if hvalue[0] == '"':
                hvalue = hvalue[1:-1]

            hashset.add(htype, hvalue)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Import hashes from LED .txt file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def import_led(f, hashset):
    fp = mobius.core.io.line_reader(f.new_reader(), 'iso-8859-1')

    for line in fp:
        if line[0] != '#':
            data = line.rstrip().split(' *')
            hashset.add('md5', data[0])
            hashset.add('ed2k', data[2])
            hashset.add('sha1', data[3])
            hashset.add('sha2-256', data[6])


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Export hashset as .json file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def export_json(f, hashset):
    # generate data
    d = {}
    for htype, hvalue in hashset.hashes:
        d.setdefault(htype, []).append(hvalue)

    # write data
    fp = mobius.core.io.text_writer(f.new_writer())
    json.dump(d, fp)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Export hashset as tab separated file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def export_tab(f, hashset):
    fp = mobius.core.io.text_writer(f.new_writer())

    for htype, hvalue in hashset.hashes:
        fp.write('%s\t%s\n' % (htype, hvalue))


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Export hashset as csv file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def export_csv(f, hashset):
    fp = mobius.core.io.text_writer(f.new_writer())
    fp.write('id,name,value\r\n')

    for htype, hvalue in hashset.hashes:
        fp.write('%s,%s\n' % (htype, hvalue))


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Walk through folder, recursively
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def walk(folder):
    for entry in folder.get_children():
        if entry.is_file():
            yield entry
        else:
            for child in walk(entry):
                yield child


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Calculate file hash
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def calculate_hash(f, hash_type):
    reader = f.new_reader()
    h = mobius.core.crypt.hash(hash_type)
    data = reader.read(65536)

    while data:
        h.update(data)
        data = reader.read(65536)

    return h.get_hex_digest()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief KFF view
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
(HASHSET_ID, HASHSET_DESCRIPTION, HASHSET_IS_ALERT, HASHSET_SIZE, HASHSET_OBJ) = range(5)


class KFFView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__data = []

        self.name = f'{EXTENSION_NAME} v{EXTENSION_VERSION} (KFF Manager)'
        icon_path = self.__mediator.call('extension.get-icon-path', EXTENSION_ID)
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        self.__widget.set_border_width(5)
        self.__widget.set_visible(True)

        # toolbar
        toolbar = Gtk.Toolbar()
        toolbar.set_style(Gtk.ToolbarStyle.ICONS)
        toolbar.show()
        self.__widget.add_child(toolbar, mobius.core.ui.box.fill_none)

        self.__import_hashlist_toolitem = Gtk.ToolButton.new()
        self.__import_hashlist_toolitem.set_icon_name('document-import')
        self.__import_hashlist_toolitem.connect("clicked", self.__on_import_hash_list)
        self.__import_hashlist_toolitem.show()
        self.__import_hashlist_toolitem.set_tooltip_text("Import hash list from file")
        self.__import_hashlist_toolitem.set_sensitive(True)
        toolbar.insert(self.__import_hashlist_toolitem, -1)

        self.__export_hashlist_toolitem = Gtk.ToolButton.new()
        self.__export_hashlist_toolitem.set_icon_name('document-export')
        self.__export_hashlist_toolitem.connect("clicked", self.__on_export_hash_list)
        self.__export_hashlist_toolitem.show()
        self.__export_hashlist_toolitem.set_tooltip_text("Export hash list to file")
        self.__export_hashlist_toolitem.set_sensitive(True)
        toolbar.insert(self.__export_hashlist_toolitem, -1)

        # status listview
        self.__status_listview = self.__mediator.call('ui.new-widget', 'tableview')
        self.__status_listview.set_report_id('kff.status')
        self.__status_listview.set_report_name('KFF Status')
        self.__status_listview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__status_listview.show()
        self.__status_listview.set_drag_and_drop(False)

        self.__status_listview.add_column('name')
        self.__status_listview.add_column('count', 'Count', column_type='int')

        self.__widget.add_child(self.__status_listview.get_ui_widget(), mobius.core.ui.box.fill_with_widget)

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

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle close button
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_extension_close(self, widget, *args):
        self.__mediator.call('ui.working-area.close', self.working_area.id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief show save/ignore/cancel dialog if there are modified items
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_widget_stopped(self):
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate listview
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate(self):
        self.__status_listview.clear()
        self.__status_listview.add_row(('Files', mobius.core.kff.get_file_count()))
        self.__status_listview.add_row(('Alert files', mobius.core.kff.get_alert_count()))
        self.__status_listview.add_row(('Hashes', mobius.core.kff.get_hash_count()))
        self.__status_listview.add_row(('Thumbnails', mobius.core.kff.get_thumbnail_count()))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle import hash list
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_import_hash_list(self, widget, *args):

        # get selected hashset
        selection = self.__listview.get_selection()
        model, treeiter = selection.get_selected()

        if not treeiter:
            mobius.core.logf('WRN no selected row')
            return

        hashset_id = model.get_value(treeiter, HASHSET_ID)
        hashset = model.get_value(treeiter, HASHSET_OBJ)

        # build dialog
        fs = Gtk.FileChooserDialog(title=f'Importing hash list to "{hashset_id}" hash set')
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        fs.set_select_multiple(True)

        # add import handlers
        filefilter = Gtk.FileFilter()
        filefilter.handler = import_json
        filefilter.extension = 'json'
        filefilter.set_name('JSON file (.json)')
        filefilter.add_pattern('*.json')
        fs.add_filter(filefilter)

        filefilter = Gtk.FileFilter()
        filefilter.handler = import_tab_separated
        filefilter.extension = 'txt'
        filefilter.set_name('Tab separated file (.txt)')
        filefilter.add_pattern('*.txt')
        fs.add_filter(filefilter)

        filefilter = Gtk.FileFilter()
        filefilter.handler = import_csv
        filefilter.extension = 'csv'
        filefilter.set_name('CSV file (.csv)')
        filefilter.add_pattern('*.csv')
        fs.add_filter(filefilter)

        filefilter = Gtk.FileFilter()
        filefilter.handler = import_led
        filefilter.extension = 'txt'
        filefilter.set_name('LED file (.txt)')
        filefilter.add_pattern('*.txt')
        fs.add_filter(filefilter)

        # run dialog
        rc = fs.run()
        urls = fs.get_uris()
        folder = fs.get_current_folder()

        filefilter = fs.get_filter()
        handler = filefilter.handler
        fileext = filefilter.extension

        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        # import files
        transaction = hashset.new_transaction()

        for url in urls:
            self.__status_label.set_text(f'Importing {url}')
            mobius.core.ui.flush()

            f = mobius.core.io.new_file_by_url(url)
            handler(f, hashset)

        transaction.commit()

        # update info
        model.set_value(treeiter, HASHSET_SIZE, hashset.get_size())
        self.__status_label.set_text('')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle export hash list
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_hash_list(self, widget, *args):

        # get selected hashset
        selection = self.__listview.get_selection()
        model, treeiter = selection.get_selected()

        if not treeiter:
            mobius.core.logf('WRN no row selected to clear')
            return

        hashset_id = model.get_value(treeiter, HASHSET_ID)
        hashset = model.get_value(treeiter, HASHSET_OBJ)

        # build dialog
        fs = Gtk.FileChooserDialog(title=f'Exporting hash list to "{hashset_id}" hash set',
                                   action=Gtk.FileChooserAction.SAVE)
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        fs.set_do_overwrite_confirmation(True)

        # set current folder
        last_folder = mobius.framework.get_config('general.save-folder')
        if last_folder:
            fs.set_current_folder(last_folder)

        # add export handlers
        filefilter = Gtk.FileFilter()
        filefilter.handler = export_json
        filefilter.extension = 'json'
        filefilter.set_name('JSON file (.json)')
        filefilter.add_pattern('*.json')
        fs.add_filter(filefilter)
        fs.set_current_name(hashset_id + '.json')

        filefilter = Gtk.FileFilter()
        filefilter.handler = export_tab
        filefilter.extension = 'txt'
        filefilter.set_name('Tab separated file (.txt)')
        filefilter.add_pattern('*.txt')
        fs.add_filter(filefilter)

        filefilter = Gtk.FileFilter()
        filefilter.handler = export_csv
        filefilter.extension = 'csv'
        filefilter.set_name('CSV file (.csv)')
        filefilter.add_pattern('*.csv')
        fs.add_filter(filefilter)

        # run dialog
        rc = fs.run()
        url = fs.get_uri()
        folder = fs.get_current_folder()

        filefilter = fs.get_filter()
        handler = filefilter.handler
        fileext = filefilter.extension

        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        mobius.framework.set_config('general.save-folder', folder)

        # add extension to name
        uri_root, uri_ext = os.path.splitext(url)

        if uri_ext != '.' + fileext:
            url = url + '.' + fileext

        # export
        f = mobius.core.io.new_file_by_url(url)
        handler(f, hashset)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    icon = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)
    mobius.core.add_resource('menu.tools.' + EXTENSION_ID, 'Menu Tools: KFF Manager',
                             (icon, EXTENSION_NAME, on_activate))


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
    widget = KFFView()
    icon_path = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)

    working_area = pymobius.mediator.call('ui.working-area.new', EXTENSION_ID)
    working_area.set_default_size(500, 700)
    working_area.set_title(EXTENSION_NAME)
    working_area.set_icon(icon_path)
    working_area.set_widget(widget.get_ui_widget())
    working_area.show()
