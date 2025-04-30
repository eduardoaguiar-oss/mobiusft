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
import pymobius.registry.main


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieves and decrypts Internet Explorer Passwords
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.name = 'Turing IE Passwords'
        self.version = '1.0'
        self.__item = item
        self.__secrets = []
        self.__decrypted_count = 0
        self.__total_count = 0
        self.__turing = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        try:
            self.__turing = mobius.turing.turing()
            self.__retrieve_ie_passwords()
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
        mobius.core.logf(
            f'INF {self.__decrypted_count:d}/{self.__total_count:d} Internet Explorer password(s) decrypted')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_key</i>: Process key object
    # @param ant Mediator ant object
    # @param key Key object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_key(self, ant, key):
        if key.type != 'dpapi.user':
            return

        for secret in self.__secrets:
            blob = secret.blob

            if not blob.is_decrypted() and blob.master_key_guid == key.id:
                self.__decrypt_secret(ant, secret, key)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Try to decrypt secret
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __decrypt_secret(self, ant, secret, key):
        blob = secret.blob

        if blob.decrypt(key.value, secret.entropy):
            passwords = self.__retrieve_ie_passwords_from_data(blob.plain_text, secret.url)

            for p in passwords:
                p.metadata.append(('Source', f'Registry value {secret.source}'))
                ant.add_password(p)

            if passwords:
                self.__decrypted_count += 1
            else:
                self.__total_count -= 1

        else:
            mobius.core.logf('WRN IE DPAPI blob not decrypted')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve IE passwords
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_ie_passwords(self):
        try:
            ant = pymobius.registry.main.Ant(self.__item)

            for registry in ant.get_data():
                self.__retrieve_ie_passwords_from_registry(registry)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve IE password from registry
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_ie_passwords_from_registry(self, registry):

        for value in registry.get_value_by_mask(
                '\\HKU\\*\\Software\\Microsoft\\Internet Explorer\\IntelliForms\\Storage2\\*'):
            self.__total_count += 1

            status, url = self.__turing.get_hash_password('ie.entropy', value.name.lower())

            if status == 1:  # found
                secret = pymobius.Data()
                secret.blob = mobius.os.win.dpapi.blob(value.data.data)
                secret.entropy = (url + '\0').encode('utf-16le')
                secret.url = url
                secret.source = value.name

                self.__secrets.append(secret)

            else:
                mobius.core.logf(f'WRN IE entropy not found: {value.name}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve IE password from decrypted data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_ie_passwords_from_data(self, data, url):
        decoder = mobius.decoder.data_decoder(data)

        header_size = decoder.get_uint32_le()
        index_size = decoder.get_uint32_le()
        data_size = decoder.get_uint32_le()

        index_offset = header_size
        data_offset = header_size + index_size

        signature = decoder.get_string_by_size(4)
        if signature != 'WICK':
            return None

        pos = decoder.get_uint32_le()
        count = decoder.get_uint32_le()
        values = []
        decoder.seek(pos + index_offset)

        for i in range(count):
            offset = decoder.get_uint32_le()
            timestamp = decoder.get_nt_datetime()
            size = decoder.get_uint32_le()
            value = data[data_offset + offset: data_offset + offset + size * 2].decode('utf-16le')
            values.append(value)

        passwords = []

        for i in range(0, len(values), 2):
            username = values[i]
            password = values[i + 1]

            if password:
                p = pymobius.Data()
                p.type = 'net.http'
                p.value = password
                p.description = "Web password. URL: " + url

                p.metadata = []
                p.metadata.append(('URL', url))
                p.metadata.append(('User ID', username))
                p.metadata.append(('Application', 'Internet Explorer'))

                passwords.append(p)

        return passwords
