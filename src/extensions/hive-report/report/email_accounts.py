# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023 Eduardo Aguiar
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
import pymobius

from common import *
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief email accounts report
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class EmailAccountsReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'email-accounts'
        self.name = 'E-mail accounts'
        self.group = 'app'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_report_name('%s of computer <%s>' % (self.name, get_computer_name(registry)))

        # fill data
        for username, user_key in iter_hkey_users(registry):
            sid = user_key.name
            pssp_key = user_key.get_key_by_path('Software\\Microsoft\\Protected Storage System Provider\\' + sid)

            # outlook 98-2000 and outlook express
            for subkey in user_key.get_key_by_mask('Software\\Microsoft\\Internet Account Manager\\Accounts\\*'):
                account_name = get_data_as_string(subkey.get_data_by_name('SMTP Display Name'))

                if account_name:
                    email_address = get_data_as_string(subkey.get_data_by_name('SMTP Email Address'))

                    # passwords
                    if pssp_key:
                        pop3_value_name = subkey.get_data_by_name('POP3 Password2')
                        pop3_password = self.get_pssp_password(pssp_key, pop3_value_name)

                        smtp_value_name = subkey.get_data_by_name('SMTP Password2')
                        smtp_password = self.get_pssp_password(pssp_key, smtp_value_name)

                        http_value_name = subkey.get_data_by_name('HTTPMail Password2')
                        http_password = self.get_pssp_password(pssp_key, http_value_name)

                    # password status
                    status_list = []

                    if pop3_password is not None:
                        status_list.append('POP3')

                    if smtp_password is not None:
                        status_list.append('SMTP')

                    if http_password is not None:
                        status_list.append('HTTPMail')

                    if status_list:
                        status = ', '.join(status_list) + ' found'
                    else:
                        status = 'not found'

                    # add data
                    self.viewer.add_row(
                        (username, account_name, email_address, pop3_password, smtp_password, http_password, status))

            # outlook 2002-2008
            for subkey in user_key.get_key_by_mask(
                    'Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows Messaging Subsystem\\Profiles\\*\\9375CFF0413111d3B88A00104B2A6676'):
                account_name_data = subkey.get_data_by_name('Display Name')

                if account_name_data:
                    account_name = get_data_as_string(account_name_data)
                    email_address = get_data_as_string(subkey.get_data_by_name('Email'))
                    dpapi_password = subkey.get_data_by_name('POP3 Password').data

                    self.viewer.add_row((username, account_name, email_address, None, None, None, 'not found'))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get Protected Storage password
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_pssp_password(self, pssp_key, idx_data):
        password = None

        if idx_data:
            value_name = idx_data.data[2:].decode('utf-16-le').rstrip('\0')
            data = pssp_key.get_data_by_path(
                'Data\\220d5cc1-853a-11d0-84bc-00c04fd43f8f\\417e2d75-84bd-11d0-84bb-00c04fd43f8f\\' + value_name)

            if data:
                password = data.data.rstrip('\0')

        return password

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'tableview')

        column = self.viewer.add_column('username')
        column.is_sortable = True

        column = self.viewer.add_column('name')
        column.is_sortable = True

        column = self.viewer.add_column('e-mail address')
        column.is_sortable = True

        column = self.viewer.add_column('POP3 password')
        column = self.viewer.add_column('SMTP password')
        column = self.viewer.add_column('HTTPMail password')
        column = self.viewer.add_column('password status')

        self.viewer.set_report_id('registry.' + self.id)
        self.viewer.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
