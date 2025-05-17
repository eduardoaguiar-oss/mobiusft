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
import mobius.core.turing
import pymobius


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Test passwords for hashes found
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.name = 'Turing Test Hash Passwords'
        self.version = '1.0'
        self.__item = item
        self.__turing = None
        self.__count = 0
        self.__password_count = 0
        self.__hashes = []
        self.__passwords = set()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__turing = mobius.core.turing.turing()

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
        mobius.core.logf(f'INF ant.turing: {self.__password_count:d}/{self.__count:d} password(s) found for hashes')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_hash</i>: Process hash object
    # @param ant Mediator ant object
    # @param h Hash object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_hash(self, ant, h):
        if h.password is None:
            p = self.__test_hash(h)

            if p:
                ant.add_password(p)

            else:
                self.__hashes.append(h)

            self.__count += 1

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Test hash passwords
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __test_hash(self, h):
        p = None

        if h.password is None:
            p = self.__test_database(h)

            if not p:
                p = self.__test_passwords(h)

        return p

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Test hash against turing database
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __test_database(self, h):
        p = None

        value = mobius.encoder.hexstring(h.value)
        status, password = self.__turing.get_hash_password(h.type, value)

        if status == 1:
            p = self.__create_password(h, password)
            self.__password_count += 1

        return p

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Test hash against collected passwords
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __test_passwords(self, h):
        passwords = set(self.__passwords)
        attrs = dict(h.metadata)

        for password in passwords:
            value = ''

            if h.type == 'nt':
                value = mobius.os.win.hash_nt(password)

            elif h.type == 'lm':
                value = mobius.os.win.hash_lm(password)

            elif h.type == 'msdcc1':
                value = mobius.os.win.hash_msdcc1(password, attrs['username'])

            elif h.type == 'msdcc2':
                value = mobius.os.win.hash_msdcc2(password, attrs['username'], int(attrs['iterations']))

            elif h.type == 'sha1.utf16':
                value = mobius.crypt.hash_digest('sha1', password.encode('utf-16-le'))

            if h.value == mobius.encoder.hexstring(value):
                self.__password_count += 1
                p = self.__create_password(h, password)
                return p

        return None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Create password object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __create_password(self, h, password):
        p = None
        h.password = password

        # don't add os.user sha1.utf16 passwords
        pwd_type = getattr(h, 'pwd_type', None)

        if pwd_type != 'os.user' or h.type != 'sha1.utf16':
            p = pymobius.Data()
            p.type = pwd_type or 'pwd.hash'
            p.value = password
            p.description = "Password for " + h.description

            p.metadata = h.metadata[:]
            p.metadata.append(('Hash type', h.type))
            p.metadata.append(('Hash value', mobius.encoder.hexstring(h.value)))

        return p

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_test_password</i>: Test password
    # @param ant Mediator ant object
    # @param h Hash object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_test_password(self, ant, password):
        self.__passwords.add(password)

        hashes = list(self.__hashes)

        for h in hashes:
            self.__test_hash(h)
