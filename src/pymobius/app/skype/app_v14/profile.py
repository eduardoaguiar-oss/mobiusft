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
import os
import os.path
import re
import sqlite3
import tempfile

import mobius
import pymobius
from . import decoder_calls
from . import decoder_messages
from . import decoder_profilecachev8


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode %UUUU chars from string
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pct_decode(s):
    def repl(matchobj):
        return chr(int(matchobj.group(1), 16))

    s = re.sub('%([0-9a-f]{4})', repl, s)

    return s


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get database schema version
# @param db Database object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_schema_version(db):
    version = None

    stmt = '''
       SELECT value
         FROM metadata
        WHERE name  = 'schemaVersion'
        '''

    try:
        cursor = db.execute(stmt)
        row = cursor.fetchone()

        if row:
            version = int(row[0])

    except Exception as e:
        mobius.core.logf('WRN ' + str(e))

    return version


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Skype Profile class
# @author Eduardo Aguiar
# @see https://bebinary4n6.blogspot.com/2019/07/
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Profile(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, f, item):
        self.__db = None
        self.__item = item
        self.__schema_version = None
        self.__f = f
        self.__evidence_source = pymobius.get_evidence_source_from_file(f)

        # set profile attributes
        self.name = pct_decode(f.name[4:-3])
        self.path = mobius.core.io.to_win_path(f.path)
        self.metadata = mobius.core.pod.map()
        self.metadata.set('db_path', self.path)

        # set data attributes
        self.__accounts_loaded = False
        self.__calls_loaded = False
        self.__chat_messages_loaded = False
        self.__file_transfers_loaded = False

        self.__accounts = []
        self.__calls = []
        self.__chat_messages = []
        self.__file_transfers = []
        self.__skype_names = {}
        self.__conversation_members = {}

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get accounts
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_accounts(self):
        if not self.__accounts_loaded:
            self.__load_accounts()

        return self.__accounts

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get calls
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_calls(self):
        if not self.__calls_loaded:
            self.__load_calls()

        return self.__calls

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get chat messages
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_chat_messages(self):
        if not self.__chat_messages_loaded:
            self.__load_chat_messages()

        return self.__chat_messages

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get file transfers
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_file_transfers(self):
        if not self.__file_transfers_loaded:
            self.__load_file_transfers()

        return self.__file_transfers

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load accounts
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_accounts(self):
        if not self.__accounts_loaded:

            # get data from profilecachev8 table
            db = self.__get_db()
            profiles, skype_names = decoder_profilecachev8.decode(db, self.name)

            # update names
            self.__skype_names.update(skype_names)

            # add accounts
            for account in profiles:
                if p.id == self.name:
                    account.evidence_source = self.__evidence_source
                    self.__accounts.append(account)

            # set data loaded
            self.__accounts_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load calls
    # @todo Implement
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_calls(self):
        if not self.__calls_loaded:
            db = self.__get_db()
            self.__calls, skype_names = decoder_calls.decode(db, self.name)
            self.__skype_names.update(skype_names)
            self.__calls_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load file transfers
    # @todo Implement
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_file_transfers(self):
        self.__file_transfers_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load chat messages
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_chat_messages(self):

        if not self.__chat_messages_loaded:
            db = self.__get_db()

            self.__chat_messages, skype_names = decoder_messages.decode(db, self.name)
            self.__skype_names.update(skype_names)

            self.__chat_messages_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get database files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_database_files(self):
        files = []

        if self.__f and not self.__f.is_reallocated():
            db = self.__get_db()
            db_schema_version = get_schema_version(db)
            files.append(('s4l.db', db_schema_version, self.__f))

        return files

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve database file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_db(self):

        if not self.__db:
            reader = self.__f.new_reader()
            if not reader:
                return

            # create temporary .sqlite local file
            ext = os.path.splitext(self.__f.name)[1]
            fd, path = tempfile.mkstemp(suffix=ext)

            fp = open(path, 'wb')
            fp.write(reader.read())
            fp.close()

            # connect to db
            self.__db = sqlite3.connect(path)
            db_schema_version = get_schema_version(self.__db)
            self.metadata.set('db_path', mobius.core.io.to_win_path(self.__f.path))

            if db_schema_version:
                self.metadata.set('db_schema_version', db_schema_version)
                mobius.core.logf('INF s4l.db schema version=%d' % db_schema_version)

            # set handled
            # self.__f.set_handled ()

        return self.__db
