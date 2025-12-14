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
import pymobius.app.gecko
import pymobius.app.internet_explorer

ANT_ID = 'bookmarked-urls'
ANT_NAME = 'Bookmarked URLs'
ANT_VERSION = '1.2'
EVIDENCE_TYPE = 'bookmarked-url'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: URL Bookmarks
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
        try:
            for entry in profile.get_bookmarks():
                bookmark = pymobius.Data()
                bookmark.url = entry.url
                bookmark.name = entry.name
                bookmark.creation_time = entry.creation_time
                bookmark.folder = entry.folder
                bookmark.username = entry.username
                bookmark.app_name = profile.app_name
                bookmark.evidence_source = entry.evidence_source

                bookmark.metadata = mobius.core.pod.map()
                bookmark.metadata.set('id', entry.id)
                bookmark.metadata.set('last_modification_time', entry.last_modification_time)
                bookmark.metadata.set('profile-id', profile.name)
                bookmark.metadata.set('profile-path', profile.path)

                if profile.creation_time:
                    bookmark.metadata.set('profile-creation-time', profile.creation_time)

                bookmark.metadata.set('app-id', profile.app_id)
                bookmark.metadata.set('app-name', profile.app_name)

                self.__entries.append(bookmark)
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
            for entry in profile.get_favorites():
                bookmark = pymobius.Data()
                bookmark.url = entry.url
                bookmark.name = entry.name
                bookmark.creation_time = entry.creation_time
                bookmark.folder = entry.folder
                bookmark.username = profile.username
                bookmark.app_name = profile.app_name
                bookmark.evidence_source = entry.evidence_source

                bookmark.metadata = mobius.core.pod.map()
                bookmark.metadata.set('last-modification-time', entry.last_modification_time)
                bookmark.metadata.set('profile-id', profile.name)
                bookmark.metadata.set('profile-path', profile.path)

                if profile.creation_time:
                    bookmark.metadata.set('profile-creation-time', profile.creation_time)

                bookmark.metadata.set('app-id', profile.app_id)
                bookmark.metadata.set('app-name', profile.app_name)

                self.__entries.append(bookmark)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        transaction = self.__item.new_transaction()

        # save evidence
        for b in self.__entries:
            bookmark = self.__item.new_evidence(EVIDENCE_TYPE)
            bookmark.url = b.url
            bookmark.evidence_source = b.evidence_source
            bookmark.app_name = b.app_name

            if b.name:
                bookmark.name = b.name

            if b.creation_time:
                bookmark.creation_time = b.creation_time

            if b.folder:
                bookmark.folder = b.folder

            if b.username:
                bookmark.username = b.username

            bookmark.metadata = b.metadata

        # commit data
        transaction.commit()
