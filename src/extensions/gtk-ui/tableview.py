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
from gi.repository import GObject
from gi.repository import Gdk
from gi.repository import GdkPixbuf
from gi.repository import Gtk

from metadata import *
from common import REPORT_DATA_MIMETYPE
from common import csv_escape
from common import get_handler
from common import latex_escape

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
DATATYPES = {
    'datetime': str,
    'int': GObject.TYPE_LONG,
    'object': object,
    'icon': GdkPixbuf.Pixbuf,
    'str': str,
}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief widget: tableview
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class TableViewWidget(object):
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief column class
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    class Column(object):

        def __init__(self):
            self.is_sortable = False
            self.is_searchable = False
            self.is_visible = True
            self.is_exportable = True
            self.is_markup = False
            self.type = 'str'
            self.idx = -1
            self.id = None
            self.name = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief row class
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    class Row(object):

        def __init__(self, row):
            self.__row = row
            self.path = row.path

        def __setitem__(self, idx, value):
            self.__row[idx] = value

        def __getitem__(self, idx):
            return self.__row[idx]

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__report_id = 'unknown'
        self.__report_name = 'tableview'
        self.__report_widget = 'tableview'
        self.__report_app = f'{EXTENSION_NAME} v{EXTENSION_VERSION}'
        self.__export_handlers = []
        self.__columns = []
        self.__icons = {}
        self.__is_built = False

        # event handlers
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
        self.__listview = Gtk.TreeView.new()
        self.__listview.set_enable_search(True)
        self.__listview.show()
        sw.add(self.__listview)

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
    # @brief iterate through rows
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __iter__(self):
        model = self.__get_model()

        for row in model:
            yield self.Row(row)

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
    # @brief Set widget DND button enable/disable
    # @param flag True/False
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_drag_and_drop(self, flag):
        if flag:
            self.__dnd_toolitem.show()
        else:
            self.__dnd_toolitem.hide()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get toolbar widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_toolbar(self):
        return self.__toolbar

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set control object
    # @param control control object
    # @param wid widget id to be appended to the handler functions' names
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_control(self, control, wid=None):
        self.__on_dnd_get_data_handler = get_handler('on_dnd_get_data', control, wid)

        # event: on reload data
        self.__on_reload_data_handler = get_handler('on_reload_data', control, wid)

        if self.__on_reload_data_handler:
            self.__refresh_toolitem.set_sensitive(True)
            self.__refresh_toolitem.connect("clicked", self.__on_reload_data)

        # event: on selection changed
        self.__on_selection_changed_handler = get_handler('on_selection_changed', control, wid)

        if self.__on_selection_changed_handler:
            selection = self.__listview.get_selection()
            selection.connect('changed', self.__on_selection_changed)

        # event: on row activated
        self.__on_row_activated_handler = get_handler('on_row_activated', control, wid)

        if self.__on_row_activated_handler:
            self.__listview.connect('row-activated', self.__on_row_activated)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add column to widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_column(self, column_id, column_name=None, column_type='str', is_sortable=False):
        column = self.Column()
        column.idx = len(self.__columns)
        column.id = column_id
        column.name = column_name or column_id.capitalize()
        column.type = column_type
        column.is_exportable = column_type not in ('object', 'icon')
        column.is_sortable = column_type not in ('object', 'icon') and is_sortable
        self.__columns.append(column)

        return column

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add data row
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_row(self, row):
        model = self.__get_model()

        mrow = []
        for idx, col in enumerate(self.__columns):
            if col.type == 'icon':
                mrow.append(self.__icons.get(row[idx]))

            elif col.type == 'str':
                mrow.append(pymobius.to_string(row[idx]))

            else:
                mrow.append(row[idx])

        model.append(mrow)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add row from object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_row_from_object(self, obj):
        row = []

        for col in self.__columns:
            if col.id == 'obj':
                value = obj
            else:
                value = getattr(obj, col.id, None)

            row.append(value)

        self.add_row(row)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get selected rows
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_selected_rows(self):
        selection = self.__listview.get_selection()
        model, pathlist = selection.get_selected_rows()
        selection_list = []

        for path in pathlist:
            row_number = path[0]
            row_data = tuple(model[row_number])
            selection_list.append((row_number, row_data))

        return selection_list

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Remove selected rows
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def remove_selected_rows(self):

        # get selected rows
        selection = self.__treeview.get_selection()
        model, pathlist = selection.get_selected_rows()

        # remove rows
        treeiterlist = [model.get_iter(path) for path in pathlist]
        has_next = False

        for treeiter in treeiterlist:
            if model.iter_is_valid(treeiter):
                has_next = model.remove(treeiter)
            else:
                has_next = False

        # select next item
        if has_next:
            selection.select_iter(treeiter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Clear table
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def clear(self):
        model = self.__get_model()
        model.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief return data representation of widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_data(self):
        data = pymobius.Data()

        # store columns
        data.columns = [c for c in self.__columns if c.is_exportable]
        columns_idx = [idx for idx, c in enumerate(self.__columns) if c.is_exportable]

        # store rows
        data.rows = []
        model = self.__get_model()

        for row in model:
            d_row = tuple((row[idx] for idx in columns_idx))
            data.rows.append(d_row)

        return data

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set data of widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, data):
        self.__columns = data.columns[:]

        for idx, c in enumerate(self.__columns):
            c.idx = idx

        # reevaluate model
        self.__listview.set_model(None)
        model = self.__get_model()

        # fill data
        for row in data.rows:
            self.add_row(row)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief return text representation of widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_text(self):
        model = self.__listview.get_model()
        text = ''
        columns = [idx for idx, col in enumerate(self.__columns) if col.is_exportable]

        for row in model:
            text += '\t'.join(pymobius.to_string(row[i]) for i in columns) + '\n'

        return text

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set multiple selection mode
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_multiple_selection(self, flag):
        selection = self.__listview.get_selection()
        selection.set_mode(Gtk.SelectionMode.MULTIPLE if flag else Gtk.SelectionMode.SINGLE)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Select row
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def select_row(self, row_id):
        selection = self.__listview.get_selection()
        selection.select_path((row_id,))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Unselect row
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def unselect_row(self, row_id):
        selection = self.__listview.get_selection()
        selection.unselect_path((row_id,))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set sort column ID
    # @param column_id Column ID, starting from 0
    # @param order_asc True/False
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_sort_column_id(self, column_id, order_asc=True):
        model = self.__get_model()
        model.set_sort_column_id(column_id, Gtk.SortType.ASCENDING if order_asc else Gtk.SortType.DESCENDING)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Create empty model with the same columns as current model
    # @return model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def new_model(self):
        column_types = []

        for column in self.__columns:
            ctype = DATATYPES.get(column.type, str)
            column_types.append(ctype)

        model = Gtk.ListStore.new(column_types)
        return model

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set new model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_model(self, model):
        self.__build_treeview_columns()
        self.__listview.set_model(model)

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
    # @brief Set icon stock
    # @param icon_id Icon ID
    # #param pixbuf Gtk Pixbuf
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_icon(self, icon_id, pixbuf):
        self.__icons[icon_id] = pixbuf

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add export handler
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_export_handler(self, filetype, filename, fileext, handler):
        self.__export_handlers.append((filetype, filename, fileext, handler))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief return listview model, creating it if necessary
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_model(self):

        # if model is built, return it
        model = self.__listview.get_model()
        if model:
            return model

        # create tvcolumns
        self.__build_treeview_columns()

        # create model
        model = self.new_model()
        self.__listview.set_model(model)

        return model

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Build treeview columns on demand
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __build_treeview_columns(self):
        if self.__is_built:
            return

        for column in self.__columns:

            if column.is_visible:
                if column.type == 'icon':
                    renderer = Gtk.CellRendererPixbuf()

                else:
                    renderer = Gtk.CellRendererText()

                    if column.type == 'int':
                        renderer.set_property('xalign', 1.0)

                tvcolumn = Gtk.TreeViewColumn(column.name)
                tvcolumn.set_sizing(Gtk.TreeViewColumnSizing.AUTOSIZE)
                tvcolumn.set_resizable(True)
                tvcolumn.pack_start(renderer, True)

                if column.type == 'icon':
                    tvcolumn.add_attribute(renderer, 'pixbuf', column.idx)

                elif column.is_markup:
                    tvcolumn.add_attribute(renderer, 'markup', column.idx)

                else:
                    tvcolumn.add_attribute(renderer, 'text', column.idx)

                if column.is_sortable:
                    tvcolumn.set_sort_column_id(column.idx)

                if column.is_searchable:
                    self.__listview.set_search_column(column.idx)

                self.__listview.append_column(tvcolumn)

        self.__is_built = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_drag_data_get
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_drag_data_get(self, widget, context, selection, info, utime, *args):

        # build report object
        report = pymobius.Data()
        report.rid = self.__report_id
        report.name = self.__report_name
        report.app = self.__report_app

        if self.__on_dnd_get_data_handler:
            report.data = self.__on_dnd_get_data_handler(self)
            report.widget = 'evidence_view'

        else:
            report.data = self.get_data()
            report.widget = self.__report_widget

        # push object for DND
        uid = self.__mediator.call('ui.dnd-push', report)
        selection.set(selection.get_target(), 8, b"%d" % uid)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle reload data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_reload_data(self, widget, *args):
        self.__on_reload_data_handler(self)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle selection changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_selection_changed(self, selection, *args):
        selection_list = self.get_selected_rows()
        self.__on_selection_changed_handler(selection_list)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle row activated
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_row_activated(self, widget, row, col, *args):
        model = widget.get_model()
        row_number = row[0]
        row_data = tuple(model[row_number])

        self.__on_row_activated_handler(row_number, row_data)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle copy data to clipboard
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_copy_data(self, widget, *args):
        text = self.get_text()

        clipboard = Gtk.Clipboard.get(Gdk.SELECTION_CLIPBOARD)
        clipboard.set_text(text, -1)
        clipboard.store()

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

        # column names
        columns = [(idx, col) for idx, col in enumerate(data.columns) if col.is_exportable]
        fp.write('%s\r\n' % ','.join([csv_escape(col.name) for idx, col in columns]))

        # data
        for row in data.rows:
            fp.write(','.join((csv_escape(pymobius.to_string(row[idx]))) for idx, col in columns) + '\r\n')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle export_latex_handler
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_latex_handler(self, uri, data):

        # create writer
        f = mobius.io.new_file_by_url(uri)
        fp = mobius.io.text_writer(f.new_writer())

        # column names
        columns = [(idx, col) for idx, col in enumerate(data.columns) if col.is_exportable]
        fp.write('&'.join(['\\textbf{%s}' % latex_escape(col.name) for idx, col in columns]) + '\\\\\n')

        # data
        for row in data.rows:
            fp.write('&'.join((latex_escape(pymobius.to_string(row[idx]))) for idx, col in columns) + '\\\\\n')

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
            filefilter.set_name(f'{filename} (.{fileext})')
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
