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
import pymobius.app.gecko
import pymobius.app.internet_explorer

ANT_ID = 'visited-urls'
ANT_NAME = 'Visited URLs'
ANT_VERSION = '1.2'
EVIDENCE_TYPE = 'visited-url'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: visited-urls
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
        if not self.__item.has_datasource():
            return

        self.__entries = []

        self.__retrieve_gecko()
        self.__retrieve_internet_explorer()

        self.__save_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Gecko based browsers
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
        for entry in profile.get_history():
            if not entry.url.startswith('file://'):
                v = pymobius.Data()
                v.timestamp = entry.timestamp
                v.url = entry.url
                v.title = entry.title
                v.username = entry.username
                v.evidence_source = entry.evidence_source

                v.metadata = mobius.core.pod.map()
                v.metadata.set('id', entry.id)
                v.metadata.set('typed', entry.typed)
                v.metadata.set('visit-type', entry.visit_type)
                v.metadata.set('profile-id', profile.name)
                v.metadata.set('profile-path', profile.path)

                if profile.creation_time:
                    v.metadata.set('profile-creation-time', profile.creation_time)

                v.metadata.set('app-id', profile.app_id)
                v.metadata.set('app-name', profile.app_name)

                self.__entries.append(v)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Internet Explorer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_internet_explorer(self):
        try:
            model = pymobius.app.internet_explorer.model(self.__item)

            for profile in model.get_profiles():
                self.__retrieve_internet_explorer_profile(profile)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Internet Explorer profile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_internet_explorer_profile(self, profile):
        for entry in profile.get_history():
            if not entry.url.startswith('file://') and entry.timestamp_utc:
                v = pymobius.Data()
                v.timestamp = entry.timestamp
                v.url = entry.url
                v.title = entry.title or ''
                v.username = entry.username
                v.evidence_source = ''

                v.metadata = mobius.core.pod.map()
                v.metadata.set('file-path', entry.index_dat_path)
                v.metadata.set('file-type', entry.file_type)
                v.metadata.set('file-creation-time', entry.index_dat_creation_time)
                v.metadata.set('file-last-modification-time', entry.index_dat_last_modification_time)
                v.metadata.set('record-offset', f'0x{entry.offset:08x}')
                v.metadata.set('record-type', entry.type)
                v.metadata.set('record-size', entry.size)
                v.metadata.set('record-primary-time', entry.primary_time)
                v.metadata.set('record-secondary-time', entry.secondary_time)

                local_time = entry.tags.get(0x18)
                if local_time:
                    v.metadata.set('local-time-tag-0x18', local_time)

                if entry.type == 'URL':
                    v.metadata.set('expiration-time', entry.expiration_time)
                    v.metadata.set('last-sync-time', entry.last_sync_time)
                    v.metadata.set('hits', entry.hits)
                    v.metadata.set('cached-file-path', entry.filename)

                v.metadata.set('profile-id', profile.name)
                v.metadata.set('profile-path', profile.path)

                if profile.creation_time:
                    v.metadata.set('profile-creation-time', profile.creation_time)

                v.metadata.set('app-id', profile.app_id)
                v.metadata.set('app-name', profile.app_name)

                self.__entries.append(v)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        transaction = self.__item.new_transaction()

        # save evidences
        for e in self.__entries:
            obj = self.__item.new_evidence(EVIDENCE_TYPE)
            obj.timestamp = e.timestamp
            obj.url = e.url
            obj.title = e.title or ''
            obj.username = e.username
            obj.evidence_source = e.evidence_source
            obj.metadata = e.metadata

        # set ant run
        transaction.commit()
