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

        # Add some items to the ListStore
        model.append(["vfs.general", "General (fastest)"])
        model.append(["vfs.pedo", "Pedo (slow)"])

        # Create a ComboBox with the ListStore as its model
        self.__profile_combobox = Gtk.ComboBox.new_with_model(model)

        # Create a renderer to display the items
        renderer = Gtk.CellRendererText()
        self.__profile_combobox.pack_start(renderer, True)
        self.__profile_combobox.add_attribute(renderer, "text", 1)
        self.__profile_combobox.set_active(0)
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
    # This function takes a list of evidence items and displays them in the
    # processing view's list. It checks the attributes of each item to assign
    # a status (e.g., Running, No datasource, Completed) and adds it to the
    # list model, which updates the UI accordingly.
    #
    # @param itemlist List of evidence items to display in the processing view.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
        self.__itemlist = itemlist

        if not itemlist:
            self.__widget.set_message("Select a case item")

        elif len (itemlist) == 1:
            if itemlist[0].has_datasource ():
                self.__populate_item(itemlist[0])
            else:
                self.__widget.set_message("Selected item has no datasource")

        else:
            if any(i.has_datasource () for i in itemlist):
                self.__populate_itemlist(itemlist)

            else:
                self.__widget.set_message("Selected items have no datasource")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populates the panel with the status of the specified case item in the processing view.
    #
    # This method checks the current state of the provided item and updates the status label
    # accordingly. It determines if the item is 'Running', 'Completed', or 'Not processed',
    # and sets the sensitivity of the execute button based on whether the item can be run.
    #
    # @param item The case item to display in the processing view.
    #
    # @note If the item is currently running, its status is set to 'Running'.
    # If the item has already run, it is marked as 'Completed' and can be run again.
    # Otherwise, it is marked as 'Not processed' and can also be run.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate_item(self, item):
        can_run = False

        # set status
        if item in self.__running_items:
            status = 'Running'

        elif item.has_ant('evidence'):
            status = 'Completed'
            can_run = True

        else:
            status = 'Not processed'
            can_run = True

        self.__status_label.set_text(status)

        # populate running messages from item
        # self.__populate_messages()

        # set execution box
        # self.__profile_combobox.set_sensitive(can_run)
        self.__execute_button.set_sensitive(can_run)

        # show widget
        self.__widget.show_content()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate panel with itemlist
    # @param itemlist List of evidence items to display in the processing view.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate_itemlist(self, itemlist):
        completed = 0
        running = 0
        not_processed = 0
        datasource_types = set()

        for item in self.__itemlist:
            profile = item.get_attribute('processing.profile')
            name = item.name
            obj = item

            if not item.has_datasource():
                pass

            elif item in self.__running_items:
                running += 1

            elif item.has_ant('evidence'):
                completed += 1

            else:
                not_processed += 1

            datasource = item.get_datasource()
            if datasource:
                datasource_types.add(datasource.get_type())

        self.__status_label.set_text(f"{running} Running / {completed} Completed / {not_processed} Not processed")
        # self.__populate_messages()

        # set execution box
        can_run = len(datasource_types) == 1
        # self.__profile_combobox.set_sensitive(can_run)
        self.__execute_button.set_sensitive(can_run)

        # show widget
        self.__widget.show_content()

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
            # Show warning message if any item has already been processed
            if any (item.has_ant('evidence') for item in self.__itemlist):
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
