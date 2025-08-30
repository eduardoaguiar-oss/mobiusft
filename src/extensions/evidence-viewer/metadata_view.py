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
import pymobius
import pymobius.ant.evidence

from metadata import *
from model import Getter

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @class MetadataView
# @brief Metadata details viewer.
# @details This class manages the display and interaction with metadata information for an evidence object.
# It provides a table view representation of metadata attributes and values with customizable display options.
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class MetadataView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget.
    # @param control Control object for managing widget UI interactions.
    # @param args Dictionary of arguments containing widget configuration options.
    # @details Initializes the metadata view interface with provided control and arguments.
    # Builds the UI widget and configures its table columns for displaying metadata details.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control, args):
        self.__details_columns = args.get('rows')
        self.__control = control

        mediator = pymobius.mediator.copy()

        self.name = args.get('name') or 'Metadata'
        icon_path = mediator.call('extension.get-resource-path', EXTENSION_ID, 'tableview.png')
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__details_view = mediator.call('ui.new-widget', 'tableview')
        self.__details_view.set_report_id('evidence.details')
        self.__details_view.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__details_view.show()
        self.__details_view.set_sensitive(False)
        self.__details_view.set_drag_and_drop(False)
        self.__widget = self.__details_view

        column = self.__details_view.add_column('name', 'Attribute')
        column.is_sortable = True

        column = self.__details_view.add_column('value', 'Value')
        column.is_sortable = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get UI widget.
    # @return The UI widget associated with the metadata view.
    # @details Provides access to the underlying UI widget of the metadata view to allow interaction or embedding in the application's user interface.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed.
    # @param evidence An evidence object whose metadata will be shown.
    # @details Clears the current metadata view and populates it with the attributes and values
    #          of the provided evidence, including tags and metadata attributes.
    #          Displays the evidence item's name and metadata details in a table format.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, evidence):
        self.__details_view.clear()

        if evidence:

            # attributes
            self.__details_view.add_row(('Evidence UID', evidence.uid))
            self.__details_view.add_row(('Evidence Type', evidence.type))
            self.__details_view.add_row(('Item', evidence.item.name))

            for args in self.__details_columns:
                m_id = args.get('id')
                m_name = args.get('name') or pymobius.id_to_name(m_id)
                m_format = args.get('format') or 'string'

                getter = Getter(m_id, m_format)
                m_value = getter(evidence)

                self.__details_view.add_row((m_name, m_value))

            for name, value in evidence.metadata.get_values():
                self.__details_view.add_row((pymobius.id_to_name(name), pymobius.to_string(value)))

            self.__details_view.add_row(("Tags", ', '.join (sorted(evidence.get_tags ()))))

            self.__details_view.set_report_name(f'Evidence <{evidence.id}> metadata')
            self.__details_view.set_sensitive(True)

        else:
            self.__details_view.set_sensitive(False)
