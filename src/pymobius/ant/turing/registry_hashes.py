# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import traceback

import mobius
import pymobius
import pymobius.registry.cached_credentials
import pymobius.registry.main
import pymobius.registry.user_accounts


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: turing.registry_passwords
# @author Eduardo Aguiar
# Retrieves passwords from win registry
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.name = 'Turing Registry passwords'
        self.version = '1.0'
        self.__item = item

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__hashes = []

        try:
            ant = pymobius.registry.main.Ant(self.__item)

            for registry in ant.get_data():
                self.__retrieve_password_hashes_from_registry(registry)
        except Exception as e:
            traceback.print_exc()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_export_data</i>: Export data to mediator ant
    # @param ant Mediator ant object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_export_data(self, ant):
        for h in self.__hashes:
            ant.add_hash(h)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_stop</i>: Terminate this ant
    # @param ant Mediator ant object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_stop(self, ant):
        mobius.core.logf('INF ant.turing: %d hash(es) retrieved from registry' % len(self.__hashes))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve password hashes from registry
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_password_hashes_from_registry(self, registry):
        self.__retrieve_password_hashes_from_registry_user_accounts(registry)
        self.__retrieve_password_hashes_from_registry_cached_credential(registry)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve password hashes from user accounts
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_password_hashes_from_registry_user_accounts(self, registry):

        for account in pymobius.registry.user_accounts.get(registry):
            self.__retrieve_password_hashes_from_registry_user_account(account)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve password hashes from user account
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_password_hashes_from_registry_user_account(self, account):
        h_set = set()

        for h in account.hashes:
            k = (h.type, h.value, h.is_current)

            if k not in h_set:
                h_set.add(k)

                if h.is_current:
                    description = 'User ' + account.username + ' hash'
                else:
                    description = 'User ' + account.username + ' previous hash'

                d = pymobius.Data()
                d.type = h.type
                d.value = h.value
                d.password = None
                d.description = description
                d.pwd_type = 'os.user'

                d.metadata = []
                d.metadata.append(('user_rid', str(account.rid)))
                d.metadata.append(('user_gid', str(account.gid)))
                d.metadata.append(('username', account.username))
                d.metadata.append(('fullname', account.fullname))
                d.metadata.append(('admin_comment', account.admin_comment))
                d.metadata.append(('user_comment', account.user_comment))
                d.metadata.append(('is_current', str(h.is_current)))

                self.__hashes.append(d)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve password hashes from cached credential
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_password_hashes_from_registry_cached_credential(self, registry):

        for name, credential in pymobius.registry.cached_credentials.get(registry):

            if credential.encryption_algorithm == 0:
                t = 'msdcc1'

            elif credential.encryption_algorithm == 10:
                t = 'msdcc2'

            else:
                mobius.core.logf(
                    'DEV ant.turing: unknown credential encryption algorithm (%d)' % credential.encryption_algorithm)
                t = None

            if t:
                d = pymobius.Data()
                d.type = t
                d.value = credential.mscachehash
                d.password = None
                d.description = 'User ' + credential.username + ' cached credential'
                d.pwd_type = 'os.user'

                d.metadata = []
                d.metadata.append(('username', credential.username))

                if t == 'msdcc2':
                    d.metadata.append(('iterations', 10240))

                self.__hashes.append(d)
