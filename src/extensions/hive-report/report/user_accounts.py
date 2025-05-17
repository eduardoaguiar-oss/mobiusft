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
import mobius.core.turing
import pymobius
import pymobius.registry.user_accounts

from common import *
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief user accounts report
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class UserAccountReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'user-accounts'
        self.name = 'Accounts'
        self.group = 'user'
        self.__turing = mobius.core.turing.turing()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_master_report_name(f'{self.name} of computer <{get_computer_name(registry)}>')

        # fill data
        STATUS = ['not found', 'found', 'first half found', 'last half found']

        for account in pymobius.registry.user_accounts.get(registry):
            nthash = None
            lmhash = None

            for h in account.hashes:
                if h.type == 'nt' and h.is_current:
                    nthash = mobius.encoder.hexstring(h.value)

                elif h.type == 'lm' and h.is_current:
                    lmhash = mobius.encoder.hexstring(h.value)

            # try to retrieve password
            if nthash:
                status, account.password = self.__turing.get_hash_password('nt', nthash)
                account.password_status = STATUS[status]

            elif lmhash:
                status, account.password = self.__turing.get_hash_password('lm', lmhash)
                account.password_status = STATUS[status]

            else:
                account.password = None
                account.password_status = 'no hashes'

            # add user
            self.viewer.add_row_from_object(account)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'widetableview')
        self.viewer.set_control(self)

        column = self.viewer.add_column('rid', column_name='RID', column_type='int')
        column.is_sortable = True

        column = self.viewer.add_column('username')
        column.is_sortable = True

        column = self.viewer.add_column('fullname')
        column.is_visible = False

        column = self.viewer.add_column('password')

        column = self.viewer.add_column('password_status', column_name='password status')
        column.is_sortable = True

        column = self.viewer.add_column('admin_comment', 'admin comment')
        column.is_visible = False

        column = self.viewer.add_column('user_comment', 'user comment')
        column.is_visible = False

        column = self.viewer.add_column('home_dir', 'home directory')
        column.is_visible = False

        column = self.viewer.add_column('home_dir_letter', 'home directory logical drive')
        column.is_visible = False

        column = self.viewer.add_column('logon_script', 'logon script')
        column.is_visible = False

        column = self.viewer.add_column('profile_path', 'profile path')
        column.is_visible = False

        column = self.viewer.add_column('workstation')
        column.is_visible = False

        column = self.viewer.add_column('gid', column_name='main group ID', column_type='int')
        column.is_visible = False

        column = self.viewer.add_column('last_logon_datetime', column_name='last logon date/time')
        column.is_visible = False

        column = self.viewer.add_column('last_password_set_datetime', column_name='last password set date/time')
        column.is_visible = False

        column = self.viewer.add_column('last_failed_logon_datetime', column_name='last failed logon date/time')
        column.is_visible = False

        column = self.viewer.add_column('password_expiration_datetime', column_name='password expiration date/time')
        column.is_visible = False

        column = self.viewer.add_column('logon_count', column_name='logon count')
        column.is_visible = False

        column = self.viewer.add_column('failed_logon_count', column_name='failed logon count')
        column.is_visible = False

        column = self.viewer.add_column('flag_account_enabled', column_name='is enabled')
        column.is_visible = False

        column = self.viewer.add_column('flag_home_dir_required', column_name='is home_dir required')
        column.is_visible = False

        column = self.viewer.add_column('flag_password_required', column_name='is password required')
        column.is_visible = False

        column = self.viewer.add_column('flag_normal_user_account', column_name='is normal user account')
        column.is_visible = False

        column = self.viewer.add_column('flag_password_expires', column_name='does password expires')
        column.is_visible = False

        self.viewer.set_master_report_id('registry.' + self.id)
        self.viewer.set_master_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')

        self.viewer.set_detail_report_id('registry.user-account')
        self.viewer.set_detail_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle main view selection changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_selection_changed(self, row_id, row_data):
        if row_data:
            user_name = row_data[1]
            self.viewer.set_detail_report_name(f'User account <{user_name}>')
