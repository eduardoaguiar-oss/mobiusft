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
import mobius
import mobius.framework
import pymobius
from gi.repository import Gtk

from common import get_icon_path
from metadata import *
from hashes_view import HashesView
from metadata_view import MetadataView
from model import Getter
from sources_view import SourcesView


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Generic evidence viewer
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class GenericView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control, args):
        mediator = pymobius.mediator.copy()
        self.__evidence_type = args.get('id')
        self.__evidence_name = args.get('name')
        self.__details_columns = args.get('detail_views')[0].get('rows')
        self.__itemlist = []

        self.id = self.__evidence_type
        self.name = self.__evidence_name
        icon_path = get_icon_path(self.__evidence_type)
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.core.ui.container()
        self.__widget.set_visible(True)

        # vpaned
        self.__vpaned = Gtk.VPaned()
        self.__vpaned.show()
        self.__widget.set_content(self.__vpaned)

        pos = mobius.framework.get_config(f'evidence.{self.__evidence_type}.vpaned-pos')
        if pos:
            self.__vpaned.set_position(pos)

        # master view selector
        self.__master_view_selector = mediator.call('ui.new-widget', 'view-selector')
        self.__master_view_selector.set_control(self, 'master_view')
        self.__master_view_selector.show()
        self.__vpaned.pack1(self.__master_view_selector.get_ui_widget(), False, True)

        # build master views
        for idx, view_args in enumerate(args.get('master_views', []), 1):
            widget_id = view_args.get('id')

            if widget_id == 'table':
                view_class = TableView

            else:
                view_class = mobius.core.get_resource_value(f"widget.evidence.master.{widget_id}")

            view = view_class(self, self.__evidence_type, self.__evidence_name, view_args)
            self.__master_view_selector.add(f"view-{idx}", view)

        # detail view selector
        self.__detail_view_selector = mediator.call('ui.new-widget', 'view-selector')
        self.__detail_view_selector.set_control(self, 'detail_view')
        self.__detail_view_selector.show()
        self.__vpaned.pack2(self.__detail_view_selector.get_ui_widget(), True, True)

        # build details views
        for idx, view_args in enumerate(args.get('detail_views', []), 1):
            widget_id = view_args.get('id')

            if widget_id == 'metadata':
                view = MetadataView(self, view_args)
                self.__detail_view_selector.add(f"view-{idx}", view)

            elif widget_id == 'hashes':
                view = HashesView(self, view_args)
                self.__detail_view_selector.add(f"view-{idx}", view)

            else:
                raise Exception(f"Invalid detail view widget: {widget_id}")

        # Add sources view
        self.__source_view = SourcesView(self)
        self.__detail_view_selector.add(f"sources", self.__source_view)

        # set initial message
        self.__widget.set_message(f'Select item(s) to view {self.__evidence_name}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
        self.__itemlist = itemlist
        evidences = []

        for item in itemlist:
            evidences += item.get_evidences(self.__evidence_type)

        self.__master_view_selector.set_data(evidences)
        self.__widget.show_content()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save current state
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        mobius.framework.set_config(f'evidence.{self.__evidence_type}.vpaned-pos', self.__vpaned.get_position())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle tableview selection-changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_evidence_selection_changed(self, evidence):
        self.__detail_view_selector.set_data(evidence)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_report_data event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def master_view_on_report_data(self, widget):
        report = pymobius.Data()
        report.rid = f"bookmark.evidence-{self.__evidence_type}"
        report.name = self.__evidence_name
        report.app = f'{EXTENSION_NAME} v{EXTENSION_VERSION}'
        report.widget = 'custom'

        report.data = pymobius.Data()
        report.data.evidence_type = self.__evidence_type
        report.data.uid_list = [item.uid for item in self.__itemlist]

        return report


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Master View: table
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class TableView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control, evidence_type, evidence_name, args):
        self.__control = control
        self.__evidence_type = evidence_type
        self.__evidence_name = evidence_name
        columns = args.get('columns', [])

        self.name = args.get('name') or 'Table view'
        mediator = pymobius.mediator.copy()
        icon_path = mediator.call('extension.get-resource-path', EXTENSION_ID, 'tableview.png')
        self.icon_data = open(icon_path, 'rb').read()

        # build tableview
        self.__tableview = mediator.call('ui.new-widget', 'tableview')
        self.__tableview.set_report_id(f"evidence.{self.__evidence_type}-list")
        self.__tableview.set_report_name(self.__evidence_name)
        self.__tableview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__tableview.set_control(self)
        self.__tableview.set_drag_and_drop(False)
        self.__widget = self.__tableview

        # generate getters
        self.__getters = []

        for col_args in columns:
            col_id = col_args.get('id')
            col_format = col_args.get('format')

            g = Getter(col_id, col_format)
            self.__getters.append(g)

        # add alert column
        alert_column = self.__tableview.add_column('alert', '', column_type='icon')
        alert_column.is_visible = True
        alert_column.is_sortable = True

        # add columns to tableview
        self.__first_sortable = -1

        for idx, col_args in enumerate(columns, 1):
            col_id = col_args.get('id')
            col_name = col_args.get('name') or pymobius.id_to_name(col_id)
            col_type = col_args.get('type') or 'str'
            col_is_visible = col_args.get('is_visible', True)
            col_is_sortable = col_args.get('is_sortable')
            col_is_first_sortable = col_args.get('first_sortable')
            col_is_markup = col_args.get('is_markup')
            col_is_exportable = col_args.get('is_exportable')

            # create tableview column
            column = self.__tableview.add_column(col_id, col_name, column_type=col_type)
            column.is_visible = col_is_visible
            column.is_sortable = col_is_sortable or col_is_first_sortable or False

            if col_is_markup is not None:
                column.is_markup = col_is_markup

            if col_is_exportable is not None:
                column.is_exportable = col_is_exportable

            # try to infer which column is the first to be sorted
            if col_is_first_sortable:
                self.__first_sortable = idx

            elif self.__first_sortable == -1 and col_is_sortable:
                self.__first_sortable = idx

        # add object column
        column = self.__tableview.add_column('obj', 'object', column_type='object')
        column.is_visible = False

        # add exporters
        exporters = args.get('exporters', [])

        for exporter_args in exporters:
            exporter_id = exporter_args.get('id')
            exporter_name = exporter_args.get('name')
            exporter_ext = exporter_args.get('extensions')
            exporter_f = exporter_args.get('function')
            self.__tableview.add_export_handler(exporter_id, exporter_name, exporter_ext, exporter_f)

        # add alert icon
        image = mobius.core.ui.new_icon_by_name('alert', mobius.core.ui.icon.size_toolbar)
        self.__alert_icon = image.get_ui_widget().get_pixbuf()
        self.__tableview.set_icon(evidence_type, self.__alert_icon)

        # show widget
        self.__tableview.show()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, evidences):
        model = self.__tableview.new_model()

        for e in evidences:
            icon = self.__alert_icon if e.has_tag('alert') else None
            row = [icon] + [g(e) for g in self.__getters] + [e]
            model.append(row)

        if self.__first_sortable != -1:
            model.set_sort_column_id(self.__first_sortable, Gtk.SortType.ASCENDING)

        self.__tableview.set_model(model)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle tableview selection-changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_selection_changed(self, selection_list):

        if selection_list:
            row_id, row_data = selection_list[0]
            evidence = row_data[-1]

        else:
            evidence = None

        self.__control.on_evidence_selection_changed(evidence)
