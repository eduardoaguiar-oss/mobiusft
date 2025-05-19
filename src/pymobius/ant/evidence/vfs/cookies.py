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
import mobius.core.crypt
import mobius.core.os
import pymobius
import pymobius.ant.turing
import pymobius.app.chromium
import pymobius.app.edge
import pymobius.app.gecko
import pymobius.app.internet_explorer

ANT_ID = 'cookies'
ANT_NAME = 'Cookies'
ANT_VERSION = '1.3'
EVIDENCE_TYPE = 'cookie'
DPAPI_GUID = b'\xd0\x8c\x9d\xdf\x01\x15\xd1\x11\x8c\x7a\x00\xc0\x4f\xc2\x97\xeb'


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
        self.__v10_master_keys = []
        self.__dpapi_master_keys = {}

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        if not self.__item.has_datasource():
            return

        # run ant.turing, if necessary
        if not self.__item.has_ant('turing'):
            ant = pymobius.ant.turing.Ant(self.__item)
            ant.run()

        # load master keys to decrypt cookies
        self.__v10_master_keys = []
        self.__dpapi_master_keys = {}

        for key in self.__item.get_evidences(pymobius.ant.turing.ENCRYPTION_KEY_EVIDENCE_TYPE):
            if key.key_type == 'dpapi.user':
                self.__dpapi_master_keys[key.id] = key.value

            elif key.key_type == 'chromium.v10':
                self.__v10_master_keys.append(key.value)

        # retrieve evidence
        self.__entries = []

        self.__retrieve_chromium()
        self.__retrieve_edge()
        self.__retrieve_gecko()
        self.__retrieve_internet_explorer()

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
        try:
            for entry in profile.get_cookies():
                cookie = pymobius.Data()
                cookie.name = entry.name
                cookie.value = entry.value
                cookie.domain = entry.domain
                cookie.creation_time = entry.creation_time
                cookie.last_access_time = entry.last_access_time
                cookie.expiration_time = None
                cookie.last_update_time = entry.last_update_time
                cookie.evidence_source = entry.evidence_source
                cookie.is_deleted = entry.is_deleted
                cookie.is_encrypted = entry.is_encrypted
                cookie.evidence_source = entry.evidence_source
                cookie.username = profile.username
                cookie.app_name = profile.app_name

                self.expiration_time = None

                cookie.metadata = mobius.core.pod.map()
                cookie.metadata.set('profile-id', profile.name)
                cookie.metadata.set('profile-path', profile.path)

                if profile.creation_time:
                    cookie.metadata.set('profile-creation-time', profile.creation_time)

                cookie.metadata.set('app-id', profile.app_id)
                cookie.metadata.set('app-name', profile.app_name)

                if cookie.is_encrypted:
                    self.__decrypt_cookie(cookie)

                self.__entries.append(cookie)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Try to decrypt cookie value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __decrypt_cookie(self, cookie):
        try:
            # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            # DPAPI encrypted cookies
            # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            if DPAPI_GUID in cookie.value:
                blob = mobius.core.os.win.dpapi.blob(cookie.value)
                mk_guid = blob.master_key_guid
                key_value = self.__dpapi_master_keys.get(mk_guid)

                if key_value:
                    if blob.decrypt(key_value):
                        cookie.is_encrypted = False
                        cookie.value = blob.plain_text

            # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            # v10 encrypted cookies
            # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            elif cookie.value.startswith(b'v10'):
                payload = cookie.value
                version = payload[:3]
                iv = payload[3:15]
                ciphertext = payload[15:-16]
                tag = payload[-16:]

                for key_value in self.__v10_master_keys:
                    cipher = mobius.core.crypt.new_cipher_gcm('aes', key_value, iv)
                    plaintext = cipher.decrypt(ciphertext)

                    if cipher.check_tag(tag):
                        mobius.core.logf(f"INF cookie {version} decrypted")
                        cookie.is_encrypted = False
                        cookie.value = plaintext
                        return

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

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
