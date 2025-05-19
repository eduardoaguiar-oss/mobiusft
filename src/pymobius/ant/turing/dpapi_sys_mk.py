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
import pymobius.registry.main


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: turing.dpapi_sys_mk
# @author Eduardo Aguiar
# Retrieves and decrypts DPAPI System Master Keys
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.name = 'Turing DPAPI System Master Keys'
        self.version = '1.0'
        self.__item = item

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__dpapi_system = []
        self.__keys = []
        self.__count = 0

        try:
            ant = pymobius.registry.main.Ant(self.__item)

            for registry in ant.get_data():
                self.__retrieve_dpapi_system_from_registry(registry)
        except Exception as e:
            mobius.core.logf('WRN ' + str(e) + ' ' + traceback.format_exc())

        try:
            self.__retrieve_dpapi_system_master_keys_files()
        except Exception as e:
            mobius.core.logf('WRN ' + str(e) + ' ' + traceback.format_exc())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_export_data</i>: Export data to mediator ant
    # @param ant Mediator ant object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_export_data(self, ant):
        for key in self.__keys:
            ant.add_key(key)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_stop</i>: Terminate this ant
    # @param ant Mediator ant object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_stop(self, ant):
        mobius.core.logf('INF %d/%d system master key(s) decrypted' % (len(self.__keys), self.__count))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve DPAPI_SYSTEM values from registry
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_dpapi_system_from_registry(self, registry):
        key = registry.get_key_by_path('\\HKLM\\SECURITY\\Policy\\Secrets\\DPAPI_SYSTEM')

        if key:
            data = key.get_data_by_path('CurrVal\\(default)')
            self.__retrieve_dpapi_system_from_value(data)

            data = key.get_data_by_path('OldVal\\(default)')
            self.__retrieve_dpapi_system_from_value(data)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Decoder DPAPI_SYSTEM value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_dpapi_system_from_value(self, data):

        if data and data.size == 44:
            decoder = mobius.decoder.data_decoder(data.data)

            revision = decoder.get_uint32_le()
            local_credential = decoder.get_bytearray_by_size(20)
            user_credential = decoder.get_bytearray_by_size(20)

            if local_credential not in self.__dpapi_system:
                self.__dpapi_system.append(local_credential)

            if user_credential not in self.__dpapi_system:
                self.__dpapi_system.append(user_credential)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve and decrypt DPAPI system master keys
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_dpapi_system_master_keys_files(self):
        for opsys in pymobius.operating_system.scan(self.__item):
            for root in opsys.get_root_folders():
                folder = root.get_child_by_path('windows/system32/microsoft/protect/s-1-5-18')
                if folder:
                    self.__retrieve_dpapi_system_master_keys_from_folder(folder)

                    folder = folder.get_child_by_name('User')
                    if folder:
                        self.__retrieve_dpapi_system_master_keys_from_folder(folder)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve and decrypt DPAPI system master keys from folder
    # @param folder Folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_dpapi_system_master_keys_from_folder(self, folder):
        for child in folder.get_children():
            if child.is_file() and child.name.count('-') == 4:
                try:
                    self.__retrieve_dpapi_system_master_key_from_file(child)
                except Exception as e:
                    mobius.core.logf('WRN ' + str(e) + ' ' + traceback.format_exc())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve and decrypt DPAPI system master key from file
    # @param f File
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_dpapi_system_master_key_from_file(self, f):

        # parse master key file
        reader = f.new_reader()
        if not reader:
            return

        mkf = mobius.core.os.win.dpapi.master_key_file(reader)
        if not mkf:
            return

        self.__count += 1
        f.set_handled()

        # try to decrypt using DPAPI_SYSTEM keys
        mk = mkf.master_key

        for key in self.__dpapi_system:
            mk.decrypt_with_key(key)

            if mk.is_decrypted():
                key = self.__create_key(f, mkf, mk)
                self.__keys.append(key)
                return

        mobius.core.logf('INF Sys mk not decrypted: ' + mkf.guid)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Create DPAPI sys key
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __create_key(self, f, mkf, mk):
        key = pymobius.Data()
        key.type = 'dpapi.sys'
        key.id = mkf.guid
        key.value = mk.plain_text

        key.metadata = []
        key.metadata.append(('source', f.path.replace('/', '\\')))
        key.metadata.append(('DPAPI GUID', mkf.guid))
        key.metadata.append(('Revision', mk.revision))
        key.metadata.append(('Salt', mobius.encoder.hexstring(mk.salt)))
        key.metadata.append(('Iterations', mk.iterations))
        key.metadata.append(('Hash ID', '0x%04x' % mk.hash_id))
        key.metadata.append(('Cipher ID', '0x%04x' % mk.cipher_id))
        key.metadata.append(('Flags', mk.flags))

        return key
