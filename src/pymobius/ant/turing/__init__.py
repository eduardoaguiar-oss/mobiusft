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
import mobius
import pymobius.ant.turing.add_passwords_to_database
import pymobius.ant.turing.chromium_passwords
import pymobius.ant.turing.credentials
import pymobius.ant.turing.credhist_hashes
import pymobius.ant.turing.custom_dic
import pymobius.ant.turing.dpapi_sys_mk
import pymobius.ant.turing.dpapi_user_mk
import pymobius.ant.turing.ie_passwords
import pymobius.ant.turing.registry_hashes
import pymobius.ant.turing.registry_passwords
import pymobius.ant.turing.wifi_passwords
import pymobius.ant.turing.test_case_passwords
import pymobius.ant.turing.test_hash_passwords
import traceback


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ANT_ID = 'turing'
ANT_NAME = 'Turing'
ANT_VERSION = '1.1'
ENCRYPTION_KEY_EVIDENCE_TYPE = 'encryption-key'
PASSWORD_EVIDENCE_TYPE = 'password'
PASSWORD_HASH_EVIDENCE_TYPE = 'password-hash'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Export wordlist file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def exporter_wordlist(uri, data):
    passwords = set(row[1] for row in data.rows)

    # create writer
    f = mobius.core.io.new_file_by_url(uri)
    fp = mobius.core.io.text_writer(f.new_writer())

    # export data
    for pwd in sorted(passwords):
        fp.write(f'{pwd}\n')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Generic dataholder class
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class dataholder(object):
    pass


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: Turing
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.name = 'Turing Cryptographic Agent'
        self.version = '1.0'

        self.__item = item
        self.__passwords = []
        self.__hashes = []
        self.__keys = []

        self.__unique_passwords = set()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        datasource = self.__item.get_datasource()

        if not datasource:
            raise Exception('Datasource is not set')

        if not datasource.is_available():
            raise Exception('Datasource is not available')

        mobius.core.logf('INF ant.turing started')

        # create ants
        self.__ants = [
            pymobius.ant.turing.chromium_passwords.Ant(self.__item),
            pymobius.ant.turing.credentials.Ant(self.__item),
            pymobius.ant.turing.credhist_hashes.Ant(self.__item),
            pymobius.ant.turing.custom_dic.Ant(self.__item),
            pymobius.ant.turing.dpapi_sys_mk.Ant(self.__item),
            pymobius.ant.turing.dpapi_user_mk.Ant(self.__item),
            pymobius.ant.turing.ie_passwords.Ant(self.__item),
            pymobius.ant.turing.registry_hashes.Ant(self.__item),
            pymobius.ant.turing.registry_passwords.Ant(self.__item),
            pymobius.ant.turing.wifi_passwords.Ant(self.__item),
            pymobius.ant.turing.test_hash_passwords.Ant(self.__item),
            pymobius.ant.turing.test_case_passwords.Ant(self.__item),
            pymobius.ant.turing.add_passwords_to_database.Ant(self.__item)
        ]

        # run sub-ants
        for ant in self.__ants:
            try:
                mobius.core.logf(f'DBG ant.run started: {ant.name}')
                ant.run()
                mobius.core.logf(f'DBG ant.run ended: {ant.name}')
            except Exception as e:
                mobius.core.logf(f'WRN ({ant.name}.run) {str(e)}\n{traceback.format_exc()}')

        # consolidate data
        for ant in self.__ants:
            try:
                mobius.core.logf(f'DBG ant.on_export_data started: {ant.name}')
                ant.on_export_data(self)
                mobius.core.logf(f'DBG ant.on_export_data ended: {ant.name}')
            except Exception as e:
                mobius.core.logf(f'WRN ({ant.name}.on_export_data) {str(e)}\n{traceback.format_exc()}')

        # terminate sub-ants
        for ant in self.__ants:
            try:
                mobius.core.logf(f'DBG ant.on_stop started: {ant.name}')
                ant.on_stop(self)
                mobius.core.logf(f'DBG ant.on_stop ended: {ant.name}')
            except Exception as e:
                mobius.core.logf(f'WRN ({ant.name}.on_stop) {str(e)}\n{traceback.format_exc()}')

        # log info
        mobius.core.logf(f'INF passwords = {len(self.__passwords):d}')
        mobius.core.logf(f'INF hashes = {len(self.__hashes):d}')
        mobius.core.logf(f'INF keys = {len(self.__keys):d}')

        # save data
        self.__save_data()

        mobius.core.logf('INF finished')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        c = self.__item.case
        transaction = c.new_transaction()

        # save evidences
        self.__save_encryption_keys()
        self.__save_passwords()
        self.__save_password_hashes()

        # set ant run
        self.__item.set_ant(ANT_ID, ANT_NAME, ANT_VERSION)
        transaction.commit()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save encryption keys
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_encryption_keys(self):
        self.__item.remove_evidences(ENCRYPTION_KEY_EVIDENCE_TYPE)

        keys = [(k.type, k.id, k.value, k) for k in self.__keys]

        for k_type, k_id, k_value, k in sorted(keys):
            metadata = mobius.core.pod.map()

            for var_name, var_value in k.metadata:
                metadata.set(var_name, var_value)

            key = self.__item.new_evidence(ENCRYPTION_KEY_EVIDENCE_TYPE)
            key.key_type = k_type
            key.id = k_id
            key.value = k_value
            key.metadata = metadata

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save passwords
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_passwords(self):
        self.__item.remove_evidences(PASSWORD_EVIDENCE_TYPE)

        passwords = [(p.type, p.description, p.value, p) for p in self.__passwords]

        for p_type, p_description, p_value, p_obj in sorted(passwords, key=lambda x: x[:3]):
            metadata = mobius.core.pod.map()

            for var_name, var_value in p_obj.metadata:
                metadata.set(var_name, var_value)

            p = self.__item.new_evidence(PASSWORD_EVIDENCE_TYPE)
            p.password_type = p_type
            p.value = p_value
            p.description = p_description
            p.metadata = metadata

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save password hashes
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_password_hashes(self):
        self.__item.remove_evidences(PASSWORD_HASH_EVIDENCE_TYPE)

        hashes = [(h.type, h.description, h.value, h) for h in self.__hashes]

        for h_type, h_description, h_value, h_obj in sorted(hashes):
            metadata = mobius.core.pod.map()

            for var_name, var_value in h_obj.metadata:
                metadata.set(var_name, var_value)

            hash_value = mobius.core.encoder.hexstring(h_value)

            h = self.__item.new_evidence(PASSWORD_HASH_EVIDENCE_TYPE)
            h.password_hash_type = h_type
            h.value = hash_value
            h.description = h_description

            if h_obj.password is not None:
                h.password = h_obj.password

            h.metadata = metadata

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Add key to model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_key(self, key):
        self.__keys.append(key)

        for ant in self.__ants:
            try:
                f = getattr(ant, 'on_key', None)

                if f:
                    mobius.core.logf(f'DBG ant.on_key started: {ant.name}')
                    f(self, key)
                    mobius.core.logf(f'DBG ant.on_key ended: {ant.name}')
            except Exception as e:
                mobius.core.logf('WRN (%s.on_key) %s %s' % (ant.name, str(e), traceback.format_exc()))
                mobius.core.logf(traceback.format_exc())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Add password to model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_password(self, p):
        self.__passwords.append(p)

        for ant in self.__ants:
            try:
                f = getattr(ant, 'on_add_password', None)

                if f:
                    mobius.core.logf(f'DBG ant.on_add_password started: {ant.name}')
                    f(self, p)
                    mobius.core.logf(f'DBG ant.on_add_password ended: {ant.name}')
            except Exception as e:
                mobius.core.logf('WRN (%s.on_add_password) %s %s' % (ant.name, str(e), traceback.format_exc()))

        self.test_password(p.value)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Test password
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def test_password(self, value):
        if isinstance(value, bytes):
            mobius.core.logf('WRN bytes password %s' % '\n'.join(traceback.format_stack()))

        if value in self.__unique_passwords:
            return

        self.__unique_passwords.add(value)

        for ant in self.__ants:
            try:
                f = getattr(ant, 'on_test_password', None)

                if f:
                    mobius.core.logf(f'DBG ant.on_test_password started <{value}> {ant.name}')
                    f(self, value)
                    mobius.core.logf(f'DBG ant.on_test_password ended <{value}> {ant.name}')
            except Exception as e:
                mobius.core.logf('WRN (%s.on_test_password) %s %s' % (ant.name, str(e), traceback.format_exc()))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Add hash to model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_hash(self, h):
        self.__hashes.append(h)

        for ant in self.__ants:
            f = getattr(ant, 'on_hash', None)

            if f:
                f(self, h)
