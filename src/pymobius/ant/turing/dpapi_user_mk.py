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
import pymobius.operating_system


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: turing.dpapi_user_mk
# @author Eduardo Aguiar
# Retrieves and decrypts DPAPI User Master Keys
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.name = 'Turing DPAPI User Master Keys'
        self.version = '1.0'
        self.__item = item

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__secrets = []
        self.__unique_hashes = set()

        try:
            self.__retrieve_dpapi_user_master_keys()
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
        count = len([secret for secret in self.__secrets if secret.mk.is_decrypted()])

        mobius.core.logf(f'INF {count:d}/{len(self.__secrets):d} user master key(s) decrypted')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_test_password</i>: Process password object
    # @param ant Mediator ant object
    # @param password Password value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_test_password(self, ant, password):
        for secret in self.__secrets:
            mk = secret.mk

            if not mk.is_decrypted() and mk.decrypt_with_password(secret.sid, password):
                key = self.__create_key(secret)
                ant.add_key(key)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_hash</i>: Process hash object
    # @param ant Mediator ant object
    # @param h Hash object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_hash(self, ant, h):
        if h.type != 'sha1.utf16' or (h.type, h.value) in self.__unique_hashes:
            return

        self.__unique_hashes.add((h.type, h.value))

        for secret in self.__secrets:
            mk = secret.mk

            if not mk.is_decrypted() and mk.decrypt_with_password_hash(secret.sid, h.value):
                key = self.__create_key(secret)
                ant.add_key(key)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Create key object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __create_key(self, secret):
        mk = secret.mk
        mkf = secret.mkf

        key = pymobius.Data()
        key.type = 'dpapi.user'
        key.id = mkf.guid
        key.value = mk.plain_text

        key.metadata = []
        key.metadata.append(('Source', secret.source))
        key.metadata.append(('User SID', secret.sid))
        key.metadata.append(('DPAPI GUID', mkf.guid))
        key.metadata.append(('Revision', mk.revision))
        key.metadata.append(('Salt', mobius.encoder.hexstring(mk.salt)))
        key.metadata.append(('Iterations', mk.iterations))
        key.metadata.append(('Hash ID', f'(0x{mk.hash_id:04x})'))
        key.metadata.append(('Cipher ID', f'(0x{mk.cipher_id:04x})'))
        key.metadata.append(('Flags', mk.flags))

        return key

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve DPAPI user master keys
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_dpapi_user_master_keys(self):
        for opsys in pymobius.operating_system.scan(self.__item):
            for user_profile in opsys.get_profiles():
                folder = user_profile.get_appdata_entry('Microsoft/Protect')

                if folder and not folder.is_reallocated():
                    for child in folder.get_children():
                        if child.is_folder() and child.name.startswith('S-1-'):
                            self.__retrieve_dpapi_user_master_keys_from_folder(child)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve DPAPI user master keys from SID folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_dpapi_user_master_keys_from_folder(self, folder):
        sid = folder.name

        for child in folder.get_children():
            if not child.is_reallocated() and child.is_file() and child.name.count('-') == 4:
                self.__retrieve_dpapi_user_master_keys_from_file(child, sid)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve DPAPI user master keys from MK file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_dpapi_user_master_keys_from_file(self, f, sid):
        reader = f.new_reader()

        if not reader:
            return

        try:
            mkf = mobius.core.os.win.dpapi.master_key_file(reader)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')
            return

        f.set_handled()

        secret = pymobius.Data()
        secret.mkf = mkf
        secret.mk = mkf.master_key
        secret.sid = sid
        secret.source = f.path.replace('/', '\\')

        self.__secrets.append(secret)
