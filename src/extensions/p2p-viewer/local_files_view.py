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
from gi.repository import Gtk

from common import get_hash_string
from common import set_alert_flags
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get string text for a state value
# @param state state value
# @return string
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_state_string(state):
    return {0: 'no', 1: 'yes', 2: 'always'}.get(state, '')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get string text for a state value
# @param state state value
# @return string
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_state_string_full(state):
    return {0: 'no', 1: 'yes', 2: 'always'}.get(state, 'unknown')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief view: local files
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class LocalFilesView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()

        self.name = 'Local Files'
        icon_path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'local-files.png')
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.ui.container()
        self.__widget.show()

        # vpaned
        self.__vpaned = Gtk.VPaned()
        self.__vpaned.show()
        self.__widget.set_content(self.__vpaned)

        pos = mobius.framework.get_config('p2p-viewer.local-files-vpaned-pos')
        if pos:
            self.__vpaned.set_position(pos)

        # Files tableview
        self.__tableview = self.__mediator.call('ui.new-widget', 'tableview')
        self.__tableview.set_report_id('p2p.local-files')
        self.__tableview.set_report_name('P2P: Local Files')
        self.__tableview.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
        self.__tableview.set_control(self, 'main_tableview')

        # KFF alerts
        self.__hashsets = []
        kff = mobius.kff.kff()

        for h_id, h in kff.get_hashsets():
            if h.is_alert():
                column = self.__tableview.add_column('alert_%s' % h_id, 'Hash Alert:\n%s' % h_id)
                column.is_sortable = True
                self.__hashsets.append((h_id, h))

        # Other columns
        column = self.__tableview.add_column('shared', 'Shared')
        column.is_sortable = True

        column = self.__tableview.add_column('downloaded', 'Downloaded')
        column.is_sortable = True

        column = self.__tableview.add_column('uploaded', 'Uploaded')
        column.is_sortable = True

        column = self.__tableview.add_column('complete', 'Complete')
        column.is_sortable = True

        column = self.__tableview.add_column('size', 'Size', column_type='int')
        column.is_sortable = True

        column = self.__tableview.add_column('username', 'User Name')
        column.is_sortable = True

        column = self.__tableview.add_column('name', 'Name')
        column.is_sortable = True

        column = self.__tableview.add_column('Obj', column_type='object')
        column.is_visible = False

        self.__tableview.show()
        self.__vpaned.pack1(self.__tableview.get_ui_widget(), False, True)

        # metadata view
        self.__metadata_view = self.__mediator.call('ui.new-widget', 'tableview')
        self.__metadata_view.set_report_id('p2p.local-file')
        self.__metadata_view.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
        self.__metadata_view.show()

        column = self.__metadata_view.add_column('name', 'Attribute')
        column.is_sortable = True

        column = self.__metadata_view.add_column('value', 'Value')
        column.is_sortable = True

        self.__vpaned.pack2(self.__metadata_view.get_ui_widget(), True, True)

        # set initial message
        self.__widget.set_message('Select item(s) to view local files records')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, data):
        self.__widget.show_content()
        self.__tableview.clear()
        hashset_ids = [h_id for (h_id, h) in self.__hashsets]

        for f in data.local_files:
            flag_shared = get_state_string(f.flag_shared)
            flag_downloaded = get_state_string(f.flag_downloaded)
            flag_uploaded = get_state_string(f.flag_uploaded)
            flag_completed = get_state_string(f.flag_completed)
            set_alert_flags(f, hashset_ids)
            alert_flags = ['✓' if flag else '' for flag in f.alert_flags]

            self.__tableview.add_row((*alert_flags, flag_shared, flag_downloaded, flag_uploaded, flag_completed, f.size,
                                      f.username, f.name, f))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save current state
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        mobius.framework.set_config('p2p-viewer.local-files-vpaned-pos', self.__vpaned.get_position())
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle main tableview selection-changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def main_tableview_on_selection_changed(self, selection_list):
        self.__metadata_view.clear()

        if selection_list:
            row_id, row_data = selection_list[0]
            f = row_data[-1]

            # attributes
            self.__metadata_view.add_row(('File name', f.name))

            if f.size != -1:
                self.__metadata_view.add_row(('Size', '%d bytes' % f.size))

            self.__metadata_view.add_row(('Path', f.path))
            self.__metadata_view.add_row(('User name', f.username))
            self.__metadata_view.add_row(('Application', f.app))
            self.__metadata_view.add_row(('Shared', get_state_string_full(f.flag_shared)))
            self.__metadata_view.add_row(('Downloaded', get_state_string_full(f.flag_downloaded)))
            self.__metadata_view.add_row(('Uploaded', get_state_string_full(f.flag_uploaded)))
            self.__metadata_view.add_row(('Corrupted', get_state_string_full(f.flag_corrupted)))
            self.__metadata_view.add_row(('Completed', get_state_string_full(f.flag_completed)))

            # hashes
            hashes = []
            for htype, hvalue in f.get_hashes():
                hashes.append((get_hash_string(htype), hvalue))

            for htype, hvalue in sorted(hashes):
                self.__metadata_view.add_row(('Hash %s' % htype, hvalue.upper()))

            # metadata
            for name, value in f.get_metadata():
                self.__metadata_view.add_row((name, value))

            self.__metadata_view.set_report_name('P2P: Local File %s' % f.name)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle main tableview DND get data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def main_tableview_on_dnd_get_data(self, tableview):
        data = pymobius.Data()
        data.hashsets = [(h_id, h.description) for (h_id, h) in self.__hashsets]
        data.files = []

        for row in tableview:
            lf = row[-1]
            f = pymobius.Data()
            f.name = lf.name
            f.app = lf.app
            f.size = lf.size
            f.path = lf.path
            f.username = lf.username
            f.metadata = [(k, str(v) if v is not None else '') for (k, v) in lf.get_metadata()]
            f.hashes = lf.get_hashes()
            f.flag_shared = lf.flag_shared
            f.flag_uploaded = lf.flag_uploaded
            f.flag_downloaded = lf.flag_downloaded
            f.flag_completed = lf.flag_completed
            f.flag_alerts = lf.alert_flags
            data.files.append(f)

        return data
