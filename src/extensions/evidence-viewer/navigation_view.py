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
from gi.repository import GdkPixbuf
from gi.repository import Gtk

from common import count_evidences
from common import get_icon_path
from generic_view import GenericView
from metadata import *
from model import MODEL

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
(EVIDENCE_ICON,
 EVIDENCE_TYPE,
 EVIDENCE_NAME,
 EVIDENCE_COUNT,
 EVIDENCE_DESCRIPTION,
 EVIDENCE_VIEW) = range(6)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @class NavigationView
# @brief A GUI view for navigating evidences.
# 
# The NavigationView class provides functionality to display and interact 
# with various evidence types through a combobox and an evidence view.
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class NavigationView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Constructs the NavigationView object.
    # 
    # Initializes the widgets, data model, and loads icons for the evidence types.
    # 
    # @param control A reference to the parent controller for handling events.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__items = []
        self.__control = control
        self.name = 'Evidence Navigation View'

        mediator = pymobius.mediator.copy()
        path = mediator.call('extension.get-resource-path', EXTENSION_ID, 'navigation-view.png')
        self.icon_data = open(path, 'rb').read()

        # Box
        self.__widget = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        self.__widget.show()

        # combobox
        model = Gtk.ListStore.new([GdkPixbuf.Pixbuf, str, str, int, str, object])

        self.__combobox = Gtk.ComboBox.new_with_model(model)
        self.__combobox.connect('changed', self.__on_evidence_type_selection_changed)
        self.__combobox.set_id_column(EVIDENCE_TYPE)
        self.__combobox.show()
        self.__widget.add_child(self.__combobox, mobius.core.ui.box.fill_none)

        renderer = Gtk.CellRendererPixbuf()
        self.__combobox.pack_start(renderer, False)
        self.__combobox.add_attribute(renderer, 'pixbuf', EVIDENCE_ICON)

        renderer = Gtk.CellRendererText()
        self.__combobox.pack_start(renderer, False)
        self.__combobox.add_attribute(renderer, 'markup', EVIDENCE_NAME)

        renderer = Gtk.CellRendererText()
        self.__combobox.pack_start(renderer, False)
        self.__combobox.add_attribute(renderer, 'text', EVIDENCE_COUNT)

        renderer = Gtk.CellRendererText()
        self.__combobox.pack_start(renderer, True)
        self.__combobox.add_attribute(renderer, 'text', EVIDENCE_DESCRIPTION)

        # view container
        self.__container = mobius.core.ui.container()
        self.__container.show()
        self.__widget.add_child(self.__container, mobius.core.ui.box.fill_with_widget)

        # Add evidence rows to listview
        for info in MODEL:
            view_id = info.get('view_id')
            evidence_type = info.get('id')
            name = info.get('name')
            description = info.get('description') or name

            if view_id:
                c = mobius.core.get_resource_value(f"evidence.widget.{view_id}")
                view = c(self)

            else:
                view = GenericView(self, info)

            path = get_icon_path(evidence_type)
            icon = GdkPixbuf.Pixbuf.new_from_file_at_size(path, 32, 32)
            model.append((icon, evidence_type, name, 0, description, view))

        # Add total row to combobox
        mediator = pymobius.mediator.copy()
        path = mediator.call('extension.get-resource-path', EXTENSION_ID, 'total.png')
        icon = GdkPixbuf.Pixbuf.new_from_file_at_size(path, 32, 32)
        model.append((icon, None, '<b>TOTAL</b>', 0, 'Total number of evidences', None))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieves the main UI widget of the NavigationView.
    # 
    # @return The root UI widget for this view.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Sets the data to be displayed in the NavigationView.
    # 
    # Updates the evidence counts in the combobox and refreshes the displayed view.
    # 
    # @param items A list of evidence data items.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, items):
        self.__items = items

        # update combobox count numbers
        totals = count_evidences(items)
        total = 0

        model = self.__combobox.get_model()
        first_type = None

        for row in model:
            evidence_type = row[EVIDENCE_TYPE]
            first_type = first_type or evidence_type
            count = totals.get(evidence_type, 0)
            total += count

            if evidence_type:
                row[EVIDENCE_COUNT] = count

            else:
                row[EVIDENCE_COUNT] = total

        # update current view, if any
        selected_view_id = self.__combobox.get_active_id()

        if selected_view_id:
            self.__update_current_view()

        elif first_type:
            self.__control.on_view_activated(first_type)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Cleans up resources used by the NavigationView.
    # 
    # Calls the on_destroy method for each evidence view, if available.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        for row in self.__combobox.get_model():
            view = row[EVIDENCE_VIEW]

            if view and hasattr(view, 'on_destroy'):
                view.on_destroy()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Activates the specified evidence view.
    # 
    # Selects the combobox row corresponding to the given view ID.
    # 
    # @param view_id The ID of the view to activate.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_view_activated(self, view_id):
        self.__combobox.set_active_id(view_id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handles selection changes in the evidence type combobox.
    # 
    # Updates the currently displayed view based on the selected evidence type.
    # 
    # @param combobox The combobox widget that triggered the event.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_evidence_type_selection_changed(self, combobox, *args):
        self.__update_current_view()
        self.__control.on_view_selected(combobox.get_active_id())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Updates the currently displayed evidence view.
    # 
    # Loads and displays the content for the evidence type selected in the combobox.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __update_current_view(self):
        combo_iter = self.__combobox.get_active_iter()

        if combo_iter:
            model = self.__combobox.get_model()
            view = model.get_value(combo_iter, EVIDENCE_VIEW)
            if view:
                view.set_data(self.__items)
                self.__container.set_content(view.get_ui_widget())
            else:
                self.__container.set_message('Select evidence type')
        else:
            self.__container.set_message('Select evidence type')
