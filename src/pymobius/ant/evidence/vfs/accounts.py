# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import traceback

import mobius
import pymobius.ant.turing
import pymobius.app.skype

ANT_ID = 'accounts'
ANT_NAME = 'Accounts'
ANT_VERSION = '1.2'

EVIDENCE_TYPE = 'user-account'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: Accounts
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.id = ANT_ID
        self.name = ANT_NAME
        self.version = ANT_VERSION
        self.__item = item
        self.__entries = []

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        if not self.__item.has_datasource():
            return

        self.__entries = []
        self.__retrieve_passwords()
        self.__retrieve_skype()

        self.entries = self.__entries
        self.__save_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from passwords
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_passwords(self):
        try:
            # run ant.turing, if necessary
            if not self.__item.has_ant('turing'):
                ant = pymobius.ant.turing.Ant(self.__item)
                ant.run()

            # retrieve data
            for p in self.__item.get_evidences(pymobius.ant.turing.PASSWORD_EVIDENCE_TYPE):
                self.__retrieve_password(p)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from password
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_password(self, p):
        try:
            metadata = p.metadata.to_python()

            acc = pymobius.Data()
            acc.type = None
            acc.id = None
            acc.name = None
            acc.password = None
            acc.metadata = mobius.core.pod.map()

            if p.password_type.startswith('net.http'):
                acc.type = p.password_type
                acc.id = metadata.get("User ID")
                acc.password = p.value
                acc.metadata = p.metadata

            elif p.password_type == 'net.account':
                domain = p.metadata.get('Domain', '')

                if domain.startswith('WindowsLive:name='):
                    acc.type = 'web.live.com'
                    acc.id = metadata.get('Account')
                    acc.password = p.value
                    acc.metadata.set('credential-file-path', metadata.get('Credential file path'))

            elif p.password_type == 'os.user':
                acc.type = 'os.user'
                acc.id = metadata.get('username')
                acc.password = p.value
                acc.name = metadata.get('fullname') or metadata.get('admin_comment')

            # add account, if it is valid
            if acc.type and acc.id:
                acc.evidence_source = p.evidence_source
                self.__entries.append(acc)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Skype
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_skype(self):
        try:
            model = pymobius.app.skype.model(self.__item)

            for profile in model.get_profiles():
                self.__retrieve_skype_profile(profile)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Skype profile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_skype_profile(self, profile):
        try:
            for account in profile.get_accounts():
                entry = pymobius.Data()
                entry.type = 'app.skype'
                entry.id = account.id
                entry.name = account.fullname
                entry.evidence_source = account.evidence_source
                entry.password = None

                entry.metadata = mobius.core.pod.map()
                entry.metadata.set('username', profile.username)
                entry.metadata.set('app-id', 'skype')
                entry.metadata.set('app-name', 'Skype')
                entry.metadata.set('profile-path', profile.path)
                entry.metadata.set('fullname', account.fullname)
                entry.metadata.set('mood-text', account.mood_text)
                entry.metadata.set('city', account.city)
                entry.metadata.set('province', account.province)
                entry.metadata.set('country', account.country)
                entry.metadata.set('emails', ', '.join(account.emails))

                if account.birthday:
                    entry.metadata.set('birthday', account.birthday)

                if account.gender == 1:
                    entry.metadata.set('gender', 'male')

                elif account.gender == 2:
                    entry.metadata.set('gender', 'female')

                if account.type == 1:  # main.db based
                    self.__retrieve_skype_profile_v1(entry, account)

                elif account.type == 2:  # s4l.db based
                    self.__retrieve_skype_profile_v2(entry, account)

                self.__entries.append(entry)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Skype profile v1 (main.db based)
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_skype_profile_v1(self, entry, account):
        entry.metadata.set('homepage', account.homepage)
        entry.metadata.set('about', account.about)
        entry.metadata.set('languages', account.languages)
        entry.metadata.set('phone_home', account.phone_home)
        entry.metadata.set('phone_office', account.phone_office)
        entry.metadata.set('phone_mobile', account.phone_mobile)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Skype profile v2 (s4l.db based)
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_skype_profile_v2(self, entry, account):
        entry.metadata.set('phones', ' ,'.join(account.phones))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        transaction = self.__item.new_transaction()

        # save evidences
        for e in self.__entries:
            account = self.__item.new_evidence(EVIDENCE_TYPE)
            account.account_type = e.type
            account.id = e.id

            if e.name:
                account.name = e.name

            if e.password is not None:
                account.password = e.password
                account.password_found = 'yes'
            else:
                account.password_found = 'no'

            account.evidence_source = e.evidence_source
            account.metadata = e.metadata
            # account.avatars = acc.avatars

        # commit data
        transaction.commit()
