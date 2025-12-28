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
import pymobius.registry.main
from pymobius.registry import *

TBAL_VALUE_NAME = '_TBAL_{68EDDCF5-0AEB-4C28-A770-AF5302ECA3C9}'


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
        self.__passwords = []

        try:
            ant = pymobius.registry.main.Ant(self.__item)

            for registry in ant.get_data():
                self.__retrieve_passwords_from_registry_lsa(registry)
                self.__retrieve_passwords_from_registry_outlook(registry)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_export_data</i>: Export data to mediator ant
    # @param ant Mediator ant object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_export_data(self, ant):
        for password in self.__passwords:
            ant.add_password(password)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_stop</i>: Terminate this ant
    # @param ant Mediator ant object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_stop(self, ant):
        mobius.core.logf(f'INF {len(self.__passwords):d} password(s) retrieved from registry')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve passwords from LSA secrets
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_passwords_from_registry_lsa(self, registry):

        for key in registry.get_key_by_mask('\\HKLM\\SECURITY\\Policy\\Secrets\\*'):

            # @see https://msdn.microsoft.com/en-us/library/windows/desktop/aa378826(v=vs.85).aspx
            if key.name == 'DefaultPassword':
                description = 'Automatic logon'
                ptype = 'os.default'

            elif key.name == '0083343a-f925-4ed7-b1d6-d95d17a0b57b-RemoteDesktopHelpAssistantAccount':
                description = 'User HelpAssistant logon'
                ptype = 'os.user'

            elif key.name == 'aspnet_WP_PASSWORD':
                description = 'User ASPNET logon'
                ptype = 'os.user'

            elif key.name == 'SCM:{3D14228D-FBE1-11D0-995D-00C04FD919C1}':
                description = 'IIS IWAM'
                ptype = 'app.iis'

            # @see http://nvidia.custhelp.com/app/answers/detail/a_id/3067/~/what-is-nvidia-%E2%80%99updatususer%E2%80%99%3F
            elif key.name == '_SC_nvUpdatusService':
                description = 'User UpdatusUser logon'
                ptype = 'os.user'

            elif key.name.startswith('_SC_DB2'):
                description = 'DB2'
                ptype = 'app.db2'

            elif key.name.startswith('_SC_postgresql-') or key.name.startswith('_SC_pgsql-'):
                description = 'PostgreSQL'
                ptype = 'app.postgresql'

            else:
                description = None
                ptype = None

            # add current and old passwords
            if description:
                currval = get_data_as_string(key.get_data_by_path('Currval\\(default)'))

                if currval and currval != TBAL_VALUE_NAME:
                    p = pymobius.Data()
                    p.type = ptype
                    p.value = currval
                    p.description = description + ' password'

                    p.metadata = []
                    p.metadata.append(('source', 'LSA key %s currval' % key.name))

                    self.__passwords.append(p)

                oldval = get_data_as_string(key.get_data_by_path('Oldval\\(default)'))

                if oldval and currval != TBAL_VALUE_NAME:
                    p = pymobius.Data()
                    p.type = ptype
                    p.value = oldval
                    p.description = description + ' old password'

                    p.metadata = []
                    p.metadata.append(('source', 'LSA key %s oldval' % key.name))

                    self.__passwords.append(p)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve passwords for Outlook Express and Outlook 98-2000
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_passwords_from_registry_outlook(self, registry):

        for user_key in registry.get_key_by_mask('\\HKU\\*'):
            sid = user_key.name
            pssp_key = user_key.get_key_by_path('Software\\Microsoft\\Protected Storage System Provider\\' + sid)

            for subkey in user_key.get_key_by_mask('Software\\Microsoft\\Internet Account Manager\\Accounts\\*'):
                account_name = get_data_as_string(subkey.get_data_by_name('SMTP Display Name'))
                email_address = get_data_as_string(subkey.get_data_by_name('SMTP Email Address'))

                # POP3 password
                pop3_value_name = subkey.get_data_by_name('POP3 Password2')
                pop3_password = self.__get_pssp_password(pssp_key, pop3_value_name)

                if pop3_password:
                    p = pymobius.Data()
                    p.type = 'email.pop3'
                    p.value = pop3_password
                    p.description = "E-mail " + email_address + " POP3 password"

                    p.metadata = []
                    p.metadata.append(('source', 'PSSP POP3 Password2 value'))
                    p.metadata.append(('email', email_address))
                    p.metadata.append(('user_sid', sid))
                    self.__passwords.append(p)

                # SMTP password
                smtp_value_name = subkey.get_data_by_name('SMTP Password2')
                smtp_password = self.__get_pssp_password(pssp_key, smtp_value_name)

                if smtp_password:
                    p = pymobius.Data()
                    p.type = 'email.smtp'
                    p.value = smtp_password
                    p.description = "E-mail " + email_address + " SMTP password"

                    p.metadata = []
                    p.metadata.append(('source', 'PSSP SMTP Password2 value'))
                    p.metadata.append(('email', email_address))
                    p.metadata.append(('user_sid', sid))
                    self.__passwords.append(p)

                # HTTP password
                http_value_name = subkey.get_data_by_name('HTTPMail Password2')
                http_password = self.__get_pssp_password(pssp_key, http_value_name)

                if http_password:
                    p = pymobius.Data()
                    p.type = 'email.http'
                    p.value = http_password
                    p.description = "E-mail " + email_address + " HTTP password"

                    p.metadata = []
                    p.metadata.append(('source', 'PSSP HTTPMail Password2 value'))
                    p.metadata.append(('email', email_address))
                    p.metadata.append(('user_sid', sid))
                    self.__passwords.append(p)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get Protected Storage password
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_pssp_password(self, pssp_key, idx_data):
        password = None

        if pssp_key and idx_data:
            value_name = idx_data.data[2:].decode('utf-16-le').rstrip('\0')
            data = pssp_key.get_data_by_path(
                'Data\\220d5cc1-853a-11d0-84bc-00c04fd43f8f\\417e2d75-84bd-11d0-84bb-00c04fd43f8f\\' + value_name)

            if data:
                password = data.data.rstrip('\0')

        return password
