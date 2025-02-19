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
import os.path

import mobius
import pymobius
from gi.repository import Gdk
from gi.repository import Gtk

from metadata import *
from common import REPORT_DATA_MIMETYPE
from common import csv_escape
from common import get_handler
from common import latex_escape

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
COLOR_READ_ONLY = '#808080'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief widget: attribute-list
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class AttributeListWidget(object):
    COLUMN_ID, COLUMN_NAME, COLUMN_VALUE, COLUMN_EDITABLE, COLUMN_COLOR = range(5)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__is_editable = False
        self.__paths = {}
        self.__report_id = 'unknown'
        self.__report_name = 'attribute list'
        self.__report_widget = 'attribute-list'
        self.__report_app = f'{EXTENSION_NAME} v{EXTENSION_VERSION}'
        self.__export_handlers = []

        # event handlers
        self.__on_editing_value_handler = None
        self.__after_editing_value_handler = None
        self.__on_reload_data_handler = None
        self.__on_dnd_get_data_handler = None

        # main widget
        self.__widget = mobius.ui.box(mobius.ui.box.orientation_vertical)

        # scrolled window
        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        self.__widget.add_child(sw, mobius.ui.box.fill_with_widget)

        # listview
        model = Gtk.ListStore.new([str, str, str, bool, str])

        self.__listview = Gtk.TreeView.new_with_model(model)
        self.__listview.set_enable_search(True)
        self.__listview.show()
        sw.add(self.__listview)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('Property')
        tvcolumn.set_sizing(Gtk.TreeViewColumnSizing.AUTOSIZE)
        tvcolumn.set_resizable(True)
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', self.COLUMN_NAME)
        self.__listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        renderer.connect('edited', self.__on_value_edited)

        tvcolumn = Gtk.TreeViewColumn('Value')
        tvcolumn.set_sizing(Gtk.TreeViewColumnSizing.AUTOSIZE)
        tvcolumn.set_resizable(True)
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', self.COLUMN_VALUE)
        tvcolumn.add_attribute(renderer, 'editable', self.COLUMN_EDITABLE)
        tvcolumn.add_attribute(renderer, 'foreground', self.COLUMN_COLOR)
        self.__listview.append_column(tvcolumn)

        # toolbar
        self.__toolbar = Gtk.Toolbar()
        self.__toolbar.set_style(Gtk.ToolbarStyle.ICONS)
        self.__toolbar.show()
        self.__widget.add_child(self.__toolbar, mobius.ui.box.fill_none)

        # DND button
        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'report-data.png')
        icon = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_dnd)
        icon.set_visible(True)

        self.__dnd_toolitem = Gtk.ToolButton.new()
        self.__dnd_toolitem.set_icon_widget(icon.get_ui_widget())
        self.__dnd_toolitem.set_use_drag_window(True)
        self.__dnd_toolitem.drag_source_set(Gdk.ModifierType.BUTTON1_MASK,
                                            [Gtk.TargetEntry.new(REPORT_DATA_MIMETYPE, 0, 0)], Gdk.DragAction.COPY)
        self.__dnd_toolitem.drag_source_set_icon_pixbuf(icon.get_ui_widget().get_pixbuf())
        self.__dnd_toolitem.show()
        self.__dnd_toolitem.set_tooltip_text("Drag this icon into case view to create a report")
        self.__dnd_toolitem.connect('drag-data-get', self.__on_drag_data_get)
        self.__toolbar.insert(self.__dnd_toolitem, -1)

        # Refresh button
        self.__refresh_toolitem = Gtk.ToolButton.new()
        self.__refresh_toolitem.set_icon_name('view-refresh')
        self.__refresh_toolitem.set_sensitive(False)
        self.__refresh_toolitem.show()
        self.__refresh_toolitem.set_tooltip_text("Reload data")
        self.__toolbar.insert(self.__refresh_toolitem, -1)

        # Copy button
        self.__copy_toolitem = Gtk.ToolButton.new()
        self.__copy_toolitem.set_icon_name('edit-copy')
        self.__copy_toolitem.show()
        self.__copy_toolitem.set_tooltip_text("Copy data to clipboard")
        self.__copy_toolitem.connect("clicked", self.__on_copy_data)
        self.__toolbar.insert(self.__copy_toolitem, -1)

        # Export button
        self.__export_toolitem = Gtk.ToolButton.new()
        self.__export_toolitem.set_icon_name('document-save-as')
        self.__export_toolitem.show()
        self.__export_toolitem.set_tooltip_text("Export data...")
        self.__export_toolitem.connect("clicked", self.__on_export_data)
        self.__toolbar.insert(self.__export_toolitem, -1)

        # Print button
        self.__print_toolitem = Gtk.ToolButton.new()
        self.__print_toolitem.set_icon_name('document-print')
        self.__print_toolitem.set_sensitive(False)
        self.__print_toolitem.show()
        self.__print_toolitem.set_tooltip_text("Print data")
        self.__toolbar.insert(self.__print_toolitem, -1)

        # add export handlers
        self.add_export_handler('csv', 'CSV file (RFC 4180)', 'csv', self.__on_export_csv_handler)
        self.add_export_handler('txt', 'text file', 'txt', self.__on_export_txt_handler)
        self.add_export_handler('tex', 'LaTeX table', 'tex', self.__on_export_latex_handler)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief iterate through data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __iter__(self):
        model = self.__listview.get_model()

        for attr_id, attr_name, attr_value, is_editable, color in model:
            yield attr_id, attr_name, attr_value

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief show widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def show(self):
        return self.__widget.set_visible(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief hide widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def hide(self):
        return self.__widget.set_visible(False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set widget sensitive status
    # @param sensitive True/False
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_sensitive(self, sensitive):
        return self.__widget.set_sensitive(sensitive)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set control object
    # @param control control object
    # @param wid widget id to be appended to the handler functions' names
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_control(self, control, wid=None):
        self.__on_editing_value_handler = get_handler('on_editing_value', control, wid)
        self.__after_editing_value_handler = get_handler('after_editing_value', control, wid)
        self.__on_reload_data_handler = get_handler('on_reload_data', control, wid)
        self.__on_dnd_get_data_handler = get_handler('on_dnd_get_data', control, wid)

        if self.__on_reload_data_handler:
            self.__refresh_toolitem.set_sensitive(True)
            self.__refresh_toolitem.connect("clicked", self.__on_reload_data)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set if widget is editable
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_is_editable(self, is_editable):
        self.__is_editable = is_editable

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get if widget is editable
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_is_editable(self):
        return self.__is_editable

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Check if has value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def has_value(self, attr_id):
        return attr_id in self.__paths

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add attribute to widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_value(self, attrid, attrname, attrvalue=None, is_editable=False):
        model = self.__listview.get_model()
        is_widget_editable = self.__is_editable
        is_attr_editable = is_widget_editable and is_editable
        color = COLOR_READ_ONLY if is_widget_editable and not is_attr_editable else '#000'

        treeiter = model.append((attrid, attrname, pymobius.to_string(attrvalue), is_attr_editable, color))
        path = model.get_path(treeiter)
        self.__paths[attrid] = path

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set attribute value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_value(self, attr_id, attr_value):
        path = self.__paths.get(attr_id)

        if path:
            model = self.__listview.get_model()
            treeiter = model.get_iter(path)
            model.set_value(treeiter, self.COLUMN_VALUE, pymobius.to_string(attr_value))
        else:
            mobius.core.logf(f'WRN unknown attr_id "{attr_id}"')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief clear attributes
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def clear(self):
        model = self.__listview.get_model()
        model.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief clear values
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def clear_values(self):
        model = self.__listview.get_model()

        for row in model or []:
            row[self.COLUMN_VALUE] = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief return data representation of widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_data(self):
        data = pymobius.Data()

        # store attributes and values
        data.attributes = []
        data.values = {}

        model = self.__listview.get_model()

        for attr_id, attr_name, attr_value, is_editable, color in model:
            attr = pymobius.Data()
            attr.id = attr_id
            attr.name = attr_name
            attr.value = attr_value

            data.attributes.append(attr)
            data.values[attr.id] = attr_value

        return data

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set data of widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, data):
        model = self.__listview.get_model()
        model.clear()

        for attr in data.attributes:
            if attr.value is None:
                value = ''
            else:
                value = str(attr.value)
            model.append((attr.id, attr.name, value, False, None))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief return text representation of widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_text(self):
        model = self.__listview.get_model()
        text = ''

        for attr_id, attr_name, attr_value, is_editable, color in model:
            text += attr_name + '\t' + pymobius.to_string(attr_value) + '\n'

        return text

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set report ID
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_report_id(self, report_id):
        self.__report_id = report_id

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get report ID
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_report_id(self):
        return self.__report_id

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set report name
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_report_name(self, report_name):
        self.__report_name = report_name

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get report name
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_report_name(self):
        return self.__report_name

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set report application
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_report_app(self, report_app):
        self.__report_app = report_app

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get report application
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_report_app(self):
        return self.__report_app

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add export handler
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_export_handler(self, filetype, filename, fileext, handler):
        self.__export_handlers.append((filetype, filename, fileext, handler))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle value edition
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_value_edited(self, cell, path_string, new_value, *args):
        model = self.__listview.get_model()
        treeiter = model.get_iter_from_string(path_string)
        attr_id = model.get_value(treeiter, self.COLUMN_ID)
        old_value = model.get_value(treeiter, self.COLUMN_VALUE)

        if old_value != new_value:
            if not self.__on_editing_value_handler or self.__on_editing_value_handler(attr_id, old_value, new_value):
                model.set_value(treeiter, self.COLUMN_VALUE, new_value)

                if self.__after_editing_value_handler:
                    self.__after_editing_value_handler(attr_id, old_value, new_value)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle on_drag_data_get
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_drag_data_get(self, widget, context, selection, info, utime, *args):

        # build report object
        report = pymobius.Data()
        report.rid = self.__report_id
        report.name = self.__report_name
        report.app = self.__report_app
        report.widget = self.__report_widget

        if self.__on_dnd_get_data_handler:
            report.data = self.__on_dnd_get_data_handler(self)
        else:
            report.data = self.get_data()

        # push object for DND
        uid = self.__mediator.call('ui.dnd-push', report)
        selection.set(selection.get_target(), 8, b'%d' % uid)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle reload data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_reload_data(self, widget, *args):
        self.__on_reload_data_handler(self)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle copy data to clipboard
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_copy_data(self, widget, *args):
        text = self.get_text()

        clipboard = Gtk.Clipboard.get(Gdk.SELECTION_CLIPBOARD)
        clipboard.set_text(text, -1)
        clipboard.store()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle on_export_data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_data(self, widget, *args):

        # build dialog
        fs = Gtk.FileChooserDialog(title='Export data', action=Gtk.FileChooserAction.SAVE)
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        fs.set_do_overwrite_confirmation(True)

        # set current folder
        last_folder = mobius.framework.get_config('general.save-folder')
        if last_folder:
            fs.set_current_folder(last_folder)

        # add export handlers
        first = True

        for filetype, filename, fileext, handler in self.__export_handlers:
            filefilter = Gtk.FileFilter()
            filefilter.handler = handler
            filefilter.extension = fileext
            filefilter.set_name('%s (.%s)' % (filename, fileext))
            filefilter.add_pattern('*.' + fileext)
            fs.add_filter(filefilter)

            # fs.filter = filefilter
            if first:
                fs.set_current_name(self.__report_id + '.' + fileext)
                first = False

        # run dialog
        rc = fs.run()
        uri = fs.get_uri()
        folder = fs.get_current_folder()

        filefilter = fs.get_filter()
        handler = filefilter.handler
        fileext = filefilter.extension

        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        mobius.framework.set_config('general.save-folder', folder)

        # add extension to name
        uri_root, uri_ext = os.path.splitext(uri)

        if uri_ext != '.' + fileext:
            uri = uri + '.' + fileext

        # export
        data = self.get_data()
        handler(uri, data)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle export_txt_handler
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_txt_handler(self, uri, data):
        f = mobius.io.new_file_by_url(uri)
        fp = mobius.io.text_writer(f.new_writer())
        fp.write(self.get_text())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle export_csv_handler
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_csv_handler(self, uri, data):

        # create writer
        f = mobius.io.new_file_by_url(uri)
        fp = mobius.io.text_writer(f.new_writer())

        # store data
        fp.write('id,name,value\r\n')

        for attr in data.attributes:
            row = attr.id, attr.name, attr.value or ''
            fp.write(','.join((csv_escape(c) for c in row)) + '\r\n')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle export_latex_handler
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_latex_handler(self, uri, data):

        # create writer
        f = mobius.io.new_file_by_url(uri)
        fp = mobius.io.text_writer(f.new_writer())

        # store data
        for attr in data.attributes:
            row = attr.id, attr.name, attr.value or ''
            fp.write('&'.join((latex_escape(c) for c in row)) + '\\\\\n')
