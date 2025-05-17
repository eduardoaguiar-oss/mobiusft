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
import traceback

import mobius
import mobius.framework
import pymobius
import pymobius.item_browser
from gi.repository import Gtk
from add_item_dialog import AddItemDialog
from case_treeview import CaseTreeView

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
(ITEM_ICON, ITEM_NAME, ITEM_OBJ) = range(3)

# DND mimetypes
REPORT_DATA_MIMETYPE = 'application/x-mobius-report-data'
URI_LIST_MIMETYPE = 'text/uri-list'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Case view widget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CaseView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__case = None
        self.__groups = []
        self.__itemlist = []

        # hpaned
        self.__hpaned = Gtk.HPaned()
        self.__widget = self.__hpaned

        position = mobius.framework.get_config('ice.hpaned-position')
        if position:
            self.__hpaned.set_position(position)

        # treeview
        self.__treeview = CaseTreeView()
        self.__treeview.set_multiple_selection(True)
        self.__treeview.add_dnd_dest_mimetype(URI_LIST_MIMETYPE)
        self.__treeview.add_dnd_dest_mimetype(REPORT_DATA_MIMETYPE)
        self.__treeview.set_control(self, 'treeview')
        self.__treeview.show()
        self.__hpaned.pack1(self.__treeview.get_ui_widget(), True, True)

        # details
        frame = Gtk.Frame()
        frame.show()
        self.__hpaned.pack2(frame, False, True)

        self.__view_selector = self.__mediator.call('ui.new-widget', 'view-selector')
        self.__view_selector.show()
        frame.add(self.__view_selector.get_ui_widget())

        # toolitem: add item
        toolbar = self.__view_selector.get_toolbar()

        self.add_toolitem = Gtk.ToolButton.new()
        self.add_toolitem.set_icon_name('list-add')
        self.add_toolitem.set_sensitive(False)
        self.add_toolitem.connect("clicked", self.__on_add_item)
        self.add_toolitem.show()
        self.add_toolitem.set_tooltip_text("Add items to case")
        toolbar.insert(self.add_toolitem, -1)

        # toolitem: remove item
        self.remove_toolitem = Gtk.ToolButton.new()
        self.remove_toolitem.set_icon_name('list-remove')
        self.remove_toolitem.set_sensitive(False)
        self.remove_toolitem.connect("clicked", self.__on_remove_item)
        self.remove_toolitem.show()
        self.remove_toolitem.set_tooltip_text("Remove items from case")
        toolbar.insert(self.remove_toolitem, -1)

        # toolitem: run report
        self.report_toolitem = Gtk.ToolButton.new()

        image = mobius.ui.new_icon_by_name('report-run', mobius.ui.icon.size_toolbar)
        image.show()
        self.report_toolitem.set_icon_widget(image.get_ui_widget())

        self.report_toolitem.set_sensitive(False)
        self.report_toolitem.connect("clicked", self.__on_report_item)
        self.report_toolitem.show()
        self.report_toolitem.set_tooltip_text("Run report on selected item")
        toolbar.insert(self.report_toolitem, -1)

        # add view groups
        for resource in mobius.core.get_resources('group-view'):

            # add toolbar separator
            toolitem = Gtk.SeparatorToolItem()
            toolitem.show()
            toolbar.insert(toolitem, -1)

            # add group
            group = resource.value(self.__view_selector)
            self.__groups.append(group)

        # separator
        toolitem = Gtk.SeparatorToolItem()
        toolitem.show()
        toolbar.insert(toolitem, -1)

        # add other views
        for resource in mobius.core.get_resources('view'):
            self.__view_selector.add(resource.id, resource.value())

        # subscribe to events
        self.__event_uid_1 = mobius.core.subscribe('attribute-modified', self.__on_attribute_modified)
        self.__event_uid_2 = mobius.core.subscribe('attribute-removed', self.__on_attribute_removed)
        self.__event_uid_3 = mobius.core.subscribe('resource-added', self.__on_resource_added)
        self.__event_uid_4 = mobius.core.subscribe('resource-removed', self.__on_resource_removed)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Show widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def show(self):
        self.__widget.show()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Hide widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def hide(self):
        self.__widget.hide()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get low level widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set case object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_case(self, case):
        self.__case = case

        # populate treeview
        if self.__case:
            item = self.__case.root_item
            self.__treeview.set_root_items([item])

        else:
            self.__treeview.clear()

        # set case for all views
        for view in self.__view_selector:
            if hasattr(view, 'set_case'):
                view.set_case(self.__case)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_destroy event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        mobius.framework.set_config('ice.hpaned-position', self.__hpaned.get_position())

        for view in self.__view_selector:
            if hasattr(view, 'on_destroy'):
                try:
                    view.on_destroy()
                except Exception as e:
                    mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

        mobius.core.unsubscribe(self.__event_uid_1)
        mobius.core.unsubscribe(self.__event_uid_2)
        mobius.core.unsubscribe(self.__event_uid_3)
        mobius.core.unsubscribe(self.__event_uid_4)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief call report item dialog
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_report_item(self, widget, *args):
        selected_items = self.__treeview.get_selected_items()

        if len(selected_items) == 1:
            item = selected_items[0]
            item_proxy = pymobius.item_browser.ItemBrowser(item)

            dialog = self.__mediator.call('report.run-dialog')
            dialog.run(item_proxy)
            dialog.destroy()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle treeview->retrieve-icon
    # @param item treeitem
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def treeview_on_retrieve_icon(self, item):
        category = mobius.framework.get_category(item.category)
        return category.icon_data

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle treeview->selection-changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def treeview_on_selection_changed(self, itemlist):
        self.__itemlist = itemlist

        # can add item
        add_item_enabled = len(itemlist) == 1

        # can remove item
        has_case_item = any(item for item in itemlist if item.category == 'case')
        remove_item_enabled = len(itemlist) > 0 and not has_case_item

        # can run report
        report_run_enabled = len(itemlist) == 1

        # enable/disable widgets
        self.add_toolitem.set_sensitive(add_item_enabled)
        self.remove_toolitem.set_sensitive(remove_item_enabled)
        self.report_toolitem.set_sensitive(report_run_enabled)

        # update view selector
        self.__view_selector.set_data(itemlist)

        # emit 'case.selection-changed'
        self.__mediator.emit('case.selection-changed', self.__case, itemlist)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle treeview->on_reordering_item
    # @param item item being reordered
    # @param old_parent old parent item
    # @param new_parent new parent item
    # @param pos item order into new parent's children
    # @return True if can/False if cannot
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def treeview_on_reordering_item(self, item, old_parent, new_parent, pos):

        # check if case item is being moved
        if item.category == 'case':
            return False

        # check if destination is valid
        if not new_parent:
            return False

        # move item
        transaction = self.__case.new_transaction()
        item.move(pos + 1, new_parent)
        transaction.commit()

        return True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle treeview->on_file_dropped
    # @param parent parent item
    # @param pos child position
    # @param mimetype data mimetype
    # @param data data
    # @return True if accepted/False otherwise
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def treeview_on_file_dropped(self, parent, pos, mimetype, data):
        rc = False

        if parent:

            # REPORT DATA DND
            if mimetype == REPORT_DATA_MIMETYPE:
                rc = self.__on_dnd_report_data(parent, pos, data)

            # FILE DND
            elif mimetype == URI_LIST_MIMETYPE:
                rc = self.__on_dnd_uri_list(parent, pos, data)

        return rc

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle treeview->on_remove_item
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def treeview_on_remove_item(self, item):
        transaction = self.__case.new_transaction()
        item.remove()
        transaction.commit()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle REPORT_DATA_MIMETYPE DND
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_dnd_report_data(self, parent, pos, data):
        uid = int(data)
        report = self.__mediator.call('ui.dnd-pop', uid)
        transaction = self.__case.new_transaction()

        child = parent.new_child('report-data', pos + 1)
        child.rid = report.rid
        child.name = report.name
        child.application = report.app
        child.widget = report.widget
        child.data = report.data

        transaction.commit()

        return True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle URI_LIST_MIMETYPE DND
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_dnd_uri_list(self, parent, pos, data):

        # get DND handlers
        handlers = []
        fallback = None

        for resource in mobius.core.get_resources('ui.dnd-file'):

            if resource.id == 'fallback':
                fallback = resource.value

            else:
                handlers.append(resource.value)

        if fallback:
            handlers.append(fallback)

        # process files
        uri_list = data.decode('utf-8').rstrip().split('\r\n')
        item_count = 0
        transaction = self.__case.new_transaction()

        for uri in uri_list:

            # run through ui.dnd-file handlers until one handles this file
            for handler in handlers:
                count = handler(parent, pos, uri)

                if count > 0:
                    pos += count
                    item_count += count
                    break

        transaction.commit()

        return item_count > 0

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle add item
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_add_item(self, widget, *args):

        # get selected item
        if self.__treeview.count_selected_items() != 1:
            return

        parent = self.__treeview.get_selected_items()[0]

        # call dialog
        dialog = AddItemDialog()

        last_category_added = mobius.framework.get_config('ice.last-category-added')
        if last_category_added:
            dialog.set_category(last_category_added)

        rc = dialog.run()

        # read data
        amount = dialog.get_amount()
        category = dialog.get_category()
        attribute_values = dialog.get_attribute_values()
        dialog.destroy()

        if rc != Gtk.ResponseType.OK or not category:
            return

        # create items
        transaction = self.__case.new_transaction()

        for i in range(amount):
            item = parent.new_child(category.id)

            # set item attributes
            for attr_id, attr_value in attribute_values:
                item.set_attribute(attr_id, attr_value)

            item.expand_masks()

        transaction.commit()

        # reload item
        self.__treeview.reload_selected_items(True)
        self.__treeview.expand_selected_items()

        # save configuration
        transaction = mobius.framework.new_config_transaction()
        mobius.framework.set_config('ice.last-category-added', category.id)
        transaction.commit()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle remove item
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_remove_item(self, widget, *args):
        count = self.__treeview.count_selected_items()

        # show dialog
        dialog = mobius.ui.message_dialog(mobius.ui.message_dialog.type_question)
        dialog.text = 'You are about to remove %s. Are you sure?' % ('an item' if count == 1 else f'{count:d} items')
        dialog.add_button(mobius.ui.message_dialog.button_yes)
        dialog.add_button(mobius.ui.message_dialog.button_no)
        dialog.set_default_response(mobius.ui.message_dialog.button_no)
        rc = dialog.run()

        if rc != mobius.ui.message_dialog.button_yes:
            return

        # remove items
        self.__treeview.remove_selected_items()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle attribute-modified event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_attribute_modified(self, obj, attr_id, old_value, new_value):
        self.__on_attribute_changed(obj, attr_id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle attribute-removed event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_attribute_removed(self, obj, attr_id, old_value):
        self.__on_attribute_changed(obj, attr_id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle attribute changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_attribute_changed(self, obj, attr_id):

        if self.__case and obj.case and obj.case.uid == self.__case.uid:

            if attr_id == 'name':
                self.__treeview.reload_selected_items(False)

            elif attr_id == 'children':
                self.__treeview.reload_selected_items(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle resource-added event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_resource_added(self, report_id, resource):
        if report_id.startswith('view.'):
            item_id = report_id.rsplit('.', 1)[1]
            view = resource.value()
            toolitem = self.__view_selector.add(item_id, view)
            toolitem.set_active(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle resource-removed event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_resource_removed(self, report_id):
        if report_id.startswith('view.'):
            item_id = report_id.rsplit('.', 1)[1]
            self.__view_selector.remove(item_id)
