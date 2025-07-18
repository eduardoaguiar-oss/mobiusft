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
import pymobius

ANT_ID = 'user-accounts-from-cookies'
ANT_NAME = 'User Accounts from Cookies'
ANT_VERSION = '1.0'


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


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: User Accounts from Cookies
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
        self.__entries = []
        self.__retrieve_cookies()
        self.__save_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from cookies
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_cookies(self):
        for c in self.__item.get_evidences('cookie'):
            if not c.is_encrypted:
                self.__retrieve_cookie(c)

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
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        transaction = self.__item.new_transaction()

        # save evidences
        for e in self.__entries:
            account = self.__item.new_evidence('user-account')
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
