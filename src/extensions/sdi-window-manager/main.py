# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023 Eduardo Aguiar
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

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Extension metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
EXTENSION_ID = 'sdi-window-manager'
EXTENSION_NAME = 'Single Document Interface'
EXTENSION_AUTHOR = 'Eduardo Aguiar'
EXTENSION_VERSION = '0.1.8'
EXTENSION_DESCRIPTION = 'Single document interface window manager'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Working Area
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class WorkingArea(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief create working area
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.mediator = pymobius.mediator.copy()
        self.uid = None
        self.id = None

        self.window = mobius.ui.window()
        self.window.set_callback('closing', self.on_window_closing)
        self.widget = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set default size
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_default_size(self, width, height):
        self.window.set_size(width, height)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set title of working area
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_title(self, title=''):
        app = mobius.core.application()

        w_title = app.title
        if title:
            w_title += ' - ' + title

        self.window.set_title(w_title)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set icon
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_icon(self, path):
        icon = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_toolbar)
        self.window.set_icon(icon)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_widget(self, widget):
        self.widget = widget
        self.window.set_content(widget)
        widget.working_area = self

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_widget(self):
        return self.widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Show working area
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def show(self):

        if mobius.framework.has_config(f'{self.id}.window.x'):
            x = mobius.framework.get_config(f'{self.id}.window.x')
            y = mobius.framework.get_config(f'{self.id}.window.y')
            width = mobius.framework.get_config(f'{self.id}.window.width')
            height = mobius.framework.get_config(f'{self.id}.window.height')

            self.window.set_position(x, y)
            self.window.set_size(width, height)

        self.window.show_all()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event: on_window_closing
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_window_closing(self):

        # check if widget can be destroyed
        can_destroy = True

        if self.widget:
            on_widget_stopped = getattr(self.widget, 'on_widget_stopped', None)
            if on_widget_stopped:
                can_destroy = not on_widget_stopped()

        # destroy it if necessary
        if can_destroy:
            destroy_working_area(self.uid)

        else:
            return True


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Extension data
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
g_next_working_area_uid = 1
g_working_area = {}
g_working_area_by_id = {}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Destroy working area
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def destroy_working_area(uid):
    working_area = g_working_area.pop(uid)

    # save window position
    x, y = working_area.window.get_position()
    width, height = working_area.window.get_size()

    transaction = mobius.framework.new_config_transaction()
    mobius.framework.set_config(f'{working_area.id}.window.x', x)
    mobius.framework.set_config(f'{working_area.id}.window.y', y)
    mobius.framework.set_config(f'{working_area.id}.window.width', width)
    mobius.framework.set_config(f'{working_area.id}.window.height', height)
    transaction.commit()

    # destroy working area
    working_area.window.destroy()

    uids = g_working_area_by_id.get(working_area.id)
    uids.remove(uid)

    if uids:
        g_working_area_by_id[id] = uids
    else:
        g_working_area_by_id.pop(working_area.id)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# service <ui.working-area.close> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_ui_working_area_close(working_area_id):
    uids = g_working_area_by_id.get(working_area_id, [])

    for uid in uids:
        working_area = g_working_area.get(uid)
        working_area.on_window_closing()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# service <ui.working-area.del> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_ui_working_area_del(working_area_id):
    uids = g_working_area_by_id.get(working_area_id, [])
    for uid in uids:
        destroy_working_area(uid)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# service <ui.working-area.get> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_ui_working_area_get(working_area_id):
    uids = g_working_area_by_id.get(working_area_id)

    if uids:
        return g_working_area.get(uids[0])

    return None


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# service <ui.working-area.new> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_ui_working_area_new(working_area_id):
    global g_next_working_area_uid

    working_area = WorkingArea()
    working_area.uid = g_next_working_area_uid
    working_area.id = working_area_id

    g_working_area[working_area.uid] = working_area
    g_working_area_by_id.setdefault(working_area_id, []).append(working_area.uid)
    g_next_working_area_uid += 1

    return working_area


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# API initialization
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start_api():
    pymobius.mediator.advertise('ui.working-area.close', svc_ui_working_area_close)
    pymobius.mediator.advertise('ui.working-area.del', svc_ui_working_area_del)
    pymobius.mediator.advertise('ui.working-area.get', svc_ui_working_area_get)
    pymobius.mediator.advertise('ui.working-area.new', svc_ui_working_area_new)
