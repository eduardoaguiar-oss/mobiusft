# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import traceback

import mobius
import pymobius
import pymobius.app.itubego

ANT_ID = 'received-files'
ANT_NAME = 'Received files'
ANT_VERSION = '1.2'
EVIDENCE_TYPE = 'received-file'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: received-files
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.id = ANT_ID
        self.name = ANT_NAME
        self.version = ANT_VERSION
        self.__item = item
        self.__entries = []

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__entries = []

        # check if datasource is available
        datasource = self.__item.get_datasource()

        if not datasource:
            return

        if not datasource.is_available():
            raise Exception('Datasource is not available')

        # retrieve data
        self.__retrieve_itubego()

        # save data
        self.__save_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from iTubeGo app
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_itubego(self):
        try:
            model = pymobius.app.itubego.model(self.__item)

            for profile in model.get_profiles():
                self.__retrieve_itubego_profile(profile)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from iTubeGo profile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_itubego_profile(self, profile):

        try:
            for d in profile.get_downloaded_files():
                uri = mobius.core.io.uri(d.url)
                entry = pymobius.Data()

                entry.username = profile.username
                entry.timestamp = None
                entry.filename = uri.get_filename()
                entry.path = d.file_path
                entry.app_id = profile.app_id
                entry.app_name = profile.app_name

                entry.metadata = mobius.core.pod.map()
                entry.metadata.set('url', d.url)
                entry.metadata.set('title', d.title)
                entry.metadata.set('size', d.file_size)
                entry.metadata.set('destination-folder', d.dst_folder)
                entry.metadata.set('file-type', d.file_type)
                entry.metadata.set('temp-file-path', d.temp_file_path)
                entry.metadata.set('thumbnail-url', d.thumbnail_url)
                entry.metadata.set('thumbnail-path', d.thumbnail_path)
                entry.metadata.set('bytes-downloaded', d.bytes_downloaded)
                entry.metadata.set('download-state', d.download_state)
                entry.metadata.update(d.metadata)
                entry.metadata.set('profile-id', profile.name)
                entry.metadata.set('profile-path', profile.path)

                self.__entries.append(entry)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        transaction = self.__item.new_transaction()

        # save evidences
        for e in self.__entries:
            entry = self.__item.new_evidence(EVIDENCE_TYPE)

            entry.username = e.username
            entry.timestamp = e.timestamp
            entry.filename = e.filename
            entry.path = e.path
            entry.app_id = e.app_id
            entry.app_name = e.app_name
            entry.metadata = e.metadata

        # commit data
        transaction.commit()
