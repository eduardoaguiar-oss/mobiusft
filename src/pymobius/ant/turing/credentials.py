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
import mobius.core.os
import pymobius
import pymobius.operating_system


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieves and decrypts Win Credentials
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.name = 'Turing Credentials'
        self.version = '1.0'
        self.__item = item

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__secrets = []
        self.__master_keys = {}
        self.__count = 0

        try:
            self.__retrieve_credentials()
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_export_data</i>: Export data to mediator ant
    # @param ant Mediator ant object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_export_data(self, ant):
        pass

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_stop</i>: Terminate this ant
    # @param ant Mediator ant object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_stop(self, ant):
        mobius.core.logf(f'INF {self.__count:d}/{len(self.__secrets):d} Credentials password(s) decrypted')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_key</i>: Process key object
    # @param ant Mediator ant object
    # @param key Key object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_key(self, ant, key):
        if key.type not in ('dpapi.sys', 'dpapi.user'):
            return

        self.__master_keys[key.id] = key.value

        for secret in self.__secrets:
            blob = secret.blob

            try:
                if not blob.is_decrypted() and blob.master_key_guid == key.id:
                    self.__decrypt_secret(ant, secret, key)
            except Exception as e:
                mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Try to decrypt secret
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __decrypt_secret(self, ant, secret, key):
        blob = secret.blob

        if blob.decrypt(key.value):
            p = self.__create_password_from_credential_data(blob.plain_text)

            if p:
                self.__count += 1
                p.metadata.append(('Credential file path', secret.path))
                ant.add_password(p)

        else:
            mobius.core.logf('WRN Credential DPAPI blob not decrypted')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve Credentials
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_credentials(self):
        self.__retrieve_sys_credentials()
        self.__retrieve_user_credentials()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve System Credentials
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_sys_credentials(self):
        for opsys in pymobius.operating_system.scan(self.__item):
            for root in opsys.get_root_folders():
                try:
                    # systemprofile
                    folder = root.get_child_by_path('Windows/System32/config/systemprofile')
                    self.__retrieve_sys_credentials_from_profile(folder)

                    # serviceprofiles
                    folder = root.get_child_by_path('Windows/serviceprofiles')

                    if folder and not folder.is_reallocated() and folder.is_folder():
                        for child in folder.get_children():
                            self.__retrieve_sys_credentials_from_profile(child)

                except Exception as e:
                    mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve System Credentials from profile folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_sys_credentials_from_profile(self, folder):
        try:
            if not folder or folder.is_reallocated() or not folder.is_folder():
                return

            f = folder.get_child_by_path('AppData/Local/Microsoft/Credentials')
            self.__retrieve_credentials_from_folder(f)

            f = folder.get_child_by_path('AppData/Roaming/Microsoft/Credentials')
            self.__retrieve_credentials_from_folder(f)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve User Credentials
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_user_credentials(self):
        try:
            for opsys in pymobius.operating_system.scan(self.__item):
                for user_profile in opsys.get_profiles():
                    self.__retrieve_credentials_from_profile(user_profile)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve Credentials from profile folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_credentials_from_profile(self, user_profile):
        try:
            folder = user_profile.get_entry_by_path('%appdata%/Microsoft/Credentials')
            self.__retrieve_credentials_from_folder(folder)

            folder = user_profile.get_entry_by_path('%localappdata%/Microsoft/Credentials')
            self.__retrieve_credentials_from_folder(folder)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve Credentials from folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_credentials_from_folder(self, folder):
        try:
            if not folder or folder.is_reallocated() or not folder.is_folder():
                return

            for child in folder.get_children():
                self.__retrieve_credentials_from_file(child)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve Credentials from file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_credentials_from_file(self, f):
        try:
            if not f or f.is_reallocated() or not f.is_file():
                return

            reader = f.new_reader()

            if not reader:
                return

            decoder = mobius.core.decoder.data_decoder(reader)
            revision = decoder.get_uint32_le()
            blob_size = decoder.get_uint64_le()

            if blob_size > 0:
                blob_data = decoder.get_bytearray_by_size(blob_size)

                secret = pymobius.Data()
                secret.blob = mobius.core.os.win.dpapi.blob(blob_data)
                secret.path = f.path.replace('/', '\\')

                self.__secrets.append(secret)

            f.set_handled()
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Create password from Credentials data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __create_password_from_credential_data(self, data):
        credential = mobius.core.os.win.credential(data)

        # handle blob data
        blob_data = b''
        for name, data in credential.data:
            blob_data += data

        if blob_data.startswith(b'\x01\x00\x00\x00'):
            b = mobius.core.os.win.dpapi.blob(blob_data)
            mk = self.__master_keys.get(b.master_key_guid)

            if mk and b.decrypt(mk):
                blob_data = b.plain_text
            else:
                mobius.core.logf('WRN Credential (virtualapp/didlogical) not decrypted')

        if blob_data:
            mobius.core.logf('DBG BLOB_DATA')
            mobius.core.logf('DBG ' + pymobius.dump(blob_data))

        # process credential according to domain
        if credential.domain.startswith('WindowsLive:'):
            p = self.__create_password_from_credential_windows_live(credential)

        elif credential.password:
            p = self.__create_password_from_credential_generic(credential)

        else:
            p = None
            mobius.core.logf('DEV Credential not handled -> ' + credential.domain)

            if credential.password_data:
                mobius.core.logf('DEV Credential password -> ' + pymobius.dump(credential.password_data))

            for name, value in credential.data:
                mobius.core.logf('DEV Credential data -> %s' % name)
                mobius.core.logf('DEV ' + pymobius.dump(value))

        return p

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Create password from Credentials
    # Domain: WindowsLive:
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __create_password_from_credential_windows_live(self, credential):
        p = None

        if credential.password:
            p = pymobius.Data()
            p.type = 'net.account'
            p.value = credential.password
            p.description = f'Windows Live password. Account: {credential.username}'

            p.metadata = []
            p.metadata.append(('Domain', credential.domain))
            p.metadata.append(('Account', credential.username))
            p.metadata.append(('Last update', credential.last_update_time))

        return p

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Create password from Credentials
    # Domain: Generic
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __create_password_from_credential_generic(self, credential):
        p = pymobius.Data()
        p.type = 'os.credential'
        p.value = credential.password
        p.description = 'Win Credential. Domain: ' + credential.domain

        p.metadata = []
        p.metadata.append(('Domain', credential.domain))
        p.metadata.append(('Account', credential.username))
        p.metadata.append(('Last update', credential.last_update_time))

        return p
