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
EXTENSION_VERSION = '1.3'
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

        self.__new_hashset_toolitem = Gtk.ToolButton.new()
        self.__new_hashset_toolitem.set_icon_name('list-add')
        self.__new_hashset_toolitem.connect("clicked", self.__on_new_hashset)
        self.__new_hashset_toolitem.show()
        self.__new_hashset_toolitem.set_tooltip_text("Create new hashset")
        toolbar.insert(self.__new_hashset_toolitem, -1)

        self.__remove_hashset_toolitem = Gtk.ToolButton.new()
        self.__remove_hashset_toolitem.set_icon_name('list-remove')
        self.__remove_hashset_toolitem.connect("clicked", self.__on_remove_hashset)
        self.__remove_hashset_toolitem.show()
        self.__remove_hashset_toolitem.set_tooltip_text("Remove hashset")
        self.__remove_hashset_toolitem.set_sensitive(False)
        toolbar.insert(self.__remove_hashset_toolitem, -1)

        self.__clear_hashset_toolitem = Gtk.ToolButton.new()
        self.__clear_hashset_toolitem.set_icon_name('edit-clear')
        self.__clear_hashset_toolitem.connect("clicked", self.__on_clear_hashset)
        self.__clear_hashset_toolitem.show()
        self.__clear_hashset_toolitem.set_tooltip_text("Clear hashset")
        self.__clear_hashset_toolitem.set_sensitive(False)
        toolbar.insert(self.__clear_hashset_toolitem, -1)

        self.__import_hashlist_toolitem = Gtk.ToolButton.new()
        self.__import_hashlist_toolitem.set_icon_name('document-import')
        self.__import_hashlist_toolitem.connect("clicked", self.__on_import_hash_list)
        self.__import_hashlist_toolitem.show()
        self.__import_hashlist_toolitem.set_tooltip_text("Import hash list from file")
        self.__import_hashlist_toolitem.set_sensitive(False)
        toolbar.insert(self.__import_hashlist_toolitem, -1)

        self.__export_hashlist_toolitem = Gtk.ToolButton.new()
        self.__export_hashlist_toolitem.set_icon_name('document-export')
        self.__export_hashlist_toolitem.connect("clicked", self.__on_export_hash_list)
        self.__export_hashlist_toolitem.show()
        self.__export_hashlist_toolitem.set_tooltip_text("Export hash list to file")
        self.__export_hashlist_toolitem.set_sensitive(False)
        toolbar.insert(self.__export_hashlist_toolitem, -1)

        self.__calculate_hashes_toolitem = Gtk.ToolButton.new()
        self.__calculate_hashes_toolitem.set_icon_name('folder-open')
        self.__calculate_hashes_toolitem.connect("clicked", self.__on_calculate_hashes)
        self.__calculate_hashes_toolitem.show()
        self.__calculate_hashes_toolitem.set_tooltip_text("Calculate hashes from files in a folder")
        self.__calculate_hashes_toolitem.set_sensitive(False)
        toolbar.insert(self.__calculate_hashes_toolitem, -1)

        # listview
        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        self.__widget.add_child(sw, mobius.core.ui.box.fill_with_widget)

        model = Gtk.ListStore.new([str, str, str, GObject.TYPE_LONG, object])
        model.set_sort_column_id(HASHSET_ID, Gtk.SortType.ASCENDING)

        self.__listview = Gtk.TreeView.new_with_model(model)
        # self.__listview.set_headers_visible (False)
        self.__listview.show()
        sw.add(self.__listview)

        selection = self.__listview.get_selection()
        selection.connect('changed', self.__on_listview_selection_changed)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('ID')
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', HASHSET_ID)
        tvcolumn.set_sort_column_id(HASHSET_ID)
        self.__listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        renderer.set_property('editable', True)
        renderer.connect('edited', self.__on_description_edited)

        tvcolumn = Gtk.TreeViewColumn('Description')
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', HASHSET_DESCRIPTION)
        tvcolumn.set_sort_column_id(HASHSET_DESCRIPTION)
        self.__listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('Is alert?')
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', HASHSET_IS_ALERT)
        tvcolumn.set_sort_column_id(HASHSET_IS_ALERT)
        self.__listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        renderer.set_property('xalign', 1.0)

        tvcolumn = Gtk.TreeViewColumn('Hashes')
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', HASHSET_SIZE)
        tvcolumn.set_sort_column_id(HASHSET_SIZE)
        self.__listview.append_column(tvcolumn)

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
        self.__populate_listview()

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
    def __populate_listview(self):
        model = self.__listview.get_model()
        model.clear()

        kff = mobius.core.kff.kff()

        for h_id, h in kff.get_hashsets():
            model.append((h_id, h.description, '✓' if h.is_alert() else '', h.get_size(), h))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle listview selection changed event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_listview_selection_changed(self, selection, *args):
        model, treeiter = selection.get_selected()

        if treeiter:
            is_hashset_selected = True

        else:
            is_hashset_selected = False

        # enable/disable widgets
        self.__remove_hashset_toolitem.set_sensitive(is_hashset_selected)
        self.__clear_hashset_toolitem.set_sensitive(is_hashset_selected)
        self.__import_hashlist_toolitem.set_sensitive(is_hashset_selected)
        self.__export_hashlist_toolitem.set_sensitive(is_hashset_selected)
        self.__calculate_hashes_toolitem.set_sensitive(is_hashset_selected)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle new hashset
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_new_hashset(self, widget, *args):

        # show dialog
        dialog = Gtk.Dialog(title='New hashset', modal=True)
        dialog.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        dialog.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        dialog.set_position(Gtk.WindowPosition.CENTER)
        dialog.set_type_hint(Gdk.WindowTypeHint.DIALOG)
        dialog.set_border_width(10)

        grid = Gtk.Grid.new()
        grid.set_row_spacing(5)
        grid.set_column_spacing(5)
        grid.show()
        dialog.vbox.pack_start(grid, True, True, 0)

        label = mobius.core.ui.label('ID')
        label.set_halign(mobius.core.ui.box.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 0, 1, 1)

        dialog.id_entry = Gtk.Entry()
        dialog.id_entry.show()
        grid.attach(dialog.id_entry, 1, 0, 1, 1)

        label = mobius.core.ui.label('Description')
        label.set_halign(mobius.core.ui.box.align_right)
        label.set_visible(True)
        grid.attach(label, 0, 1, 1, 1)

        dialog.description_entry = Gtk.Entry()
        dialog.description_entry.show()
        grid.attach(dialog.description_entry, 1, 1, 1, 1)

        is_alert_check_button = Gtk.CheckButton.new_with_mnemonic('Is _Alert hash set')
        is_alert_check_button.set_active(True)
        is_alert_check_button.show()
        grid.attach(is_alert_check_button, 0, 2, 2, 1)

        rc = dialog.run()
        hashset_id = dialog.id_entry.get_text().rstrip()
        hashset_description = dialog.description_entry.get_text()
        is_alert = is_alert_check_button.get_active()
        dialog.destroy()

        # return if user pressed anything but OK or if hashset_id is empty
        if rc != Gtk.ResponseType.OK or not hashset_id:
            return

        # create hashset
        kff = mobius.core.kff.kff()
        h = kff.new_hashset(hashset_id, is_alert)
        h.description = hashset_description

        # update window
        model = self.__listview.get_model()
        treeiter = model.append((hashset_id, h.description, '✓' if h.is_alert() else '', h.get_size(), h))

        selection = self.__listview.get_selection()
        selection.select_iter(treeiter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle remove hashset
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_remove_hashset(self, widget, *args):

        # get selected hashset
        selection = self.__listview.get_selection()
        model, treeiter = selection.get_selected()

        if not treeiter:
            mobius.core.logf('WRN no row selected to remove')
            return

        hashset_id = model.get_value(treeiter, HASHSET_ID)
        hashset = model.get_value(treeiter, HASHSET_OBJ)

        # show confirmation dialog
        dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
        dialog.text = f"You are about to remove hash set '{hashset_id}'. Are you sure?"
        dialog.add_button(mobius.core.ui.message_dialog.button_yes)
        dialog.add_button(mobius.core.ui.message_dialog.button_no)
        dialog.set_default_response(mobius.core.ui.message_dialog.button_no)
        rc = dialog.run()

        if rc != mobius.core.ui.message_dialog.button_yes:
            return

        # remove hashset
        kff = mobius.core.kff.kff()
        kff.remove_hashset(hashset_id)

        # update window
        has_next = model.remove(treeiter)

        if has_next:
            selection.select_iter(treeiter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle clear hashset
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_clear_hashset(self, widget, *args):

        # get selected hashset
        selection = self.__listview.get_selection()
        model, treeiter = selection.get_selected()

        if not treeiter:
            mobius.core.logf('WRN no row selected to clear')
            return

        hashset_id = model.get_value(treeiter, HASHSET_ID)
        hashset = model.get_value(treeiter, HASHSET_OBJ)

        # show confirmation dialog
        dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
        dialog.text = f"You are about to remove all hashes from '{hashset_id}' hash set. Are you sure?"
        dialog.add_button(mobius.core.ui.message_dialog.button_yes)
        dialog.add_button(mobius.core.ui.message_dialog.button_no)
        dialog.set_default_response(mobius.core.ui.message_dialog.button_no)
        rc = dialog.run()

        if rc != mobius.core.ui.message_dialog.button_yes:
            return

        # clear hashset
        transaction = hashset.new_transaction()
        hashset.clear()
        transaction.commit()

        # update listview
        model.set_value(treeiter, HASHSET_SIZE, 0)

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

        transaction = mobius.framework.new_config_transaction()
        mobius.framework.set_config('general.save-folder', folder)
        transaction.commit()

        # add extension to name
        uri_root, uri_ext = os.path.splitext(url)

        if uri_ext != '.' + fileext:
            url = url + '.' + fileext

        # export
        f = mobius.core.io.new_file_by_url(url)
        handler(f, hashset)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle calculate hashes option
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_calculate_hashes(self, widget, *args):

        # get selected hashset
        selection = self.__listview.get_selection()
        model, treeiter = selection.get_selected()

        if not treeiter:
            mobius.core.logf('WRN no selected row')
            return

        hashset_id = model.get_value(treeiter, HASHSET_ID)
        hashset = model.get_value(treeiter, HASHSET_OBJ)

        # build dialog
        fs = Gtk.FileChooserDialog(title=f'Import hashes from folder into "{hashset_id}" hash set')
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        fs.set_action(Gtk.FileChooserAction.SELECT_FOLDER)
        fs.vbox.set_border_width(5)
        fs.vbox.set_spacing(5)

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(5)
        hbox.set_visible(True)
        fs.vbox.pack_start(hbox.get_ui_widget(), False, False, 0)

        label = mobius.core.ui.label('Hash type:')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        # hardcode for now
        # @todo Use mobius.core.crypt.get_hash_list, when available
        c_model = Gtk.ListStore.new([str, str])
        c_model.append(('adler32', 'Adler 32'))
        c_model.append(('crc32', 'CRC-32'))
        c_model.append(('ed2k', 'E-Donkey'))
        c_model.append(('md4', 'MD4'))
        c_model.append(('md5', 'MD5'))
        c_model.append(('sha1', 'SHA-1'))
        c_model.append(('sha2-224', 'SHA-2 224 bits'))
        c_model.append(('sha2-256', 'SHA-2 256 bits'))
        c_model.append(('sha2-384', 'SHA-2 384 bits'))
        c_model.append(('sha2-512', 'SHA-2 512 bits'))
        c_model.append(('sha2-512-224', 'SHA-2 512/224 bits'))
        c_model.append(('sha2-512-256', 'SHA-2 512/256 bits'))
        c_model.append(('zip', 'ZIP'))
        renderer = Gtk.CellRendererText()

        combobox = Gtk.ComboBox.new_with_model(c_model)
        combobox.set_hexpand(True)
        combobox.pack_start(renderer, True)
        combobox.add_attribute(renderer, 'text', 1)
        combobox.set_id_column(0)
        combobox.set_active_id('sha2-256')
        combobox.show()

        hbox.add_child(combobox, mobius.core.ui.box.fill_with_widget)

        # run dialog
        rc = fs.run()

        url = fs.get_uri()
        hash_type = combobox.get_active_id()
        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        # calculate hashes
        folder = mobius.core.io.new_folder_by_url(url)
        hashes = []

        for f in walk(folder):
            self.__status_label.set_text(f'Importing {f.path}')
            mobius.core.ui.flush()

            hash_value = calculate_hash(f, hash_type)
            hashes.append((hash_type, hash_value))

        # add to hashset
        self.__status_label.set_text(f'Adding hashes to "{hashset_id}" hash set')
        mobius.core.ui.flush()

        transaction = hashset.new_transaction()

        for hash_type, hash_value in hashes:
            hashset.add(hash_type, hash_value)

        transaction.commit()

        # update info
        model.set_value(treeiter, HASHSET_SIZE, hashset.get_size())
        self.__status_label.set_text('')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle description edited event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_description_edited(self, cell, path, new_text, *args):
        model = self.__listview.get_model()
        treeiter = model.get_iter_from_string(path)
        text = model.get_value(treeiter, HASHSET_DESCRIPTION)

        if text != new_text:
            hashset = model.get_value(treeiter, HASHSET_OBJ)
            hashset.description = new_text
            model.set_value(treeiter, HASHSET_DESCRIPTION, new_text)


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
