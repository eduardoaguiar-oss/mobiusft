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
import pymobius
import pymobius.datasource.ufdr.case_info
from gi.repository import Gtk

from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: UFDRFileInfoView
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class UFDRFileInfoView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__mediator = pymobius.mediator.copy()
        self.__control = control
        self.__itemlist = []
        self.name = 'UFDR File Info'

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'ufdr-info.png')
        self.icon_data = open(path, 'rb').read()

        # vbox
        vbox = mobius.ui.box(mobius.ui.box.orientation_vertical)
        vbox.set_spacing(5)
        vbox.set_visible(True)
        self.__widget = vbox

        # grid
        grid = Gtk.Grid.new()
        grid.set_row_spacing(10)
        grid.set_column_spacing(5)
        grid.show()
        vbox.add_child(grid, mobius.ui.box.fill_none)

        label = mobius.ui.label()
        label.set_markup('<b>UFDR file location</b>')
        label.set_halign(mobius.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 0, 1, 1)

        self.__ufdr_file_button = Gtk.FileChooserButton(title="Select UFDR report file")
        self.__ufdr_file_button.set_hexpand(True)
        self.__ufdr_file_button.set_action(Gtk.FileChooserAction.OPEN)
        # self.__ufdr_file_button.set_current_folder (self.__iped_path or '/opt')
        self.__ufdr_file_button.connect('file-set', self.__on_ufdr_file_set)
        self.__ufdr_file_button.show()
        grid.attach(self.__ufdr_file_button, 1, 0, 4, 1)

        # UFDR metadata view
        self.__metadata_view = self.__mediator.call('ui.new-widget', 'tableview')
        self.__metadata_view.set_report_id('datasource-ufdr.file-info')
        self.__metadata_view.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
        self.__metadata_view.show()
        vbox.add_child(self.__metadata_view.get_ui_widget(), mobius.ui.box.fill_with_widget)

        column = self.__metadata_view.add_column('name', 'Attribute')
        column.is_sortable = True

        column = self.__metadata_view.add_column('value', 'Value')
        column.is_sortable = True

        # Data
        self.__datasource = None
        self.__populating_data = False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_destroy event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        pass

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, datasource):
        self.__datasource = datasource
        self.__populate_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate_data(self):
        self.__populating_data = True
        self.__metadata_view.clear()

        if self.__datasource:
            self.__ufdr_file_button.set_uri(self.__datasource.get_url())

            for name, value in self.__datasource.get_file_info().get_values():
                self.__metadata_view.add_row((pymobius.id_to_name(name), pymobius.to_string(value)))

            self.__metadata_view.set_sensitive(True)

        else:
            self.__ufdr_file_button.unselect_all()
            self.__metadata_view.set_sensitive(False)

        self.__populating_data = False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle 'file-set' filechooserbutton event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_ufdr_file_set(self, filechooserbutton, *args):
        if self.__populating_data:
            return

        url = filechooserbutton.get_uri()
        self.__datasource = mobius.core.datasource.new_datasource_by_ufdr_url(url)

        ant = pymobius.datasource.ufdr.case_info.UFDRCaseInfo(self.__datasource)
        ant.run()

        self.__control.on_datasource_modified(self.__datasource)
        self.__populate_data()
