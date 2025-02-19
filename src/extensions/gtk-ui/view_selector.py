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
from gi.repository import Gtk

from common import get_handler


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Widget: view-selector
#
# This widget is composed of a toolbar and a notebook widgets. Clicking on
# toolitems automatically selects the related notebook's page
#
# Internals:
#
# self.__views is a dictionary with: id -> view
#
# Each view has the following attributes set:
#   id -> view_id
#   toolitem -> corresponding toolbar's toolitem
#   page -> notebook's page
#   updated -> True/False if data shown is up to date
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ViewSelectorWidget(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__views = {}
        self.__toolbar_group = None
        self.__data = None
        self.__updated = False

        # event handlers
        self.__on_view_changed_handler = None

        # main widget
        self.__widget = mobius.ui.box(mobius.ui.box.orientation_vertical)
        self.__widget.set_spacing(3)

        # toolbar
        self.__toolbar = Gtk.Toolbar()
        self.__toolbar.set_style(Gtk.ToolbarStyle.ICONS)
        self.__toolbar.show()
        self.__widget.add_child(self.__toolbar, mobius.ui.box.fill_none)

        # notebook
        self.__notebook = Gtk.Notebook()
        self.__notebook.set_show_tabs(False)
        self.__notebook.show()
        self.__widget.add_child(self.__notebook, mobius.ui.box.fill_with_widget)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Iterate through views
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __iter__(self):
        return iter(self.__views.values())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief show widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def show(self):
        return self.__widget.set_visible(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief hide widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def hide(self):
        return self.__widget.set_visible(False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set widget sensitive status
    # @param sensitive True/False
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_sensitive(self, sensitive):
        return self.__widget.set_sensitive(sensitive)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get toolbar
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_toolbar(self):
        return self.__toolbar

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set control object
    # @param control Control object
    # @param wid widget id to be appended to the handler functions' names
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_control(self, control, wid=None):
        self.__on_view_changed_handler = get_handler('on_view_changed', control, wid)

        # DND button
        on_report_data_handler = get_handler('on_report_data', control, wid)

        if on_report_data_handler:
            self.__dnd_toolitem = pymobius.mediator.call('ui.new-widget', 'dnd-toolitem')
            self.__dnd_toolitem.show()
            self.__dnd_toolitem.set_control(control, wid)
            self.__toolbar.insert(self.__dnd_toolitem.get_ui_widget(), -1)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Check if selector has a view
    # @param view_id View ID
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def has_view(self, view_id):
        return view_id in self.__views

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Add view to widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add(self, view_id, view, pos=-1):

        # render icon
        icon = mobius.ui.new_icon_from_data(view.icon_data, mobius.ui.icon.size_dnd)
        icon.set_visible(True)

        # add page to notebook
        ui_widget = view.get_ui_widget()
        ui_widget.view_id = view_id
        ui_widget.show()
        page_num = self.__notebook.append_page(ui_widget)

        # add icon to toolbar
        toolitem = Gtk.RadioToolButton(group=self.__toolbar_group)
        toolitem.show()
        toolitem.set_tooltip_text(view.name)
        toolitem.connect('toggled', self.__on_view_toggled)
        toolitem.set_icon_widget(icon.get_ui_widget())
        toolitem.view_id = view_id
        self.__toolbar.insert(toolitem, pos)

        # add view
        view.updated = False
        view.id = view_id
        view.toolitem = toolitem
        view.page = ui_widget
        view.parent = self
        self.__views[view_id] = view

        # set toolbar group, if necessary
        if not self.__toolbar_group:
            self.__toolbar_group = toolitem

        # return item
        return toolitem

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Remove view from widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def remove(self, view_id):

        # remove view, if it exists
        view = self.__views.pop(view_id, None)

        if not view:
            return

        # remove related widgets
        is_active = view.toolitem.get_active()
        idx = self.__toolbar.get_item_index(view.toolitem)

        self.__toolbar.remove(view.toolitem)
        self.__notebook.remove(view.page)

        # reset toolbar group if toolitem was the last one
        if len(view.toolitem.get_group()) == 1:
            self.__toolbar_group = None

        # select new toolitem
        if is_active:
            self.__select_new_toolitem(idx)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get current view
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_current_view(self):
        view_id = None
        view = None
        page_num = self.__notebook.get_current_page()

        if page_num != -1:
            page = self.__notebook.get_nth_page(page_num)
            view_id = page.view_id
            view = self.__views.get(view_id)

        return view_id, view

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set current view
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_current_view(self, view_id):
        view = self.__views.get(view_id)

        if view:
            view.toolitem.set_active(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, data, *args):
        self.__data = data
        self.__updated = True
        idx = -1  # inactived toolitem index

        # Show/hide views
        for view in self.__views.values():
            is_active = view.toolitem.get_active()

            if not hasattr(view, 'check_data') or view.check_data(data):
                view.toolitem.show()
                view.page.show()

                if is_active:
                    view.set_data(data)
                    view.updated = True

                else:
                    view.updated = False

            else:
                if is_active:
                    idx = self.__toolbar.get_item_index(view.toolitem)

                view.toolitem.hide()
                view.page.hide()

        # Change current view, if necessary
        if idx != -1:
            self.__select_new_toolitem(idx)

        # set data on current view only
        # view_id, view = self.get_current_view ()
        # if view:
        #  view.set_data (data)
        #  view.updated = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Reload data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def reload(self):
        self.__updated = True

        for view in self.__views.values():
            if view.toolitem.is_visible():
                view.updated = False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Select new toolitem
    # @param idx Old selection
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __select_new_toolitem(self, idx):
        items = self.__toolbar.get_n_items()
        selected_toolitem = None

        # Try to select from idx to last toolitem
        pos = idx

        while not selected_toolitem and pos < items:
            toolitem = self.__toolbar.get_nth_item(pos)

            if toolitem.is_visible() and hasattr(toolitem, 'view_id'):
                selected_toolitem = toolitem

            else:
                pos = pos + 1

        # Try to select from idx to first toolitem
        pos = idx - 1

        while not selected_toolitem and pos >= 0:
            toolitem = self.__toolbar.get_nth_item(pos)

            if toolitem.is_visible() and hasattr(toolitem, 'view_id'):
                selected_toolitem = toolitem

            else:
                pos = pos - 1

        # If there is a toolitem to be selected, select it
        if selected_toolitem:
            selected_toolitem.set_active(True)

        if self.__on_view_changed_handler:
            self.__on_view_changed_handler(None)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle view toggle
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_view_toggled(self, toolitem, *args):

        # Check if toolitem is active
        if not toolitem.get_active():
            return

        # Get view related to toolitem
        view = self.__views.get(toolitem.view_id)
        if not view:
            return

        # Change notebook page
        page_num = self.__notebook.page_num(view.page)
        self.__notebook.set_current_page(page_num)

        # Update data, if necessary
        if self.__updated and not view.updated:
            view.set_data(self.__data)
            view.updated = True

        # Emit view_changed event, if necessary
        if self.__on_view_changed_handler:
            self.__on_view_changed_handler(view)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_drag_data_get
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_drag_data_get(self, widget, context, selection, info, utime, *args):
        report = self.__on_report_data_handler(self)

        # push object for DND
        uid = self.__mediator.call('ui.dnd-push', report)
        selection.set(selection.get_target(), 8, b'%d' % uid)
