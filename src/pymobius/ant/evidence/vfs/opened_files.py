# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import traceback

import mobius
import pymobius
import pymobius.app.chromium
import pymobius.app.gecko
import pymobius.app.internet_explorer
import pymobius.operating_system

ANT_ID = 'opened-files'
ANT_NAME = 'Opened files'
ANT_VERSION = '1.1'
EVIDENCE_TYPE = 'opened-file'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: opened-files
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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

        # check if datasource is available
        datasource = self.__item.get_datasource()

        if not datasource:
            return

        if not datasource.is_available():
            raise Exception('Datasource is not available')

        # retrieve data
        self.__entries = []

        try:
            self.__retrieve_chromium()
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

        try:
            self.__retrieve_gecko()
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

        try:
            self.__retrieve_windows_recent()
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

        self.__save_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Chromium based browsers
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_chromium(self):
        model = pymobius.app.chromium.model(self.__item)

        for profile in model.get_profiles():
            for h in profile.get_history():
                url = mobius.core.io.uri(h.url)

                if url.get_scheme() == 'file':
                    entry = pymobius.Data()

                    entry.path = format_path(url.get_path())
                    entry.timestamp = h.timestamp
                    entry.username = h.username
                    entry.app_id = profile.app_id
                    entry.app_name = profile.app_name
                    entry.item = self.__item.name

                    entry.metadata = mobius.core.pod.map()
                    entry.metadata.set('title', h.title)
                    entry.metadata.set('url', h.url)
                    entry.metadata.set('profile', h.profile_name)
                    entry.metadata.set('profile-path', h.profile_path)

                    self.__entries.append(entry)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Gecko based browsers
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_gecko(self):
        model = pymobius.app.gecko.model(self.__item)

        for profile in model.get_profiles():
            for h in profile.get_history():
                url = mobius.core.io.uri(h.url)

                if url.get_scheme() == 'file':
                    entry = pymobius.Data()

                    entry.path = format_path(url.get_path())
                    entry.timestamp = h.timestamp
                    entry.username = h.username
                    entry.app_id = profile.app_id
                    entry.app_name = profile.app_name

                    entry.metadata = mobius.core.pod.map()
                    entry.metadata.set('typed', h.typed)
                    entry.metadata.set('url', h.url)
                    entry.metadata.set('profile', h.profile_name)
                    entry.metadata.set('profile-path', h.profile_path)

                    self.__entries.append(entry)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve opened files from Windows/Recent folders
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_windows_recent(self):
        for os in pymobius.operating_system.scan(self.__item):
            for profile in os.get_profiles():
                folder = profile.get_appdata_entry('Microsoft/Windows/Recent')
                if folder and folder.is_folder():
                    self.__retrieve_windows_recent_folder(folder, profile)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve opened files from Windows/Recent folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_windows_recent_folder(self, folder, profile):
        for entry in folder.get_children():
            if not entry.is_deleted() and entry.is_file() and entry.name.lower().endswith('.lnk'):
                self.__retrieve_windows_recent_file(entry, profile)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve opened files from Windows/Recent file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_windows_recent_file(self, f, profile):
        try:
            lnkfile = mobius.core.decoder.lnk(f.new_reader())

            if lnkfile.local_base_path:

                # If file's creation time is different, we have two separated events
                timestamps = {f.creation_time, f.modification_time}

                for timestamp in timestamps:
                    entry = pymobius.Data()
                    entry.path = lnkfile.local_base_path

                    if lnkfile.common_path_suffix:
                        if not entry.path.endswith('\\'):
                            entry.path += '\\'
                        entry.path += lnkfile.common_path_suffix

                    entry.timestamp = timestamp
                    entry.username = profile.username
                    entry.app_id = 'win'
                    entry.app_name = 'Windows'

                    entry.metadata = mobius.core.pod.map()
                    entry.metadata.set('profile-path', profile.path)
                    entry.metadata.set('lnk-path', mobius.core.io.to_win_path(f.path))
                    entry.metadata.set('target-creation-time', lnkfile.creation_time)
                    entry.metadata.set('target-access-time', lnkfile.access_time)
                    entry.metadata.set('target-write-time', lnkfile.write_time)
                    entry.metadata.set('target-size', lnkfile.file_size)
                    entry.metadata.set('netbios-name', lnkfile.netbios_name)
                    entry.metadata.set('drive-serial-number', f'0x{lnkfile.drive_serial_number:08x}')
                    entry.metadata.set('relative-path', lnkfile.relative_path)

                    self.__entries.append(entry)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)} {traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        transaction = self.__item.new_transaction()

        # save entries
        for e in self.__entries:
            entry = self.__item.new_evidence(EVIDENCE_TYPE)
            entry.timestamp = e.timestamp
            entry.path = e.path
            entry.username = e.username
            entry.app_id = e.app_id
            entry.app_name = e.app_name
            entry.metadata = e.metadata

        # commit data
        transaction.commit()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Format path
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def format_path(path):
    if path[:1] == '/' and path[2:3] == ':':
        path = path[1:]

    return path
