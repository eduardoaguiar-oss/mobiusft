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
import mobius.core.datasource
import mobius.core.vfs
import pymobius
from gi.repository import Gtk

from blocktable_view import BlockTableView
from disktable_view import DiskTableView
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: VFS
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class VFSView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__control = control
        self.__mediator = pymobius.mediator.copy()
        self.name = 'Virtual File System (VFS): imagefiles and physical devices'
        self.typename = 'vfs'

        icon_path = self.__mediator.call('extension.get-icon-path', EXTENSION_ID)
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.core.ui.container()
        self.__widget.set_visible(True)

        # vbox
        vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbox.set_spacing(5)
        vbox.set_visible(True)
        self.__widget.set_content(vbox)

        # view selector
        self.__view_selector = self.__mediator.call('ui.new-widget', 'view-selector')
        # self.__view_selector.set_control (self)
        self.__view_selector.show()

        # views
        self.__disks_view = DiskTableView(self)
        self.__view_selector.add('disks', self.__disks_view)

        self.__blocks_view = BlockTableView(self)
        self.__view_selector.add('blocks', self.__blocks_view)

        vbox.add_child(self.__view_selector.get_ui_widget(), mobius.core.ui.box.fill_with_widget)

        # set container
        self.__widget.set_message('Select an item to view VFS')

        # view data
        self.__item = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_destroy event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        for view in self.__view_selector:
            view.on_destroy()

        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, datasource):

        if datasource:
            self.__vfs = datasource.get_vfs()

        else:
            self.__vfs = mobius.core.vfs.vfs()
            self.on_vfs_modified()

        self.__view_selector.set_data(self.__vfs)
        self.__widget.show_content()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_vfs_modified event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_vfs_modified(self):
        datasource = mobius.core.datasource.new_datasource_from_vfs(self.__vfs)
        self.__control.on_datasource_modified(datasource)
        self.__view_selector.reload()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_report_data event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_report_data(self, report):
        report.app = f'{EXTENSION_NAME} v{EXTENSION_VERSION}'
        report.data = pymobius.Data()
        report.data.datasource_type = 'vfs'

        # disks
        report.data.disks = []

        for disk in self.__vfs.get_disks():
            d = pymobius.Data()
            d.type = disk.type
            d.size = disk.size
            d.name = disk.name
            d.attributes = disk.get_attributes()
            report.data.disks.append(d)

        # blocks
        report.data.blocks = []

        for block in self.__vfs.get_blocks():
            b = pymobius.Data()
            b.uid = block.uid
            b.type = block.type
            b.size = block.size
            b.attributes = block.get_attributes()
            b.parents = [p.uid for p in block.get_parents()]
            b.children = [c.uid for c in block.get_children()]
            report.data.blocks.append(b)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    mobius.core.add_resource('widget.datasource.vfs', 'VFS datasource view', VFSView)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('widget.datasource.vfs')
