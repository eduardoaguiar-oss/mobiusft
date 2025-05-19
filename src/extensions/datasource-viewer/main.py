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
import mobius
import pymobius
from gi.repository import GdkPixbuf
from gi.repository import Gtk

from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
DS_COMBO_ICON, DS_COMBO_TYPENAME, DS_COMBO_NAME, DS_COMBO_WIDGET = range(4)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief view: datasource
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class DataSourceView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.name = 'Datasource Viewer'
        self.parent = None
        self.__widgets = {}

        icon_path = self.__mediator.call('extension.get-icon-path', EXTENSION_ID)
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.core.ui.container()
        self.__widget.show()

        # regular widget
        self.__vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        self.__vbox.set_border_width(5)
        self.__vbox.set_spacing(10)
        self.__vbox.set_visible(True)

        self.__widget.set_content(self.__vbox)
        self.__widget.set_message('Select one item to view datasource data')

        # build datasource widgets
        widget_data = []

        for resource in mobius.core.get_resources('widget.datasource'):
            widget = resource.value(self)

            loader = GdkPixbuf.PixbufLoader()
            loader.set_size(24, 24)
            loader.write(widget.icon_data)
            loader.close()
            pixbuf = loader.get_pixbuf()

            widget_data.append((widget.name, widget.typename, pixbuf, widget))
            self.__widgets[widget.typename] = widget

        widget_data = [('No datasource', 'null', None, None)] + list(sorted(widget_data))

        # Hbox
        self.__hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        self.__hbox.set_spacing(5)
        self.__hbox.set_visible(True)
        self.__vbox.add_child(self.__hbox, mobius.core.ui.box.fill_none)

        # DND toolitem
        self.__dnd_toolitem = self.__mediator.call('ui.new-widget', 'dnd-toolitem')
        self.__dnd_toolitem.set_control(self)
        self.__dnd_toolitem.show()
        self.__hbox.add_child(self.__dnd_toolitem.get_ui_widget(), mobius.core.ui.box.fill_none)

        # combobox
        datastore = Gtk.ListStore.new([GdkPixbuf.Pixbuf, str, str, object])
        self.__combobox_idx = {}

        for name, typename, pixbuf, widget in widget_data:
            datastore.append((pixbuf, typename, name, widget))

        self.__combobox = Gtk.ComboBox.new_with_model(datastore)
        self.__combobox.set_id_column(DS_COMBO_TYPENAME)
        self.__combobox.connect('changed', self.__on_combobox_selection_changed)
        self.__combobox.show()

        renderer = Gtk.CellRendererPixbuf()
        self.__combobox.pack_start(renderer, False)
        self.__combobox.add_attribute(renderer, 'pixbuf', DS_COMBO_ICON)

        renderer = Gtk.CellRendererText()
        self.__combobox.pack_start(renderer, True)
        self.__combobox.add_attribute(renderer, 'text', DS_COMBO_NAME)
        self.__hbox.add_child(self.__combobox, mobius.core.ui.box.fill_with_widget)

        # working area
        self.__working_area = mobius.core.ui.container()
        self.__working_area.show()
        self.__vbox.add_child(self.__working_area, mobius.core.ui.box.fill_with_widget)

        # view data
        self.__item = None
        self.__datasource = None
        self.__current_widget = None
        self.__populating_data = False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):

        # single selection only
        if len(itemlist) == 1:
            self.__item = itemlist[0]
            self.__populate_data(self.__item.get_datasource())
            self.__widget.show_content()

        # multiple or no selection
        else:
            self.__item = None
            self.__widget.set_message('Select one item to view datasource data')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_destroy event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        if self.__current_widget:
            self.__current_widget.on_destroy()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle datasource modified
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_datasource_modified(self, datasource):
        transaction = self.__item.new_transaction()

        if datasource:
            self.__item.set_datasource(datasource)
        else:
            self.__item.remove_datasource()

        transaction.commit()

        # reload parent widget (view_selector)
        self.parent.reload()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_report_data event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_report_data(self, widget):
        report = pymobius.Data()
        report.rid = 'item.datasource'
        report.name = f'Datasource data of {self.__item.name}'
        report.app = f'{EXTENSION_NAME} v{EXTENSION_VERSION}'
        report.widget = 'internal'
        report.item_uid = self.__item.uid

        report.data = pymobius.Data()
        report.data.datasource_type = 'null'

        if self.__current_widget:
            self.__current_widget.on_report_data(report)

        return report

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate_data(self, datasource):
        self.__datasource = datasource

        # start setting data
        self.__populating_data = True

        # get datasource type
        if not datasource:
            typename = 'null'
        else:
            typename = datasource.get_type()

        # select combobox
        self.__combobox.set_active_id(typename)

        # set working area
        if typename == 'null':
            self.__working_area.hide()
            self.__current_widget = None

        else:
            self.__current_widget = self.__widgets.get(typename)

            if self.__current_widget:
                self.__current_widget.set_data(self.__datasource)
                self.__working_area.set_content(self.__current_widget.get_ui_widget())
                self.__working_area.show()

            else:
                self.__working_area.set_message(f'Unhandled datasource type "{typename}"')

        # end setting data
        self.__populating_data = False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle combobox selection changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_combobox_selection_changed(self, combobox, *args):
        if self.__populating_data:
            return

        # get new typename
        typename = combobox.get_active_id()

        # set working area
        if typename == 'null':
            self.__working_area.hide()
            self.__current_widget = None
            self.on_datasource_modified(None)

        else:
            self.__current_widget = self.__widgets.get(typename)

            if self.__current_widget:
                self.__current_widget.set_data(None)
                self.__working_area.set_content(self.__current_widget.get_ui_widget())
                self.__working_area.show()

            else:
                self.__working_area.set_message(f'Unhandled datasource type "{typename}"')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    mobius.core.add_resource('view.datasource', 'Datasource Viewer view', DataSourceView)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('view.datasource')
