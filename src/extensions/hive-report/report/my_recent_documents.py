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
import pymobius

from common import *
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief recent docs report
# @author Eduardo Aguiar
# @reference Windows Registry Forensics, by Harlan Carvey, p.169
# @reference Forensic Analysis of the Windows Registry, by Lih Wern Wong, p.7
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class MyRecentDocumentsReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'my-recent-docs'
        self.name = 'My Recent Documents'
        self.group = 'user'
        self.description = 'list of the documents that the user has recently accessed'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_report_name('%s of computer <%s>' % (self.name, get_computer_name(registry)))

        # fill data
        for username, key in iter_hkey_users(registry):
            recent_docs_key = key.get_key_by_path('Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RecentDocs')
            if recent_docs_key:

                # get date of last entries
                last_entries = {}

                for ext_key in recent_docs_key.subkeys:
                    mrulistex = get_data_as_buffer(ext_key.get_data_by_name('MRUListEx'))

                    if len(mrulistex) >= 8:
                        last_idx = struct.unpack('<i', mrulistex[:4])[0]
                        doc_name = get_data_as_string(ext_key.get_data_by_name(str(last_idx)))
                        last_entries[doc_name] = ext_key.last_modification_time

                # list documents
                for seq, doc_name in enumerate(iter_mrulistex(recent_docs_key)):
                    if doc_name:
                        timestamp = last_entries.get(doc_name)
                        self.viewer.add_row((username, seq + 1, timestamp, doc_name))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'tableview')

        column = self.viewer.add_column('username')
        column.is_sortable = True

        column = self.viewer.add_column('sequence', column_type='int')
        column.is_sortable = True

        column = self.viewer.add_column('last access date')
        column.is_sortable = True

        column = self.viewer.add_column('document')
        column.is_sortable = True
        column.is_searchable = True

        self.viewer.set_report_id('registry.' + self.id)
        self.viewer.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief recent docs by filetype report
# @author Eduardo Aguiar
# @reference Windows Registry Forensics, by Harlan Carvey, p.169
# @reference Forensic Analysis of the Windows Registry, by Lih Wern Wong, p.7
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class MyRecentDocumentsByFileTypeReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'my-recent-docs-by-filetype'
        self.name = 'My Recent Documents by filetype'
        self.group = 'user'
        self.description = 'list of the documents that the user has recently accessed, by filetype'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_report_name('%s of computer <%s>' % (self.name, get_computer_name(registry)))

        # fill data
        for username, key in iter_hkey_users(registry):

            for subkey in key.get_key_by_mask('Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RecentDocs\\*'):
                filetype = subkey.name
                value_count = len(subkey.values) - 1

                for seq, doc_name in enumerate(iter_mrulistex(subkey)):
                    if seq + 1 == value_count:
                        timestamp = subkey.last_modification_time
                    else:
                        timestamp = None
                    self.viewer.add_row((username, filetype, seq + 1, timestamp, doc_name))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'tableview')

        column = self.viewer.add_column('username')
        column.is_sortable = True

        column = self.viewer.add_column('filetype')
        column.is_sortable = True

        column = self.viewer.add_column('sequence', column_type='int')

        column = self.viewer.add_column('last access date')
        column.is_sortable = True

        column = self.viewer.add_column('document')
        column.is_sortable = True
        column.is_searchable = True

        self.viewer.set_report_id('registry.' + self.id)
        self.viewer.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
