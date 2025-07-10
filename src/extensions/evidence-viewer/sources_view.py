# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @file hashes_view.py
# @brief Contains the implementation of the HashesView class, which provides 
#        a user interface widget for viewing hashes associated with evidence.
# 
# Mobius Forensic Toolkit
# Copyright (C) 2008-2025 Eduardo Aguiar
# 
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the 
# Free Software Foundation; either version 2, or any later version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
# more details.
# 
# You should have received a copy of the GNU General Public License along 
# with this program. If not, see <http://www.gnu.org/licenses/>.
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import pymobius
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @class SourcesView
# @brief Provides a user interface widget for viewing evidence sources.
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SourcesView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initializes the sources view widget.
    # @param control The control object for managing the widget.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__control = control

        mediator = pymobius.mediator.copy()

        self.name = 'Evidence Sources'
        icon_path = mediator.call('extension.get-resource-path', EXTENSION_ID, 'sources.png')
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__details_view = mediator.call('ui.new-widget', 'tableview')
        self.__details_view.set_report_id('evidence.hashes')
        self.__details_view.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__details_view.show()
        self.__details_view.set_sensitive(False)
        self.__details_view.set_drag_and_drop(False)
        self.__widget = self.__details_view

        column = self.__details_view.add_column('type', 'Type')
        column.is_sortable = True

        column = self.__details_view.add_column('description', 'Description')
        column.is_sortable = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieves the UI widget for the sources view.
    # @return A UI widget object representing the sources view.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Sets the evidence data to be displayed in the sources view widget.
    # @param evidence The evidence object containing sources to display. 
    #                 If None, clears and disables the widget.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, evidence):
        self.__details_view.clear()

        if evidence:
            for source in evidence.get_sources():
                if source.type == 1:  # File
                    type_name = 'File'

                elif source.type == 2:  # Evidence
                    type_name = 'Evidence'

                elif source.type == 3:  # Folder
                    type_name = 'Folder'

                else:
                    type_name = 'Unknown'            

                self.__details_view.add_row((type_name, f"{type_name} {source.description}"))

            self.__details_view.set_report_name(f'Evidence <{evidence.id}> sources')
            self.__details_view.set_sensitive(True)

        else:
            self.__details_view.set_sensitive(False)
