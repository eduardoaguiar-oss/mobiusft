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
import traceback

import mobius
import pymobius
import pymobius.app.edge
import pymobius.app.gecko
import pymobius.app.internet_explorer

ANT_ID = 'cookies'
ANT_NAME = 'Cookies'
ANT_VERSION = '1.4'
EVIDENCE_TYPE = 'cookie'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: Cookies
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

        self.__retrieve_edge()
        self.__retrieve_gecko()
        self.__retrieve_internet_explorer()

        self.__save_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Microsoft Edge
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_edge(self):
        try:
            model = pymobius.app.edge.model(self.__item)

            for profile in model.get_profiles():
                self.__retrieve_edge_profile(profile)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Microsoft Edge profile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_edge_profile(self, profile):
        try:
            for entry in profile.get_cookies():
                cookie = pymobius.Data()
                cookie.name = entry.name
                cookie.value = entry.value
                cookie.domain = entry.domain
                cookie.creation_time = entry.creation_time
                cookie.last_access_time = entry.last_access_time
                cookie.expiration_time = entry.expiration_time
                cookie.last_update_time = None
                cookie.evidence_source = entry.evidence_source
                cookie.is_deleted = entry.is_deleted
                cookie.is_encrypted = False
                cookie.username = profile.username
                cookie.app_name = profile.app_name

                cookie.metadata = mobius.core.pod.map()
                cookie.metadata.set('profile-id', profile.name)
                cookie.metadata.set('profile-path', profile.path)

                if profile.creation_time:
                    cookie.metadata.set('profile-creation-time', profile.creation_time)

                cookie.metadata.set('app-id', profile.app_id)
                cookie.metadata.set('app-name', profile.app_name)

                self.__entries.append(cookie)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

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
    # @see http://doxygen.db48x.net/mozilla/html/interfacensIDownloadManager.html
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_gecko_profile(self, profile):
        try:
            for entry in profile.get_cookies():
                cookie = pymobius.Data()
                cookie.name = entry.name
                cookie.value = entry.value
                cookie.domain = entry.domain
                cookie.creation_time = entry.creation_time
                cookie.last_access_time = entry.last_access_time
                cookie.expiration_time = entry.expiration_time
                cookie.last_update_time = None
                cookie.evidence_source = entry.evidence_source
                cookie.is_deleted = entry.is_deleted
                cookie.is_encrypted = False
                cookie.username = profile.username
                cookie.app_name = profile.app_name

                cookie.metadata = mobius.core.pod.map()
                cookie.metadata.set('profile-id', profile.name)
                cookie.metadata.set('profile-path', profile.path)

                if profile.creation_time:
                    cookie.metadata.set('profile-creation-time', profile.creation_time)

                cookie.metadata.set('app-id', profile.app_id)
                cookie.metadata.set('app-name', profile.app_name)

                self.__entries.append(cookie)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

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
        try:
            for entry in profile.get_cookies():
                cookie = pymobius.Data()
                cookie.name = entry.name
                cookie.value = entry.value
                cookie.domain = entry.domain
                cookie.creation_time = entry.creation_time
                cookie.last_access_time = entry.last_access_time
                cookie.expiration_time = entry.expiration_time
                cookie.last_update_time = None
                cookie.evidence_source = entry.evidence_source
                cookie.is_deleted = entry.is_deleted
                cookie.is_encrypted = False
                cookie.username = profile.username
                cookie.app_name = profile.app_name

                cookie.metadata = mobius.core.pod.map()
                cookie.metadata.set('profile-id', profile.name)
                cookie.metadata.set('profile-path', profile.path)

                if profile.creation_time:
                    cookie.metadata.set('profile-creation-time', profile.creation_time)

                cookie.metadata.set('app-id', profile.app_id)
                cookie.metadata.set('app-name', profile.app_name)

                self.__entries.append(cookie)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        transaction = self.__item.new_transaction()

        # save evidences
        for c in self.__entries:
            cookie = self.__item.new_evidence(EVIDENCE_TYPE)

            cookie.name = c.name
            cookie.value = c.value
            cookie.is_encrypted = c.is_encrypted
            cookie.is_deleted = bool(c.is_deleted)
            cookie.domain = c.domain
            cookie.creation_time = c.creation_time
            cookie.last_access_time = c.last_access_time
            cookie.expiration_time = c.expiration_time

            if c.last_update_time:
                cookie.last_update_time = c.last_update_time
            elif c.creation_time == c.last_access_time:
                cookie.last_update_time = c.creation_time
            else:
                cookie.last_update_time = None

            cookie.username = c.username
            cookie.app_name = c.app_name
            cookie.evidence_source = c.evidence_source
            cookie.metadata = c.metadata

        # commit data
        transaction.commit()
