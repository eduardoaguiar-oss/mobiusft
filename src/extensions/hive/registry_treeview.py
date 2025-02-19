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
import datetime
import html
import traceback

import mobius
import pymobius.mediator
from gi.repository import Gtk

from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
KEY_ICON_NAME, KEY_NAME, KEY_PATH, KEY_OBJECT = range(4)
VALUE_NAME, VALUE_TYPE, VALUE_DATA, VALUE_OBJECT = range(4)
VIEW_FILELIST, VIEW_REGISTRY, VIEW_REPORT, VIEW_FILETREE = range(4)
REGISTRYVIEW_LOGICAL_MODE, REGISTRYVIEW_PHYSICAL_MODE = range(2)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Registry constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
(REG_NONE,
 REG_SZ,
 REG_EXPAND_SZ,
 REG_BINARY,
 REG_DWORD,
 REG_DWORD_BIG_ENDIAN,
 REG_LINK,
 REG_MULTI_SZ,
 REG_RESOURCE_LIST,
 REG_FULL_RESOURCE_DESCRIPTOR,
 REG_RESOURCE_REQUIREMENTS_LIST,
 REG_QWORD) = range(12)

REG_TYPE = [
    'NONE',
    'SZ',
    'EXPAND_SZ',
    'BINARY',
    'DWORD',
    'DWORD_BIG_ENDIAN',
    'LINK',
    'MULTI_SZ',
    'RESOURCE_LIST',
    'FULL_RESOURCE_DESCRIPTOR',
    'RESOURCE_REQUIREMENTS_LIST',
    'QWORD'
]


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get date from UNIX timestamp
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_unix_time(timestamp):
    delta = datetime.timedelta(seconds=timestamp)
    date = datetime.datetime(1970, 1, 1, 0, 0, 0) + delta
    return date


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get date from NT timestamp
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_nt_time(timestamp):
    delta = datetime.timedelta(seconds=timestamp // 10000000)
    date = datetime.datetime(1601, 1, 1, 0, 0, 0) + delta
    return date


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief RegistryTreeView widget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class RegistryTreeView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, mode):
        self.__mode = mode
        self.__mediator = pymobius.mediator.copy()
        self.__can_copy_value = False
        app = mobius.core.application()

        # view data
        if mode == REGISTRYVIEW_LOGICAL_MODE:
            icon_name = 'registry.png'
            self.name = 'Registry'
        else:
            icon_name = 'filetree.png'
            self.name = 'Physical files'

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, icon_name)
        self.icon_data = open(path, 'rb').read()

        # build widget
        self.__widget = mobius.ui.container()
        self.__widget.show()

        # vbox
        vbox = mobius.ui.box(mobius.ui.box.orientation_vertical)
        vbox.set_spacing(5)
        vbox.set_visible(True)
        self.__widget.set_content(vbox)

        # hpaned
        self.__hpaned = Gtk.HPaned()
        self.__hpaned.show()
        vbox.add_child(self.__hpaned, mobius.ui.box.fill_with_widget)

        position = mobius.framework.get_config('hive.registry-paned-position')
        if position:
            self.__hpaned.set_position(position)

        # keys treeview
        frame = Gtk.Frame()
        frame.show()
        self.__hpaned.pack1(frame, False, True)

        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        frame.add(sw)

        model = Gtk.TreeStore.new([str, str, str, object])

        self.treeview = Gtk.TreeView.new_with_model(model)
        self.treeview.set_headers_visible(False)
        self.treeview.set_enable_tree_lines(True)
        self.treeview.connect('test-expand-row', self.on_treeview_expand_row)

        self.treeview.show()
        sw.add(self.treeview)

        tvcolumn = Gtk.TreeViewColumn()
        tvcolumn.set_sizing(Gtk.TreeViewColumnSizing.AUTOSIZE)
        self.treeview.append_column(tvcolumn)

        renderer = Gtk.CellRendererPixbuf()
        tvcolumn.pack_start(renderer, False)
        tvcolumn.add_attribute(renderer, 'icon-name', KEY_ICON_NAME)

        renderer = Gtk.CellRendererText()
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', KEY_NAME)

        selection = self.treeview.get_selection()
        selection.connect('changed', self.on_treeview_selection_changed)

        # values listview
        frame = Gtk.Frame()
        frame.show()
        self.__hpaned.pack2(frame, False, True)

        self.__value_tableview = self.__mediator.call('ui.new-widget', 'tableview')
        frame.add(self.__value_tableview.get_ui_widget())

        column = self.__value_tableview.add_column('value')
        column.is_sortable = True

        column = self.__value_tableview.add_column('type')
        column.is_sortable = True

        column = self.__value_tableview.add_column('size', column_type='int')
        column.is_sortable = True

        column = self.__value_tableview.add_column('data')
        column.is_sortable = True

        column = self.__value_tableview.add_column('object', column_type='object')
        column.is_visible = False

        self.__value_tableview.set_report_id('registry.key-values')
        self.__value_tableview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__value_tableview.set_control(self, 'value_tableview')
        self.__value_tableview.show()

        # status bar
        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.IN)
        frame.show()
        vbox.add_child(frame, mobius.ui.box.fill_none)

        self.__status_label = mobius.ui.label()
        self.__status_label.set_selectable(True)
        self.__status_label.set_halign(mobius.ui.label.align_left)
        self.__status_label.set_elide_mode(mobius.ui.label.elide_middle)
        self.__status_label.set_visible(True)
        frame.add(self.__status_label.get_ui_widget())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, registry):
        if self.__mode == REGISTRYVIEW_LOGICAL_MODE:
            self.__populate_treeview_logical_view(registry)

        else:
            self.__populate_treeview_physical_view(registry)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Return true if value can be copied
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def can_copy_value(self):
        return self.__can_copy_value

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate treeview with logical view
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate_treeview_logical_view(self, registry):
        model = self.treeview.get_model()
        model.clear()

        for key in registry.keys:
            treeiter = model.append(None, ('folder', key.name, '\\' + key.name, key))

            # insert placeholder if key has subkeys
            if key.has_subkeys():
                model.append(treeiter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate treeview with physical view
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate_treeview_physical_view(self, registry):
        model = self.treeview.get_model()
        model.clear()

        for hivefile in registry.files:
            key = hivefile.root_key
            treeiter = model.append(None, ('folder', hivefile.path, '\\' + key.name, key))

            # insert root key
            if key.subkeys:
                model.append(treeiter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save current state
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        mobius.framework.set_config('hive.registry-paned-position', self.__hpaned.get_position())
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle test_expand_row event
    # @detail This function implements lazy evaluation. Every item inserted
    # receive a null child (obj = None). When row is expanded, three situations
    # arise:
    # 1. row has no child: object already loaded but no subitems were found
    # 2. row has null child: just a markplace. Object must be loaded
    # 3. row has valid child: object already loaded
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_treeview_expand_row(self, treeview, treeiter, path, *args):
        model = treeview.get_model()
        child_iter = model.iter_children(treeiter)

        if child_iter:
            first_subkey = model.get_value(child_iter, KEY_OBJECT)

            if not first_subkey:  # just a placeholder. Populate subkeys
                model.remove(child_iter)

                key = model.get_value(treeiter, KEY_OBJECT)
                path = model.get_value(treeiter, KEY_PATH)

                for subkey in key.subkeys:
                    subkey_path = '%s\\%s' % (path, subkey.name)
                    subkey_treeiter = model.append(treeiter, ('folder', subkey.name, subkey_path, subkey))

                    if subkey.subkeys:
                        model.append(subkey_treeiter)

        return False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle treeview selection-changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_treeview_selection_changed(self, selection, *args):
        model, treeiter = selection.get_selected()

        # item selected
        if treeiter:
            key = model.get_value(treeiter, KEY_OBJECT)
            key_path = model.get_value(treeiter, KEY_PATH)
            self.__populate_value_tableview(key)

            if key:
                text = f"<b>Last modified:</b> {key.last_modification_time or '-'}. \
                         <b>Path:</b> {key_path}. \
                         <b>Classname:</b> {key.classname or '-'}"
                self.__status_label.set_markup(text.replace('&', '&amp;'))

        # NO items selected
        else:
            self.__status_label.set_text('')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle value_tableview selection-changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def value_tableview_on_selection_changed(self, selection_list):
        if selection_list:
            row_number, row_data = selection_list[0]
            value_name, value_type, size, data, value = row_data
        else:
            value = None

        # update status label
        text = ''

        # value selected
        if value:
            data = value.data

            # show BINARY and NONE as NT datetime, if possible
            if data.type in (REG_BINARY, REG_NONE, REG_QWORD) and data.size == 8:
                qword = data.get_data_as_qword()
                try:
                    text = f'<b>Dec:</b> {qword:d}. <b>NT time:</b> {get_nt_time(qword)}'
                except Exception:
                    pass

            # show DWORD as HEX and UNIX time
            elif data.type in (REG_BINARY, REG_NONE, REG_DWORD, REG_DWORD_BIG_ENDIAN) and data.size == 4:
                dword = data.get_data_as_dword()
                try:
                    text = f'<b>Dec:</b> {dword:d}. <b>Unix time:</b> {get_unix_time(dword)}'
                except Exception as e:
                    mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

            elif data.type in (REG_BINARY, REG_NONE):
                try:
                    s = html.escape(data.data.decode('utf-16')).rstrip('\0')
                    if '\0' not in s:
                        text = f'<b>String:</b> {s}'
                except Exception as e:
                    mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

        self.__status_label.set_markup(text)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate value tableview
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate_value_tableview(self, key):
        self.__value_tableview.clear()
        self.__value_tableview.set_report_name(('values of registry key <%s>' % key.name))

        for value in key.values:
            data = value.data

            # NAME
            value_name = value.name or '(default)'

            # REG_TYPE
            if not data or data.type is None:
                value_type_text = '(not set)'

            elif data.type < len(REG_TYPE):
                value_type_text = 'REG_%s' % REG_TYPE[data.type]

            else:
                value_type_text = '0x%04x' % data.type

            # format data
            if not data:
                value_data = '(not set)'

            elif len(data.data) == 0:
                value_data = '(value not set)'

            elif data.type in (REG_DWORD, REG_DWORD_BIG_ENDIAN):
                value_data = '0x%08x' % data.get_data_as_dword()

            elif data.type == REG_QWORD:
                value_data = '0x%016x' % data.get_data_as_qword()

            elif data.type == REG_EXPAND_SZ:
                value_data = data.get_data_as_string("utf-16")

            elif data.type == REG_SZ:
                try:
                    value_data = data.data.decode('utf-16')
                except:
                    try:
                        value_data = data.data.decode('utf-8')
                    except:
                        value_data = data.data

            elif data.type == REG_MULTI_SZ:
                values = data.data.decode('utf-16', 'ignore').rstrip('\0').split('\0')
                value_data = '\n'.join(values)

            else:
                value_data = mobius.encoder.hexstring(data.data, ' ')

            size = data.size if data else 0

            self.__value_tableview.add_row((value_name, value_type_text, size, value_data, value))
