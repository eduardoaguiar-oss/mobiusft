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
import pymobius.operating_system


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieves CREDHIST hashes
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.name = 'Turing CREDHIST Hashes'
        self.version = '1.0'
        self.__item = item

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__secrets = []
        self.__count = 0
        self.__unique_hashes = set()

        try:
            self.__retrieve_credhist_hashes()
        except Exception as e:
            mobius.core.logf('WRN ' + str(e) + ' ' + traceback.format_exc())

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
        mobius.core.logf('INF %d/%d CREDHIST hash(es) decrypted' % (self.__count, len(self.__secrets)))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_test_password</i>: Process password object
    # @param ant Mediator ant object
    # @param password Password value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_test_password(self, ant, password):
        for secret in self.__secrets:
            entry = secret.entry

            if not entry.is_decrypted() and entry.decrypt_with_password(password):
                self.__create_hashes_from_secret(ant, secret)
                self.__count += 1

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
            entry = secret.entry

            if not entry.is_decrypted() and entry.decrypt_with_password_hash(h.value):
                self.__create_hashes_from_secret(ant, secret)
                self.__count += 1

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Create hashes from decrypted entry
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __create_hashes_from_secret(self, ant, secret):
        entry = secret.entry

        # SHA-1
        h = pymobius.Data()
        h.type = 'sha1.utf16'
        h.value = entry.hash_sha1
        h.password = None
        h.pwd_type = 'os.user'
        h.description = 'User ' + secret.username + ' previous SHA-1 hash #%d' % (secret.idx + 1)

        h.metadata = []
        h.metadata.append(('username', secret.username))
        h.metadata.append(('source', secret.source))
        ant.add_hash(h)

        # NT
        h = pymobius.Data()
        h.type = 'nt'
        h.value = entry.hash_ntlm
        h.password = None
        h.pwd_type = 'os.user'
        h.description = 'User ' + secret.username + ' previous hash #%d' % (secret.idx + 1)

        h.metadata = []
        h.metadata.append(('username', secret.username))
        h.metadata.append(('source', secret.source))
        ant.add_hash(h)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve password hashes from CREDHIST files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_credhist_hashes(self):

        for opsys in pymobius.operating_system.scan(self.__item):
            for user_profile in opsys.get_profiles():
                f = user_profile.get_entry_by_path('%appdata%/Microsoft/Protect/CREDHIST')

                if f and not f.is_reallocated() and f.is_file():
                    self.__retrieve_credhist_from_file(f, user_profile.username)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve password hashes from CREDHIST file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_credhist_from_file(self, f, username):

        # parse CREDHIST file
        try:
            reader = f.new_reader()

            if not reader:
                return

            credhist = mobius.os.win.dpapi.credhist_file(reader)

        except Exception as e:
            mobius.core.logf('WRN ' + str(e) + ' ' + traceback.format_exc())
            return

        f.set_handled()

        # store secrets
        for idx, entry in enumerate(reversed(credhist.entries)):
            secret = pymobius.Data()
            secret.entry = entry
            secret.idx = idx
            secret.source = f.path.replace('/', '\\')
            secret.username = username

            self.__secrets.append(secret)
