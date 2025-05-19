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
import pymobius.app.itubego
import pymobius.app.skype
import pymobius.app.utorrent

ANT_ID = 'received-files'
ANT_NAME = 'Received files'
ANT_VERSION = '1.0'
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
        self.__retrieve_chromium()
        self.__retrieve_gecko()
        self.__retrieve_itubego()
        self.__retrieve_skype()
        self.__retrieve_utorrent()

        # save data
        self.__save_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Chromium based browsers
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_chromium(self):
        try:
            model = pymobius.app.chromium.model(self.__item)

            for profile in model.get_profiles():
                self.__retrieve_chromium_profile(profile)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Chromium profile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_chromium_profile(self, profile):
        DOWNLOAD_STATE = {0: 'In progress', 1: 'Complete', 2: 'Cancelled', 3: 'Interrupted'}

        try:
            for d in profile.get_downloads():
                entry = pymobius.Data()

                entry.username = profile.username
                entry.timestamp = d.start_time
                entry.filename = d.name
                entry.path = d.target
                entry.app_id = profile.app_id
                entry.app_name = profile.app_name

                entry.metadata = mobius.core.pod.map()
                entry.metadata.set('url', d.source)
                entry.metadata.set('size', d.size)
                entry.metadata.set('start-time', d.start_time)
                entry.metadata.set('end-time', d.end_time)
                entry.metadata.set('bytes-downloaded', d.bytes_downloaded)
                entry.metadata.set('download-state', DOWNLOAD_STATE.get(d.state, 'Unknown (%d)' % d.state))
                entry.metadata.set('page-referrer', d.referrer)
                entry.metadata.set('profile-id', profile.name)
                entry.metadata.set('profile-path', profile.path)

                self.__entries.append(entry)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Gecko based browsers
    # @see http://doxygen.db48x.net/mozilla/html/interfacensIDownloadManager.html
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_gecko(self):
        try:
            model = pymobius.app.gecko.model(self.__item)

            for profile in model.get_profiles():
                self.__retrieve_gecko_profile(profile)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Gecko profile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_gecko_profile(self, profile):
        DOWNLOAD_STATE = {0: '', 1: 'Finished', 2: 'Failed', 3: 'Cancelled', 4: 'Paused', 5: 'Queued',
                          6: 'Blocked Parental', 7: 'Scanning', 8: 'Virus Detected', 9: 'Blocked Policy'}

        try:
            for d in profile.get_downloads():
                entry = pymobius.Data()

                entry.username = profile.username
                entry.timestamp = d.start_time
                entry.filename = d.name
                entry.path = d.target
                entry.app_id = profile.app_id
                entry.app_name = profile.app_name

                entry.metadata = mobius.core.pod.map()
                entry.metadata.set('url', d.source)
                entry.metadata.set('size', d.size)
                entry.metadata.set('start-time', d.start_time)
                entry.metadata.set('end-time', d.end_time)
                entry.metadata.set('bytes-downloaded', d.bytes_downloaded)
                entry.metadata.set('download-state', DOWNLOAD_STATE.get(d.state, 'Unknown (%d)' % d.state))
                entry.metadata.set('page-referrer', d.referrer)
                entry.metadata.set('profile-id', profile.name)
                entry.metadata.set('profile-path', profile.path)

                self.__entries.append(entry)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

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
    # @brief Retrieve data from Skype
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_skype(self):
        try:
            model = pymobius.app.skype.model(self.__item)

            for profile in model.get_profiles():
                self.__retrieve_skype_profile(profile)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Skype profile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_skype_profile(self, profile):
        DOWNLOAD_STATE = {0: 'Not initiated', 7: 'Cancelled', 8: 'Completed', 9: 'Error'}

        try:
            for ft in profile.get_file_transfers():
                if ft.type == 1:
                    entry = pymobius.Data()

                    entry.username = profile.username
                    entry.timestamp = ft.start_time
                    entry.filename = ft.filename
                    entry.path = ft.path
                    entry.app_id = 'skype'
                    entry.app_name = 'Skype'

                    entry.metadata = mobius.core.pod.map()
                    entry.metadata.set('size', ft.size)
                    entry.metadata.set('start-time', ft.start_time)
                    entry.metadata.set('end-time', ft.finish_time)
                    entry.metadata.set('bytes-received', ft.bytes_transferred)
                    entry.metadata.set('transfer-state', DOWNLOAD_STATE.get(ft.status, 'Unknown (%d)' % ft.status))

                    sender = ft.from_skype_account
                    if ft.from_skype_name:
                        sender += f' ({ft.from_skype_name})'

                    receiver = ft.to_skype_account
                    if ft.to_skype_name:
                        receiver += f' ({ft.to_skype_name})'

                    entry.metadata.set('sender-account', sender)
                    entry.metadata.set('receiver-account', receiver)
                    entry.metadata.set('profile-id', profile.name)
                    entry.metadata.set('profile-path', profile.path)

                    self.__entries.append(entry)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from µTorrent
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_utorrent(self):
        try:
            model = pymobius.app.utorrent.model(self.__item)

            for profile in model.get_profiles():
                self.__retrieve_utorrent_profile(profile)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from µTorrent profile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_utorrent_profile(self, profile):
        try:
            for f in profile.get_downloaded_files():
                entry = pymobius.Data()

                entry.username = profile.username
                entry.timestamp = f.added_time
                entry.filename = f.name
                entry.path = f.path
                entry.app_id = f.app_id
                entry.app_name = f.app_name
                entry.item = self.__item.name

                entry.metadata = mobius.core.pod.map()
                entry.metadata.set('size', f.size)
                entry.metadata.set('start-time', f.added_time)
                entry.metadata.set('end-time', f.completed_time)
                entry.metadata.set('caption', f.caption)
                entry.metadata.set('last-time-seen-complete', f.last_seen_complete_time)
                entry.metadata.set('resume.dat-path', f.resume_dat_path)
                entry.metadata.set('torrent-path', f.torrent_path)

                if f.download_url:
                    entry.metadata.set('url', f.download_url)

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
