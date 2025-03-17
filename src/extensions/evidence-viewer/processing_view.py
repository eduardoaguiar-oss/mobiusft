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
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PROFILES = pymobius.ant.evidence.PROFILES

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

        # vbox
        vbox = mobius.ui.box(mobius.ui.box.orientation_vertical)
        vbox.set_visible(True)
        vbox.set_spacing(10)
        vbox.set_border_width(10)
        self.__widget.set_content(vbox)

        # Status box
        hbox = mobius.ui.box(mobius.ui.box.orientation_horizontal)
        hbox.set_spacing(5)
        hbox.set_visible(True)
        vbox.add_child(hbox, mobius.ui.box.fill_none)

        label = mobius.ui.label()
        label.set_markup('<b>Status:</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.ui.box.fill_none)

        self.__status_label = mobius.ui.label()
        self.__status_label.set_visible(True)
        self.__status_label.set_halign(mobius.ui.label.align_left)
        hbox.add_child(self.__status_label, mobius.ui.box.fill_with_widget)

        # Running messages
        self.__running_view = mediator.call('ui.new-widget', 'tableview')
        self.__running_view.set_report_id('evidence.processing')
        self.__running_view.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__running_view.set_sensitive(False)
        self.__running_view.show()
        vbox.add_child(self.__running_view.get_ui_widget(), mobius.ui.box.fill_with_widget)

        column = self.__running_view.add_column('timestamp', 'Date/Time')
        column.is_sortable = True

        self.__running_view.add_column('event', 'Event')

        # Execution box
        hbox = mobius.ui.box(mobius.ui.box.orientation_horizontal)
        hbox.set_spacing(5)
        hbox.set_visible(True)
        vbox.add_child(hbox, mobius.ui.box.fill_none)

        label = mobius.ui.label('Profile:')
        label.set_visible(True)
        hbox.add_child(label, mobius.ui.box.fill_none)

        # Create a ListStore with one string column
        model = Gtk.ListStore(str, str)

        # Create a ComboBox with the ListStore as its model
        self.__profile_combobox = Gtk.ComboBox.new_with_model(model)

        # Create a renderer to display the items
        renderer = Gtk.CellRendererText()
        self.__profile_combobox.pack_start(renderer, True)
        self.__profile_combobox.add_attribute(renderer, "text", 1)
        self.__profile_combobox.set_id_column(0)
        self.__profile_combobox.set_visible(True)
        hbox.add_child(self.__profile_combobox, mobius.ui.box.fill_none)

        hbox.add_filler ()

        self.__execute_button = mobius.ui.button()
        self.__execute_button.set_icon_by_name('system-run')
        self.__execute_button.set_text('_Execute')
        self.__execute_button.set_visible(True)
        self.__execute_button.set_sensitive(False)
        self.__execute_button.set_callback('clicked', self.__on_execute_button_clicked)
        hbox.add_child(self.__execute_button, mobius.ui.box.fill_none)

        # Show initial message
        self.__widget.set_message("Select a case item")

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
                self.__populate_itemlist()

            elif len (itemlist) == 1:
                self.__widget.set_message("Selected item has no datasource")

            else:
                self.__widget.set_message("Selected items have no datasource")

        else:
            self.__widget.set_message("Select a case item")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate panel with itemlist
    #
    # This method populates the item list by categorizing items into running,
    # completed, and not processed states. It updates the status label based on
    # the count of each category and manages the sensitivity of UI elements
    # based on the datasource types associated with the items.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate_itemlist(self):
        self.__populate_status_label()
        # self.__populate_messages()
        self.__populate_profile_combobox()

        self.__widget.show_content()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate status label
    # Update the status label based on the processing state of the selected items.
    # It counts the number of items that are running, completed, and not processed,
    # and updates the label accordingly. For a single item, it displays its status,
    # while for multiple items, it shows the count of items in each state.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate_status_label(self):
        completed = 0
        running = 0
        not_processed = 0

        # Iterate through a list of items that have a data source. It counts
        # how many items are currently running, completed, and not processed.
        # It also tracks the types of data sources and checks if all data sources
        # are of the same type.
        for item in [i for i in self.__itemlist if i.has_datasource()]:

            if item in self.__running_items:
                running += 1

            else:
                if item.has_ant('evidence'):
                    completed += 1
                else:
                    not_processed += 1

        # Update the status label based on the processing state of items in a list.
        # If there is only one item, it sets the label to "Running", "Completed", or "Not processed"
        # based on each counter. If there are multiple items, it displays the count of items
        # in each state: Running, Completed, and Not processed.
        if len(self.__itemlist) == 1:
            if running:
                self.__status_label.set_text("Running")
            elif completed:
                self.__status_label.set_text("Completed")
            else:
                self.__status_label.set_text("Not processed")
        else:
            self.__status_label.set_text(f"{running} Running / {completed} Completed / {not_processed} Not processed")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate profile combobox
    #
    # Populate profile combobox based on the data sources of selected items.
    # It determines the type of data sources, filters available profiles that match the data source type,
    # and sets the last selected profile as active if it exists.
    # The combobox is also set to be sensitive based on whether any profiles are available.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate_profile_combobox(self):
        model = self.__profile_combobox.get_model()
        model.clear()

        # Iterates through selected items, checking the data source type of each item.
        # If a data source is found, it sets the datasource_type to the type of the first data source.
        # If subsequent data sources have a different type, datasource_type is set to '*' indicating mixed types.
        datasource_type = ''

        for item in self.__itemlist:
            datasource = item.get_datasource()

            if datasource:
                if not datasource_type:
                    datasource_type = datasource.get_type()

                else:
                    if datasource_type != datasource.get_type():
                        datasource_type = '*'

        # Process the list of available profiles based on a specified datasource type.
        # It filters profiles that match the prefix derived from the datasource type and
        # populates the profile combobox with these profiles. The last selected profile is also set
        # as active in the combobox, defaulting to the first profile if no active profile is found.
        if datasource_type and datasource_type != '*':
            prefix = datasource_type + '.'
            last_profile = mobius.framework.get_config('evidence.last_profile') or ''

            for profile_id, profile_name in PROFILES:
                if profile_id.startswith(prefix):
                    model.append((profile_id, profile_name))
                    if profile_id == last_profile:
                        self.__profile_combobox.set_active(len(model) - 1)

            if self.__profile_combobox.get_active() == -1:
                self.__profile_combobox.set_active(0)

        # Set profile combobox and execute button sensitivity
        can_run = len(model) > 0
        self.__profile_combobox.set_sensitive(can_run)
        self.__execute_button.set_sensitive(can_run)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handles the "Execute" button click event to process selected items.
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
            # Check if any item in the item list has evidence. If so,
            # it prompts the user with a confirmation dialog asking if they are sure
            # they want to reload evidences. If the user selects 'no', the function
            # returns early without proceeding further.
            if any (item.has_ant('evidence') for item in self.__itemlist):
                dialog = mobius.ui.message_dialog(mobius.ui.message_dialog.type_question)
                dialog.text = "You are about to reload evidences. Are you sure?"
                dialog.add_button(mobius.ui.message_dialog.button_yes)
                dialog.add_button(mobius.ui.message_dialog.button_no)
                dialog.set_default_response(mobius.ui.message_dialog.button_no)
                rc = dialog.run()

                if rc != mobius.ui.message_dialog.button_yes:
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
            for item in self.__itemlist:
                if item.has_datasource() and item not in self.__running_items:
                    t = threading.Thread(target=self.__thread_begin, args=(item, profile_id), daemon=True)
                    t.start()
                    self.__running_items[item] = t

            # Refresh panel data
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
    # @param item The evidence item to be processed.
    # @param profile_id Profile ID to process item
    #
    # After starting, the function immediately returns control to the caller while
    # the processing continues asynchronously.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __thread_begin(self, item, profile_id):
        guard = mobius.core.thread_guard()

        ant = pymobius.ant.evidence.Ant(item, profile_id)
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
        self.__running_items.pop(item, None)
        self.set_data(self.__itemlist)
        self.__control.select_navigation_view()
