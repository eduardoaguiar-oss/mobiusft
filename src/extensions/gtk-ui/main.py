# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008-2026 Eduardo Aguiar
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

from attributelist import AttributeListWidget
from dnd_toolitem import DNDToolItem
from hex_view import HexViewWidget
from report_dialog import ReportDialog
from tableview import TableViewWidget
from view_selector import ViewSelectorWidget
from widetableview import WideTableViewWidget


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Extension data
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
g_dnd_next_uid = 1
g_dnd_shelf = {}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service <ui.dnd-pop> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_ui_dnd_pop(uid):
    return g_dnd_shelf.pop(uid)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service <ui.dnd-push> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_ui_dnd_push(obj):
    global g_dnd_shelf
    global g_dnd_next_uid

    uid = g_dnd_next_uid

    g_dnd_shelf[uid] = obj
    g_dnd_next_uid += 1

    return uid


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service <ui.new-widget> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_ui_new_widget(classid):
    resource_id = 'ui.widget.' + classid

    if mobius.core.has_resource(resource_id):
        classobj = mobius.core.get_resource_value(resource_id)
        widget = classobj()

    else:
        mobius.core.logf(f'WRN no handle for "{classid}" widget')
        widget = None

    return widget


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service <report.run-dialog> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_report_run_dialog():
    return ReportDialog()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    mobius.core.add_resource('ui.widget.attribute-list', 'UI widget: attribute-list', AttributeListWidget)
    mobius.core.add_resource('ui.widget.dnd-toolitem', 'UI widget: DND ToolItem', DNDToolItem)
    mobius.core.add_resource('ui.widget.hexview', 'UI widget: hexview', HexViewWidget)
    mobius.core.add_resource('ui.widget.tableview', 'UI widget: tableview', TableViewWidget)
    mobius.core.add_resource('ui.widget.view-selector', 'UI widget: view-selector', ViewSelectorWidget)
    mobius.core.add_resource('ui.widget.widetableview', 'UI widget: widetableview', WideTableViewWidget)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('ui.widget.attribute-list')
    mobius.core.remove_resource('ui.widget.dnd-toolitem')
    mobius.core.remove_resource('ui.widget.hexview')
    mobius.core.remove_resource('ui.widget.tableview')
    mobius.core.remove_resource('ui.widget.view-selector')
    mobius.core.remove_resource('ui.widget.widetableview')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief API initialization
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start_api():
    pymobius.mediator.advertise('ui.dnd-pop', svc_ui_dnd_pop)
    pymobius.mediator.advertise('ui.dnd-push', svc_ui_dnd_push)
    pymobius.mediator.advertise('ui.new-widget', svc_ui_new_widget)
    pymobius.mediator.advertise('report.run-dialog', svc_report_run_dialog)
