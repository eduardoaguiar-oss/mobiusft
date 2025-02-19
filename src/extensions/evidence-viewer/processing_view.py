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
import threading
import traceback

import mobius
import pymobius
import pymobius.ant.evidence
from gi.repository import GLib
from gi.repository import Gtk

from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @class ProcessingView
# @brief This class defines the user interface for evidence processing.
#
# The ProcessingView class is responsible for managing how evidence items
# are displayed, selected, and processed in the interface. It provides features
# like listing evidence items, running processing in separate threads, and
# updating user interaction elements such as buttons and views based on the
# current state of the evidence.
#
# Key Responsibilities:
# - Display evidence items in a list view with their status and attributes.
# - Handle user interactions like selecting items and initiating processing.
# - Manage threads for running evidence processing in the background.
# - Maintain internal state for ongoing and completed processes.
#
# The class integrates with the Mobius framework's UI and handling mechanisms.
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ITEM_STATUS, ITEM_PROFILE, ITEM_NAME, ITEM_OBJECT = range(4)

class ProcessingView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Constructor for the ProcessingView class.
    # 
    # This method initializes the processing view widget by setting up its
    # layout, creating various UI elements (e.g., list view, buttons), and
    # preparing it to display evidence items for processing.
    # 
    #  @param control Control object to manage inter-widget interactions.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        mediator = pymobius.mediator.copy()
        self.__control = control
        self.name = 'Evidence Processing View'

        path = mediator.call('extension.get-resource-path', EXTENSION_ID, 'processing-view.png')
        self.icon_data = open(path, 'rb').read()

        # build widget
        self.__widget = mobius.ui.container()
        self.__widget.show()
        
        # vpaned
        vpaned = Gtk.VPaned()
        vpaned.show()
        self.__widget.set_content(vpaned, mobius.ui.box.fill_with_widget)

        # item listview
        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        vpaned.pack1(sw, True, True)

        model = Gtk.ListStore.new([str, str, str, object])

        self.__items_listview = Gtk.TreeView.new_with_model(model)
        self.__items_listview.show()
        sw.add(self.__items_listview)

        selection = self.__items_listview.get_selection()
        selection.connect('changed', self.__on_item_selection_changed)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('Status')
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', ITEM_STATUS)
        self.__items_listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('Profile')
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', ITEM_PROFILE)
        self.__items_listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('Item')
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', ITEM_NAME)
        self.__items_listview.append_column(tvcolumn)

        # processing details
        vbox = mobius.ui.box(mobius.ui.box.orientation_vertical)
        vbox.set_visible(True)
        vbox.add_filler ()  # development only
        vpaned.pack2(vbox.get_ui_widget(), True, True)

        hbox = mobius.ui.box(mobius.ui.box.orientation_horizontal)
        hbox.set_spacing(5)
        hbox.set_visible(True)
        hbox.add_filler ()
        vbox.add_child(hbox, mobius.ui.box.fill_none)

        self.__execute_button = mobius.ui.button()
        self.__execute_button.set_icon_by_name('system-run')
        self.__execute_button.set_text('_Execute')
        self.__execute_button.set_visible(True)
        self.__execute_button.set_sensitive(False)
        self.__execute_button.set_callback('clicked', self.__on_execute_button_clicked)
        hbox.add_child(self.__execute_button, mobius.ui.box.fill_none)

        # panel data
        self.__running_items = {}
        self.__itemlist = []
        self.__item = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get the UI widget of the processing view.
    #
    # This function returns the main UI widget for the processing view, 
    # which can be integrated into a higher-level interface. It provides
    # access to the root widget of the class.
    #
    # @return Gtk.Widget object representing the UI for this view.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed in the processing list view.
    #
    # This function takes a list of evidence items and displays them in the
    # processing view's list. It checks the attributes of each item to assign
    # a status (e.g., Running, No datasource, Completed) and adds it to the
    # list model, which updates the UI accordingly.
    #
    # @param itemlist List of evidence items to display in the processing view.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
        self.__itemlist = itemlist

        model = self.__items_listview.get_model()
        model.clear()

        for item in itemlist:
            profile = item.get_attribute('processing.profile')
            name = item.name
            obj = item

            if item in self.__running_items:
                status = 'Running'

            elif not item.has_datasource():
                status = 'No datasource'

            elif item.has_ant('evidence'):
                status = 'Completed'

            else:
                status = ''

            model.append((status, profile, name, obj))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle item selection
    #
    # This function is triggered when the selection in the item list changes.
    # It retrieves the selected item, updates the internal state, and enables
    # or disables the "Execute" button based on the item's attributes.
    #
    # @param selection The selection object associated with the list view.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_item_selection_changed(self, selection, *args):
        model, treeiter = selection.get_selected()

        # fill info
        if treeiter:
            self.__item = model.get_value(treeiter, ITEM_OBJECT)

            # enable/disable execute button
            if self.__item.has_datasource() and self.__item not in self.__running_items:
                self.__execute_button.set_sensitive(True)
            else:
                self.__execute_button.set_sensitive(False)

        else:
            self.__item = None
            self.__execute_button.set_sensitive(False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handles the "Execute" button click event to process a selected evidence item.
    #
    # When the "Execute" button is clicked, this function triggers the processing
    # of the currently selected evidence item in the list view. If the item has
    # already been processed, a confirmation dialog is shown to the user. 
    # If reprocessing is confirmed, the item's ANT module is reset before proceeding. 
    # A separate thread is then initiated to handle the processing logic.
    #
    # This function also updates the internal state of running items, including
    # disabling the "Execute" button until processing is complete.
    #
    # @exception Exception This function captures and logs any errors that occur
    # during the processing of evidence items. Errors are also displayed as messages
    # to the user.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_execute_button_clicked(self):
        try:
            # Show warning message if item has already been processed
            if self.__item.has_ant('evidence'):
                dialog = mobius.ui.message_dialog(mobius.ui.message_dialog.type_question)
                dialog.text = "You are about to reload evidences. Are you sure?"
                dialog.add_button(mobius.ui.message_dialog.button_yes)
                dialog.add_button(mobius.ui.message_dialog.button_no)
                dialog.set_default_response(mobius.ui.message_dialog.button_no)
                rc = dialog.run()

                if rc != mobius.ui.message_dialog.button_yes:
                    return

                # reset ANT
                transaction = self.__item.case.new_transaction()
                ant = pymobius.ant.evidence.Ant(self.__item)
                ant.reset()
                transaction.commit()

            # Create new thread to process item
            t = threading.Thread(target=self.__thread_begin, args=(self.__item,), daemon=True)
            t.start()
            self.__running_items[self.__item] = t
            self.set_data(self.__itemlist)

        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")
            self.__widget.set_message(str(e))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Entry point for processing an evidence item in a new thread.
    #
    # This function initializes a processing thread to handle the specified 
    # evidence item. The thread guards against concurrency issues and uses the 
    # ANT engine to execute evidence-related tasks. Once processing is complete, 
    # a callback is scheduled on the main thread to finalize the operation.
    #
    # @param item The evidence item to be processed. After starting, 
    #             the function immediately returns control to the caller while 
    #             the processing continues asynchronously.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __thread_begin(self, item):
        guard = mobius.core.thread_guard()
        ant = pymobius.ant.evidence.Ant(item)
        ant.run()

        GLib.idle_add(self.__thread_end, item)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Finalize the processing of an evidence item after its thread has completed.
    #
    # This function is called when the processing thread for an evidence item finishes.
    # It updates the internal state by removing the item from the running items list,
    # refreshing the item view data, and switching the view back to the navigation view.
    #
    # @param item The evidence item whose processing thread has finished.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __thread_end(self, item):
        self.__running_items.pop(item, None)
        self.set_data(self.__itemlist)
        self.__control.select_navigation_view()
