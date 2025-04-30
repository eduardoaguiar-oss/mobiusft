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
import re
import binascii
import traceback
import mobius
import pymobius.operating_system


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Generic dataholder class
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class dataholder(object):
    pass


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
        self.name = 'Turing Win Wifi passwords'
        self.version = '1.0'
        self.__item = item

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__secrets = []

        for opsys in pymobius.operating_system.scan(self.__item):
            for root in opsys.get_root_folders():
                try:
                    self.__retrieve_secrets(root)
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
        count = len([secret for secret in self.__secrets if secret.blob.is_decrypted()])

        mobius.core.logf(f'INF ant.turing: {count:d}/{len(self.__secrets):d} wifi password(s) decrypted')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_key</i>: Process key object
    # @param ant Mediator ant object
    # @param key Key object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_key(self, ant, key):
        if key.type != 'dpapi.sys':
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

        if blob.decrypt(key.value):
            value = blob.plain_text.rstrip(b'\x00')

            if b'\x00' not in value:
                p = dataholder()
                p.type = 'net.wifi'
                p.value = value.decode('utf-8')
                p.description = "Wifi password. SSID: " + secret.ssid

                p.metadata = []
                p.metadata.append(('Source', secret.source))
                p.metadata.append(('SSID', secret.ssid))

                ant.add_password(p)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve wifi passwords
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_secrets(self, root):
        folder = root.get_child_by_path('ProgramData/Microsoft/Wlansvc/Profiles/Interfaces')

        if folder:
            for child in folder.get_children():
                if child.is_folder() and child.name[0] == '{':
                    self.__retrieve_wifi_passwords_from_interface_folder(child)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve wifi passwords from interface folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_wifi_passwords_from_interface_folder(self, folder):
        for child in folder.get_children():
            if child.is_file() and child.name.lower().endswith('.xml'):
                try:
                    self.__retrieve_wifi_passwords_from_xml(child)
                except Exception as e:
                    mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve wifi passwords from interface folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_wifi_passwords_from_xml(self, f):
        if not f or f.is_reallocated():
            return

        reader = f.new_reader()
        if not reader:
            return

        # parse data from file
        data = reader.read().decode('utf-8')
        name = ''
        key_material = ''
        protected = False

        match = re.search('<name>(.*?)</name>', data, re.IGNORECASE)
        if match:
            name = match.group(1)

        match = re.search('<protected>(.*?)</protected>', data, re.IGNORECASE)
        if match:
            protected = match.group(1) == 'true'

        match = re.search('<keyMaterial>(.*?)</keyMaterial>', data, re.IGNORECASE)
        if match:
            key_material = match.group(1).strip()

        if not protected or name == 'WFD_GROUP_OWNER_PROFILE':
            return

        # store secret
        secret = dataholder()
        secret.blob = mobius.os.win.dpapi.blob(binascii.unhexlify(key_material))
        secret.source = f.path.replace('/', '\\')
        secret.ssid = name

        self.__secrets.append(secret)
