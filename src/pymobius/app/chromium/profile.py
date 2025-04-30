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
import json
import os
import os.path

import mobius
import pymobius.app.chromium
from . import decoder_bookmarks
from . import decoder_cookies
from . import decoder_history
from . import decoder_login_data
from . import decoder_web_data

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Known files list
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
KFF_FILES = [
    'Cookies',
    'History',
    'Login Data',
    'Web Data'
]


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Chrome Profile class
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Profile(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, folder):
        self.app_id = 'chrome'
        self.app_name = 'Google Chrome'
        self.creation_time = folder.creation_time
        self.__history_loaded = False
        self.__web_data_loaded = False
        self.__cookies_loaded = False
        self.__login_data_loaded = False
        self.__bookmarks_loaded = False
        self.__accounts = []
        self.__history = []
        self.__cookies = []
        self.__downloads = []
        self.__form_history = []
        self.__stored_passwords = []
        self.__bookmarks = []

        self.__local_folder = folder
        self.path = folder.path.replace('/', '\\')
        self.name = folder.name
        self.account_id = None
        self.email = None
        self.locale = None
        self.master_keys = {}

        # try to read Preferences file
        f = self.__local_folder.get_child_by_path('Preferences')
        self.__load_preferences(f)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Preferences file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_preferences(self, f):
        if not f or f.is_reallocated():
            return

        reader = f.new_reader()
        if not reader:
            return

        # decode Preferences file
        fp = mobius.io.text_reader(reader)
        j = json.load(fp)

        if not j or not isinstance(j, dict):
            return

        account_list = j.get('account_info')
        if not isinstance(account_list, list) or len(account_list) == 0:
            return

        # read __accounts
        for a in account_list:
            account = pymobius.Data()
            account.id = a.pop('account_id')
            account.locale = a.pop('locale')
            account.name = a.pop('given_name')
            account.fullname = a.pop('full_name')
            account.email = a.pop('email')
            account.evidence_source = pymobius.get_evidence_source_from_file(f)
            self.__accounts.append(account)

            for key, value in a.items():
                mobius.core.logf(f"DEV Preferences unused key:{key}={value}")

        # fill data from first account
        account = self.__accounts[0]
        self.name = account.fullname
        self.account_id = account.id
        self.email = account.email
        self.locale = account.locale

        f.set_handled()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get accounts
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_accounts(self):
        return self.__accounts

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get cookies
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_cookies(self):

        if not self.__cookies_loaded:
            self.__load_cookies()

        return self.__cookies

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve cookies
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_cookies(self):
        if self.__cookies_loaded:
            return

        self.__cookies = []

        f = self.__local_folder.get_child_by_path('Cookies')
        self.__cookies += decoder_cookies.decode(f)

        f = self.__local_folder.get_child_by_path('Network/Cookies')
        self.__cookies += decoder_cookies.decode(f)

        f = self.__local_folder.get_child_by_path('Extension Cookies')
        self.__cookies += decoder_cookies.decode(f)

        self.__cookies_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get bookmarks
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_bookmarks(self):

        if not self.__bookmarks_loaded:
            self.__load_bookmarks()

        return self.__bookmarks

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve bookmarks
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_bookmarks(self):
        if self.__bookmarks_loaded:
            return

        # retrieve bookmarks
        self.__bookmarks = []

        for child in self.__local_folder.get_children():
            child_name = child.name.lower()

            if child_name == 'bookmarks' and not child.is_reallocated():
                self.__bookmarks += decoder_bookmarks.decode(child)

            elif child_name == 'bookmarks.bak' and not child.is_reallocated():
                self.__bookmarks += decoder_bookmarks.decode(child)

        # set bookmarks loaded
        self.__bookmarks_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get download history from History file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_downloads(self):
        if not self.__history_loaded:
            self.__load_history()

        return self.__downloads

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get form history from Web Data file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_form_history(self):

        if not self.__web_data_loaded:
            self.__web_data_loaded = True

            f, path = self.__retrieve_file('Web Data')
            if path:
                data = decoder_web_data.decode(path)
                self.__form_history = data.form_history
                os.remove(path)

        return self.__form_history

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get history entries from History file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_history(self):
        if not self.__history_loaded:
            self.__load_history()

        return self.__history

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get stored passwords
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_stored_passwords(self):

        if not self.__login_data_loaded:
            self.__login_data_loaded = True

            f, path = self.__retrieve_file('Login Data')
            if path:
                data = decoder_login_data.decode(path, f)
                self.__stored_passwords = data.stored_passwords
                os.remove(path)

        return self.__stored_passwords

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get known files list
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_known_files(self):

        for kff_file in KFF_FILES:
            f = self.__local_folder.get_child_by_name(kff_file, False)

            if f and not f.is_reallocated():
                yield f

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get database files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_database_files(self):
        files = []

        f = self.__local_folder.get_child_by_path('Login Data')
        if f and not f.is_reallocated():
            db_schema_version = pymobius.app.chromium.get_db_schema_version(f)
            files.append(('Login Data', db_schema_version, f))

        f = self.__local_folder.get_child_by_path('History')
        if f and not f.is_reallocated():
            db_schema_version = pymobius.app.chromium.get_db_schema_version(f)
            files.append(('History', db_schema_version, f))

        f = self.__local_folder.get_child_by_path('Cookies')
        if f and not f.is_reallocated():
            db_schema_version = pymobius.app.chromium.get_db_schema_version(f)
            files.append(('Cookies', db_schema_version, f))

        f = self.__local_folder.get_child_by_path('Network/Cookies')
        if f and not f.is_reallocated():
            db_schema_version = pymobius.app.chromium.get_db_schema_version(f)
            files.append(('Cookies', db_schema_version, f))

        f = self.__local_folder.get_child_by_path('Web Data')
        if f and not f.is_reallocated():
            db_schema_version = pymobius.app.chromium.get_db_schema_version(f)
            files.append(('Web Data', db_schema_version, f))

        return files

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load data from History file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_history(self):

        # retrieve data
        f, path = self.__retrieve_file('History')

        if path:
            data = decoder_history.decode(path)

            self.__history = []

            for h in data.history:
                h.profile_path = self.path
                h.profile_name = self.name
                h.username = self.username
                self.__history.append(h)

            self.__downloads += data.downloads

            os.remove(path)

        # set history loaded
        self.__history_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve file into temp directory
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_file(self, rpath):
        f = self.__local_folder.get_child_by_path(rpath)

        return f, pymobius.app.chromium.retrieve_db_file(f)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get any file (development only)
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_file(self, rpath):

        # try to get file
        f = self.__local_folder.get_child_by_path(rpath)
        if not f:
            return

        reader = f.new_reader()
        if not reader:
            return

        # create temporary .sqlite local file
        fp = open(os.path.basename(rpath), 'wb')
        fp.write(reader.read())
        fp.close()
