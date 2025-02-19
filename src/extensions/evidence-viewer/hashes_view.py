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
# @class HashesView
# @brief Provides a user interface widget for viewing hashes associated with evidence.
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class HashesView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initializes the hashes view widget.
    # @param control The control object for managing the widget.
    # @param args A dictionary containing configuration parameters 
    #             (e.g., 'rows', 'name').
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control, args):
        self.__details_columns = args.get('rows')
        self.__control = control

        mediator = pymobius.mediator.copy()

        self.name = args.get('name') or 'Hashes'
        icon_path = mediator.call('extension.get-resource-path', EXTENSION_ID, 'hashes.png')
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__details_view = mediator.call('ui.new-widget', 'tableview')
        self.__details_view.set_report_id('evidence.hashes')
        self.__details_view.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__details_view.show()
        self.__details_view.set_sensitive(False)
        self.__details_view.set_drag_and_drop(False)
        self.__widget = self.__details_view

        column = self.__details_view.add_column('type', 'Hash Type')
        column.is_sortable = True

        column = self.__details_view.add_column('value', 'Hash Value')
        column.is_sortable = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieves the UI widget for the hashes view.
    # @return A UI widget object representing the hashes view.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Sets the evidence data to be displayed in the hashes view widget.
    # @param evidence The evidence object containing hashes to display. 
    #                 If None, clears and disables the widget.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, evidence):
        self.__details_view.clear()

        if evidence:
            hashes = evidence.hashes or []

            for hash_type, hash_value in hashes:
                self.__details_view.add_row((hash_type.upper(), hash_value))

            self.__details_view.set_report_name(f'Evidence <{evidence.id}> hashes')
            self.__details_view.set_sensitive(True)

        else:
            self.__details_view.set_sensitive(False)
