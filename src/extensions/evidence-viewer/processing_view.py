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
from gi.repository import GObject

from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
REFRESH_INTERVAL_MS = 1000

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
        self.__control = control
        self.name = 'Evidence Processing View'

        mediator = pymobius.mediator.copy()
        path = mediator.call('extension.get-resource-path', EXTENSION_ID, 'processing-view.png')
        self.icon_data = open(path, 'rb').read()

        # build widget
        self.__widget = mobius.core.ui.container()
        self.__widget.show()

        # vbox
        vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbox.set_visible(True)
        vbox.set_spacing(10)
        vbox.set_border_width(10)
        self.__widget.set_content(vbox)

        # Items Status tableview
        self.__status_tableview = mediator.call('ui.new-widget', 'tableview')
        self.__status_tableview.set_report_id('evidence.items-status')
        self.__status_tableview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__status_tableview.set_multiple_selection(True)
        self.__status_tableview.show()

        column = self.__status_tableview.add_column('status', 'Status')
        column.is_sortable = True
        
        self.__status_tableview.add_column('uid', 'UID', column_type='int')
        column.is_sortable = True
        
        self.__status_tableview.add_column('name', 'Item Name')
        self.__status_tableview.add_column('object', column_type='object')
        
        self.__status_tableview.set_sort_column_id(1)

        vbox.add_child(self.__status_tableview.get_ui_widget(), mobius.core.ui.box.fill_with_widget)

        # Processing status tableview
        self.__running_view = mediator.call('ui.new-widget', 'tableview')
        self.__running_view.set_report_id('evidence.processing-status')
        self.__running_view.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__running_view.set_sensitive(False)
        self.__running_view.show()

        self.__running_view.add_column('data', 'Data')
        self.__running_view.add_column('event', 'Value')

        vbox.add_child(self.__running_view.get_ui_widget(), mobius.core.ui.box.fill_with_widget)

        # Execution box
        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(5)
        hbox.set_visible(True)
        vbox.add_child(hbox, mobius.core.ui.box.fill_none)

        label = mobius.core.ui.label('Profile:')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        # Create a ListStore with one string column
        model = Gtk.ListStore(str, str)

        # Create a ComboBox with the ListStore as its model
        self.__profile_combobox = Gtk.ComboBox.new_with_model(model)
        model = self.__profile_combobox.get_model()

        for profile in mobius.framework.list_case_profiles():
            model.append((profile.get_id(), profile.get_name()))

        # Create a renderer to display the items
        renderer = Gtk.CellRendererText()
        self.__profile_combobox.pack_start(renderer, True)
        self.__profile_combobox.add_attribute(renderer, "text", 1)
        self.__profile_combobox.set_id_column(0)
        self.__profile_combobox.set_visible(True)
        hbox.add_child(self.__profile_combobox, mobius.core.ui.box.fill_none)

        hbox.add_filler ()

        self.__execute_button = mobius.core.ui.button()
        self.__execute_button.set_icon_by_name('system-run')
        self.__execute_button.set_text('_Execute')
        self.__execute_button.set_visible(True)
        self.__execute_button.set_sensitive(False)
        self.__execute_button.set_callback('clicked', self.__on_execute_button_clicked)
        hbox.add_child(self.__execute_button, mobius.core.ui.box.fill_none)

        # Show initial message
        self.__widget.set_message("Select a case item")

        # panel data
        self.__running_items = {}
        self.__itemlist = []
        self.__watched_item = None

        # Set widget callbacks
        self.__status_tableview.set_control(self, 'status_tableview')

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
    # Summary: This method sets the item list for the view and updates the
    # widget message based on the contents of the item list. It checks if the
    # item list is empty, if any items have a data source, or if the selected
    # items lack data sources, and updates the widget message accordingly.
    #
    # @param itemlist List of evidence items to display in the processing view.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
        self.__itemlist = itemlist

        if itemlist:
            if any(i.has_datasource() for i in itemlist):
                self.__populate_status_tableview()

            elif len (itemlist) == 1:
                self.__widget.set_message("Selected item has no datasource")

            else:
                self.__widget.set_message("Selected items have no datasource")

        else:
            self.__widget.set_message("Select a case item")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate status tableview with itemlist
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate_status_tableview(self):
        self.__status_tableview.clear()
        self.__running_view.clear()
        
        for item in self.__itemlist:
            status = self.__get_item_status(item)
            self.__status_tableview.add_row((status, item.uid, item.name, item))            

        self.__status_tableview.select_row(0)
        self.__widget.show_content()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle status tableview selection change event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def status_tableview_on_selection_changed(self, selected_rows):
        self.__on_status_tableview_updated()
        self.__running_view.set_sensitive(True)
        
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get item status
    # @param item Item to get status
    # @return Item status
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_item_status(self, item):
        if item in self.__running_items:
            return 'Running'
            
        elif item.has_ant('evidence'):
            return 'Completed'
        
        elif not item.has_datasource():
            return 'No datasource'
            
        else:
            return 'Not processed'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update item status in the itemlist
    # @param item Item to update
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __update_item_status(self, item):
        for row in self.__status_tableview:
            if row[3] == item:
                row[0] = self.__get_item_status(item)

        self.__on_status_tableview_updated()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update running view with ANT status metadata
    # @param ant ANT instance
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __update_running_view(self, ant):
        metadata = ant.get_status()
        self.__running_view.clear()
        
        for key, value in metadata.get_values():
            self.__running_view.add_row((pymobius.id_to_name(key), str(value)))
        
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update panel when status tableview selection changes
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_status_tableview_updated(self):
        selected_rows = self.__status_tableview.get_selected_rows()
        selected_items = [row[3] for (row_id, row) in selected_rows]
        can_run_items = [item for item in selected_items if item.has_datasource() and item not in self.__running_items]
        
        # Select profile according to items that can run
        profile_id = None
        profile_ids = set(item.get_attribute('profile_id') for item in can_run_items if item.has_attribute('profile_id'))

        if len(profile_ids) == 1:
            profile_id = profile_ids.pop()

        else:
            profile_id = mobius.framework.get_config('evidence.last_profile')
            
        if profile_id:
            self.__profile_combobox.set_active_id(profile_id)
                    
        # Set profile combobox and execute button sensitivity
        can_run = len(can_run_items) > 0
        self.__execute_button.set_sensitive(can_run)
        self.__profile_combobox.set_sensitive(can_run)

        # Refresh running view
        if len(selected_items) == 1 and selected_items[0] in self.__running_items:
            old_watched_item = self.__watched_item

            if not self.__watched_item or self.__watched_item != selected_items[0]:
                old_watched_item = self.__watched_item
                self.__watched_item = selected_items[0]
                
                if not old_watched_item:
                    GLib.timeout_add(REFRESH_INTERVAL_MS, self.__on_running_timer_interval)

        else:
            self.__watched_item = None
            
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle timer interval event and update running view.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_running_timer_interval(self):

        # Get running ANT
        if not self.__watched_item:
            return False

        data = self.__running_items.get(self.__watched_item, None)
        if not data:
            return False

        t, ant = data
        
        # Update running view with ANT status metadata
        self.__update_running_view(ant)
        
        # Continue updating if still running
        return True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle the "Execute" button click event to process selected items.
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
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_execute_button_clicked(self):
        try:
            # Check if any item in the item list has evidence. If so,
            # it prompts the user with a confirmation dialog asking if they are sure
            # they want to reload evidences. If the user selects 'no', the function
            # returns early without proceeding further.
            selected_rows = self.__status_tableview.get_selected_rows()
            selected_items = [row[3] for (row_id, row) in selected_rows]
            can_run_items = [item for item in selected_items if item.has_datasource() and item not in self.__running_items]
                              
            if any (item.has_ant('evidence') for item in can_run_items):
                dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
                dialog.text = "You are about to reload evidences. Are you sure?"
                dialog.add_button(mobius.core.ui.message_dialog.button_yes)
                dialog.add_button(mobius.core.ui.message_dialog.button_no)
                dialog.set_default_response(mobius.core.ui.message_dialog.button_no)
                rc = dialog.run()

                if rc != mobius.core.ui.message_dialog.button_yes:
                    return

            # Retrieve the active profile ID from a combobox, checks if it exists,
            # creates a new transaction, updates the configuration with the last profile ID,
            # and commits the transaction.
            profile_id = self.__profile_combobox.get_active_id()

            if profile_id:
                transaction = mobius.framework.new_config_transaction()
                mobius.framework.set_config('evidence.last_profile', profile_id)
                transaction.commit()

            # Iterate over selected items and start a new thread for each item that has data source,
            # using the __thread_begin method. Each thread is set as a daemon,
            # meaning it will not prevent the program from exiting. The running threads are
            # stored in the running_items dictionary with the corresponding item as the key.
            for item in can_run_items:
                ant = pymobius.ant.evidence.Ant(item, profile_id)

                t = threading.Thread(target=self.__thread_begin, args=(ant, item), daemon=True)
                t.start()
                
                self.__running_items[item] = (t, ant)
                self.__update_item_status(item)

        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")
            self.__widget.set_message(str(e))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Entry point for processing an evidence item in a new thread.
    # @param ant The ANT instance responsible for processing the evidence item.
    # @param item The evidence item to be processed.
    #
    # This function initializes a processing thread to handle the specified 
    # evidence item. The thread guards against concurrency issues and uses the 
    # ANT engine to execute evidence-related tasks. Once processing is complete, 
    # a callback is scheduled on the main thread to finalize the operation.
    #
    # After starting, the function immediately returns control to the caller while
    # the processing continues asynchronously.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __thread_begin(self, ant, item):
        guard = mobius.core.thread_guard()

        ant.reset()
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
        t, ant = self.__running_items.pop(item, [None, None])
        self.__update_running_view(ant)
        self.__update_item_status(item)
        self.__control.on_processing_end()
