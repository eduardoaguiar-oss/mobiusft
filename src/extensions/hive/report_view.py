# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024 Eduardo Aguiar
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
import pymobius.mediator
from gi.repository import GdkPixbuf
from gi.repository import Gtk

from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
TREEVIEW_ICON, TREEVIEW_TEXT, TREEVIEW_DATA = range(3)
TREEVIEW_COLUMN_TYPES = (GdkPixbuf.Pixbuf, str, object)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Report widget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ReportView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__registry = None
        self.__registry_version = 0

        # view data
        self.name = 'Reports'
        icon_path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'report.png')
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.ui.container()
        self.__widget.show()

        # hpaned
        self.__hpaned = Gtk.HPaned()
        self.__hpaned.set_sensitive(False)
        self.__hpaned.show()
        self.__widget.set_content(self.__hpaned)

        position = mobius.framework.get_config('hive.report-hpaned-position')
        if position:
            self.__hpaned.set_position(position)

        # treeview
        frame = Gtk.Frame()
        frame.show()
        self.__hpaned.pack1(frame, True, True)

        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        frame.add(sw)

        self.__model = Gtk.TreeStore.new(TREEVIEW_COLUMN_TYPES)

        self.__treeview = Gtk.TreeView.new_with_model(self.__model)
        self.__treeview.show()
        sw.add(self.__treeview)

        selection = self.__treeview.get_selection()
        selection.connect('changed', self.__on_treeview_selection_changed)

        self.__treeview.set_headers_visible(False)
        self.__treeview.set_enable_tree_lines(True)

        tvcolumn = Gtk.TreeViewColumn()
        tvcolumn.set_sizing(Gtk.TreeViewColumnSizing.AUTOSIZE)
        self.__treeview.append_column(tvcolumn)

        renderer = Gtk.CellRendererPixbuf()
        tvcolumn.pack_start(renderer, False)
        tvcolumn.add_attribute(renderer, 'pixbuf', TREEVIEW_ICON)

        renderer = Gtk.CellRendererText()
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', TREEVIEW_TEXT)

        self.__group_nodes = {}

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'report-os.png')
        image = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_toolbar)
        icon = image.get_ui_widget().get_pixbuf()
        treeiter = self.__model.append(None, [icon, 'Operating System', None])
        self.__group_nodes['os'] = self.__model.get_path(treeiter)

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'report-computer.png')
        image = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_toolbar)
        icon = image.get_ui_widget().get_pixbuf()
        treeiter = self.__model.append(None, [icon, 'Computer', None])
        self.__group_nodes['computer'] = self.__model.get_path(treeiter)

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'report-user.png')
        image = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_toolbar)
        icon = image.get_ui_widget().get_pixbuf()
        treeiter = self.__model.append(None, [icon, 'User', None])
        self.__group_nodes['user'] = self.__model.get_path(treeiter)

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'report-app.png')
        image = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_toolbar)
        icon = image.get_ui_widget().get_pixbuf()
        treeiter = self.__model.append(None, [icon, 'Application', None])
        self.__group_nodes['app'] = self.__model.get_path(treeiter)

        # report area
        self.__report_area = Gtk.Frame()
        self.__report_area.show()
        self.__hpaned.pack2(self.__report_area, False, True)
        self.__set_report_area_widget(None)

        # generic report icon
        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'report.png')
        image = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_toolbar)
        self.report_icon = image.get_ui_widget().get_pixbuf()

        # populate reports
        for resource in sorted(mobius.core.get_resources('registry.report'), key=lambda r: r.value.name):
            self.add_report(resource.value)

        # connect to events
        self.__event_uid_1 = mobius.core.subscribe('resource-added', self.__on_resource_added)
        self.__event_uid_2 = mobius.core.subscribe('resource-removed', self.__on_resource_removed)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, registry):
        self.__registry = registry
        self.__registry_version += 1

        if registry:
            self.__hpaned.set_sensitive(True)
            selection = self.__treeview.get_selection()
            self.__on_treeview_selection_changed(selection)

        else:
            self.__set_report_area_widget(None)
            self.__hpaned.set_sensitive(False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Add report to widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_report(self, report):
        report.version = -1
        report.viewer = None

        group_treepath = self.__group_nodes.get(report.group)
        if not group_treepath:
            raise Exception("Invalid report group: %s" % report.group)

        group_treeiter = self.__model.get_iter(group_treepath)
        self.__model.append(group_treeiter, [self.report_icon, report.name, report])
        self.__treeview.expand_row(group_treepath, False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Remove report from widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def remove_report(self, report_id):

        # transverse through treemodel
        def walk(model, treeiter=None):
            if treeiter:
                yield treeiter

            treeiter = model.iter_children(treeiter)
            while treeiter:
                for c in walk(model, treeiter):
                    yield c
                treeiter = model.iter_next(treeiter)

        # remove report by id
        for treeiter in walk(self.__model):
            report = self.__model.get_value(treeiter, TREEVIEW_DATA)

            if report and report.id == report_id:
                self.__model.remove(treeiter)
                return

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save current state
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        mobius.framework.set_config('hive.report-hpaned-position', self.__hpaned.get_position())
        mobius.core.unsubscribe(self.__event_uid_1)
        mobius.core.unsubscribe(self.__event_uid_2)
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set report area widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __set_report_area_widget(self, widget=None):
        for child in self.__report_area.get_children():
            self.__report_area.remove(child)

        if not widget:
            widget = mobius.ui.label('Select report')
            widget.set_visible(True)

        if isinstance(widget, mobius.ui.widget):
            self.__report_area.add(widget.get_ui_widget())

        else:
            self.__report_area.add(widget)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle treeview->on-selection-changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_treeview_selection_changed(self, selection, *args):
        model, treeiter = selection.get_selected()

        # get report selected
        ui_widget_viewer = None

        if treeiter:
            report = self.__model.get_value(treeiter, TREEVIEW_DATA)

            if report:
                if not report.viewer:
                    report.build_viewer()
                    report.viewer.show()

                if report.version < self.__registry_version:
                    report.generate(self.__registry)
                    report.version = self.__registry_version

                report.viewer.set_sensitive(True)
                ui_widget_viewer = report.viewer.get_ui_widget()
                ui_widget_viewer.set_sensitive(True)
                ui_widget_viewer.show()

        # add widget to report area
        self.__report_area.set_sensitive(True)
        self.__set_report_area_widget(ui_widget_viewer)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle resource-added event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_resource_added(self, resource_id, resource):
        if resource_id.startswith('registry.report.'):
            self.add_report(resource.value)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle resource-removed event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_resource_removed(self, resource_id):
        if resource_id.startswith('registry.report.'):
            item_id = resource_id.rsplit('.', 1)[1]
            self.remove_report(item_id)
