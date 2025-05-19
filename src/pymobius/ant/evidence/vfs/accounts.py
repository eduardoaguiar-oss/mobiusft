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
import pymobius.app.chromium
import pymobius.app.skype

from . import cookies

ANT_ID = 'accounts'
ANT_NAME = 'Accounts'
ANT_VERSION = '1.1'

EVIDENCE_TYPE = 'user-account'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode login.live.com MSSPre cookie
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def func_login_live_com(c, data):
    c_data = c.value.decode('utf-8').split('|')
    data.id = c_data[0]
    data.metadata.set('mspcid', c_data[1])


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode generic cookie
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def func_value(c, data):
    data.id = c.value.decode('utf-8')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode slideplayer.com.br login cookie
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def func_slideplayer_com_br(c, data):
    c_data = c.value.decode('utf-8').split('+')
    data.id = c_data[0]
    data.name = c_data[1]
    data.password = c_data[2]


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode hotmart.com __kdtc cookie
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def func_hotmart_com(c, data):
    # s = 'cid%3Dswdsil%40yahoo.com.br%3Bt%3D1569186164243'
    u = mobius.core.io.uri(c.value.decode('ascii'))
    text = u.get_path('utf-8')

    if text.startswith('cid='):
        pos = text.find(';')

        if pos != -1:
            data.id = text[4:pos]


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Cookies by domain and cookie name
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
COOKIES = {
    ('hotmart.com', '__kdtc'): 'hotmart_com',
    ('imvu.com', '_imvu_avnm'): 'value',
    ('login.live.com', 'MSPPre'): 'login_live_com',
    ('pof.com', 'usernameb'): 'value',
    ('pof.com.br', 'usernameb'): 'value',
    ('slideplayer.com.br', 'login'): 'slideplayer_com_br',
}

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Cookie decoders
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
COOKIE_DECODERS = {
    'hotmart_com': func_hotmart_com,
    'login_live_com': func_login_live_com,
    'slideplayer_com_br': func_slideplayer_com_br,
    'value': func_value
}


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
        self.__retrieve_chromium()
        self.__retrieve_cookies()
        self.__retrieve_passwords()
        self.__retrieve_skype()

        self.entries = self.__entries
        self.__save_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Chromium based browsers
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_chromium(self):
        try:
            model = pymobius.app.chromium.model(self.__item)

            for profile in model.get_profiles():
                self.__retrieve_chromium_profile(profile)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Chromium profile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_chromium_profile(self, profile):
        try:
            for account in profile.get_accounts():
                entry = pymobius.Data()
                entry.type = f'app.{profile.app_id}'
                entry.id = account.id
                entry.name = account.name
                entry.password = None
                entry.metadata = mobius.core.pod.map()
                entry.evidence_source = account.evidence_source
                entry.metadata.set('fullname', account.fullname)
                entry.metadata.set('locale', account.locale)
                entry.metadata.set('email', account.email)
                entry.metadata.set('profile_path', profile.path)
                entry.metadata.set('username', profile.username)
                entry.metadata.set('app_id', profile.app_id)
                entry.metadata.set('app_name', profile.app_name)
                self.__entries.append(entry)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from cookies
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_cookies(self):
        try:

            # run ant.cookies, if necessary
            if not self.__item.has_ant('cookies'):
                ant = cookies.Ant(self.__item)
                ant.run()

            # retrieve data
            for c in self.__item.get_evidences('cookie'):
                if not c.is_encrypted:
                    self.__retrieve_cookie(c)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from cookie
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_cookie(self, c):
        try:
            # get cookie decoder
            decoder_name = COOKIES.get((c.domain, c.name))
            if not decoder_name:
                return

            # get decoder function
            decoder = COOKIE_DECODERS.get(decoder_name)
            if not decoder:
                mobius.core.logf(f'WRN Decoder "{decoder_name}" not found')
                return

            # run decoder function
            acc = pymobius.Data()
            acc.id = None
            acc.name = None
            acc.password = None
            acc.metadata = mobius.core.pod.map()

            decoder(c, acc)
            if not acc.id:
                return

            # create account
            acc.type = f'web.{c.domain}'
            acc.evidence_source = f"Cookie '{c.name}'. Created in {c.creation_time}"
            acc.metadata.set('cookie-creation-time', c.creation_time)
            acc.metadata.set('cookie-domain', c.domain)
            acc.metadata.set('cookie-name', c.name)
            acc.metadata.set('cookie-evidence-source', c.evidence_source)
            acc.metadata.update(c.metadata)

            self.__entries.append(acc)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

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
