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
from gi.repository import GdkPixbuf
from gi.repository import Gtk

from common import count_evidences
from common import get_icon_path
from metadata import *
from model import MODEL

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
(IV_ICON,
 IV_TYPE,
 IV_NAME,
 IV_LABEL,
 IV_TOOLTIP) = range(5)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @class IconView
# @brief Represents an icon-based view for listing evidences.
# The IconView class displays evidences using icons with labels and tooltips.
# It allows users to select or activate different views for evidences.
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class IconView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initializes the IconView widget and sets up its components.
    # @param control An object that mediates the control over this view.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__category_icons = {}
        self.__control = control
        self.name = 'Evidence Icon View'

        mediator = pymobius.mediator.copy()
        path = mediator.call('extension.get-resource-path', EXTENSION_ID, 'icon-view.png')
        self.icon_data = open(path, 'rb').read()

        # build widget
        self.__widget = Gtk.ScrolledWindow()
        self.__widget.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)

        self.__model = Gtk.ListStore.new([GdkPixbuf.Pixbuf, str, str, str, str])
        self.__model.set_sort_column_id(IV_NAME, Gtk.SortType.ASCENDING)

        self.__iconview = Gtk.IconView.new_with_model(self.__model)
        self.__iconview.set_pixbuf_column(IV_ICON)
        self.__iconview.set_markup_column(IV_LABEL)
        self.__iconview.set_tooltip_column(IV_TOOLTIP)
        self.__iconview.set_columns(-1)
        self.__iconview.set_selection_mode(Gtk.SelectionMode.SINGLE)
        self.__iconview.connect('item-activated', self.__on_icon_activated)
        self.__iconview.show()
        self.__widget.add(self.__iconview)

        # Add evidence icons
        for idx, info in enumerate(MODEL):
            evidence_type = info.get('id')
            name = info.get('name')
            description = info.get('description') or name
            label = name + '\n(0)'

            path = get_icon_path(evidence_type)
            icon = GdkPixbuf.Pixbuf.new_from_file_at_size(path, 64, 64)
            tooltip = description

            self.__model.append((icon, evidence_type, name, label, tooltip))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieves the UI widget for the icon view.
    # @return Gtk.Widget The widget containing the icon view.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Updates the IconView with new data.
    # @param itemlist A list of items for which the evidence count needs to be displayed.
    # Updates the labels in the view with the item count for each evidence.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
        totals = count_evidences(itemlist)

        for row in self.__model:
            evidence_type = row[IV_TYPE]
            name = row[IV_NAME]
            count = totals.get(evidence_type, 0)
            row[IV_LABEL] = f"{name}\n({count})"

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Highlights the view in the IconView corresponding to the view ID.
    # @param view_id A string representing the ID of the view to select.
    # If the view ID matches an item, it gets selected in the IconView.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def select_view(self, view_id):
        for row in self.__model:
            if row[IV_TYPE] == view_id:
                self.__iconview.select_path(row.path)
                return

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handles the event when an icon is activated in the IconView.
    # @param icon_view Gtk.IconView The icon view where the activation occurred.
    # @param path str The path to the activated item.
    # @param args Additional arguments related to the activation event.
    # Calls the control's on_view_activated method with the activated view ID.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_icon_activated(self, icon_view, path, *args):
        view_id = self.__model[path][IV_TYPE]
        self.__control.on_view_activated(view_id)
