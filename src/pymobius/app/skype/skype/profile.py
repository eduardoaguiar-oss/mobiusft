# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import os
import os.path
import sqlite3
import tempfile
import traceback

import mobius
import pymobius
from . import decoder_accounts
from . import decoder_calls
from . import decoder_messages
from . import decoder_transfers


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get column names from table
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_table_columns(db, table):
    columns = set()

    if db:
        SQL_STATEMENT = f'PRAGMA TABLE_INFO ({table})'

        for row in db.execute(SQL_STATEMENT):
            columns.add(row[1])

    return columns


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get database schema version
# @param db Database object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_schema_version(db):
    version = None

    stmt = '''
       SELECT SQLiteSchemaVersion
         FROM AppSchemaVersion'''

    try:
        cursor = db.execute(stmt)
        row = cursor.fetchone()

        if row:
            version = int(row[0])

    except Exception as e:
        mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    return version


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Skype Profile class
# @author Eduardo Aguiar
# @see https://sqliteforensictoolkit.com/using-group_concat-to-amalgamate-the-results-of-queries/
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Profile(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, folder, item):
        self.__folder = folder
        self.__item = item
        self.__db = None
        self.__evidence_source = None

        # set profile attributes
        self.name = folder.name
        self.path = mobius.core.io.to_win_path(folder.path)
        self.folder = folder
        self.metadata = mobius.core.pod.map()

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
        if self.__accounts_loaded:
            return

        db = self.__get_db()
        if not db:
            return

        # get accounts
        self.__accounts = decoder_accounts.decode(db)

        # map account ID to account name
        for account in self.__accounts:
            account.evidence_source = self.__evidence_source
            self.__skype_names[account.id] = account.fullname

        # get user account ID
        if self.__accounts:
            self.__account_id = self.__accounts[0].id

        # set accounts loaded
        self.__accounts_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load calls
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_calls(self):
        if not self.__calls_loaded:
            db = self.__get_db()

            if db:
                self.__load_accounts()
                self.__calls = decoder_calls.decode(db, self.__account_id)

            self.__calls_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load chat messages
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_chat_messages(self):
        if not self.__chat_messages_loaded:
            db = self.__get_db()
            cols = get_table_columns(db, 'messages')

            if 'chatname' in cols:
                self.__chat_messages, self.__skype_names = decoder_messages.decode(db)

            self.__chat_messages_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load file transfers
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_file_transfers(self):
        if not self.__file_transfers_loaded:
            db = self.__get_db()
            cols = get_table_columns(db, 'transfers')

            if 'status' in cols:
                self.__load_accounts()
                self.__file_transfers = decoder_transfers.decode(db, self.__accounts[0])

            self.__file_transfers_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get database files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_database_files(self):
        files = []

        f = self.__folder.get_child_by_name('main.db')
        if f and not f.is_reallocated():
            db = self.__get_db()
            db_schema_version = get_schema_version(db)
            files.append(('main.db', db_schema_version, f))

        return files

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve sqlite database
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_db(self):

        if not self.__db:
            f = self.__folder.get_child_by_name('main.db')
            if not f:
                return

            reader = f.new_reader()
            if not reader:
                return

            # create temporary .sqlite local file
            ext = os.path.splitext(f.name)[1]
            fd, path = tempfile.mkstemp(suffix=ext)

            fp = open(path, 'wb')
            fp.write(reader.read())
            fp.close()

            # connect to db
            self.__db = sqlite3.connect(path)
            self.__evidence_source = pymobius.get_evidence_source_from_file(f)

            # set metadata
            self.metadata.set('main.db_path', mobius.core.io.to_win_path(f.path))
            db_schema_version = get_schema_version(self.__db)

            if db_schema_version:
                self.metadata.set('main.db_schema_version', db_schema_version)
                mobius.core.logf(f'INF main.db schema version={db_schema_version:d}')

            # f.set_handled ()

        return self.__db
