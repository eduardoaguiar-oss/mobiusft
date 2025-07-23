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
import base64
import json
import os.path
import sqlite3
import tempfile
import traceback

import mobius
import pymobius.app.chromium.profile
import pymobius.operating_system

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Chromium based browsers
# Some of the rows below were copied from the Lazagne Project
# (see https://github.com/AlessandroZ/LaZagne)
# Columns: app_id, app_name, profiles path, multi-profile (True/False)
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
CHROMIUM_APPS = [
    ('1stBrowser', '1st Browser', '%localappdata%/1stBrowser/User Data', True),
    ('7Star', '7 Star', '%localappdata%/7Star/7Star/User Data', True),
    ('aliexpress', 'AliExpress', '%localappdata%/AliExpress/User Data', True),
    ('amigo', 'Amigo', '%localappdata%/Amigo/User Data', True),
    ('appkiwi', 'AppKiwi', '%localappdata%/AppKiwi/User Data', True),
    ('avast', 'Avast Browser', '%localappdata%/AVAST Software/Browser/User Data', True),
    ('bobrowser', 'BoBrowser', '%localappdata%/BoBrowser/User Data', True),
    ('brave', 'Brave', '%localappdata%/BraveSoftware/Brave-Browser/User Data', True),
    ('ccleaner', 'CentBrowser', '%localappdata%/CCleaner Browser/User Data', True),
    ('centbrowser', 'CentBrowser', '%localappdata%/CentBrowser/User Data', True),
    ('chedot', 'Chedot', '%localappdata%/Chedot/User Data', True),
    ('chrome.bcltmp', 'Google Chrome from Avast Browser Cleanup', '%localappdata%/Temp/BCLTMP/Chrome', True),
    ('chrome', 'Google Chrome', '%localappdata%/Google/Chrome/User Data', True),
    ('chrome canary', 'Chrome Canary', '%localappdata%/Google/Chrome SxS/User Data', True),
    ('chromium', 'Chromium', '%localappdata%/Chromium/User Data', True),
    ('ckaach', 'Ckaach', '%localappdata%/Ckaach', True),
    ('coccoc', 'Coccoc', '%localappdata%/CocCoc/Browser/User Data', True),
    ('comodo', 'Comodo Dragon', '%localappdata%/Comodo/Dragon/User Data', True),
    ('cryptotab', 'CryptoTab Browser', '%localappdata%/CryptoTab Browser/User Data', True),
    ('discord', 'Discord', '%appdata%/Discord', False),
    ('edge', 'Microsoft Edge', '%localappdata%/Microsoft/Edge/User Data', True),
    ('elements', 'Elements Browser', '%localappdata%/Elements Browser/User Data', True),
    ('epic', 'Epic Privacy Browser', '%localappdata%/Epic Privacy Browser/User Data', True),
    ('kodi.browser', 'Kodi Browser Launcher',
     '%appdata%/Kodi/userdata/addon_data/plugin.program.browser.launcher/profile/2', True),
    ('kodi.chrome', 'Kodi Chrome Launcher', '%appdata%/Kodi/userdata/addon_data/plugin.program.chrome.launcher/profile',
     True),
    ('kometa', 'Kometa', '%localappdata%/Kometa/User Data', True),
    ('opera', 'Opera', '%appdata%/Opera Software/Opera Stable', False),
    ('opera-gx', 'Opera GX', '%appdata%/Opera Software/Opera GX Stable', False),
    ('orbitum', 'Orbitum', '%localappdata%/Orbitum/User Data', True),
    ('plutotv', 'PlutoTV', '%localappdata%/PlutoTV', False),
    ('spotify', 'Spotify', '%localappdata%/Spotify/Browser', False),
    ('sputnik', 'Sputnik', '%localappdata%/Sputnik/Sputnik/User Data', True),
    ('torch', 'Torch', '%localappdata%/Torch/User Data', True),
    ('uran', 'Uran', '%localappdata%/uCozMedia/Uran/User Data', True),
    ('vivaldi', 'Vivaldi', '%localappdata%/Vivaldi/User Data', True)
]


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Chromium based model class
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class model(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.__item = item

        self.__profiles = []
        self.__profiles_loaded = False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get profiles
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_profiles(self):
        if not self.__profiles_loaded:
            self.__load_profiles()

        return self.__profiles

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load profiles from disk
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_profiles(self):

        # load profiles
        for opsys in pymobius.operating_system.scan(self.__item):
            for user_profile in opsys.get_profiles():

                for app_id, app_name, app_path, flag_multiprofile in CHROMIUM_APPS:
                    folder = user_profile.get_entry_by_path(app_path)

                    if folder:
                        local_state = self.__load_local_state(folder)

                        if flag_multiprofile:
                            self.__load_app_profiles(folder, app_id, app_name, user_profile.username, local_state)
                        else:
                            self.__load_app_profile(folder, app_id, app_name, user_profile.username, local_state)

        self.__profiles_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load profiles from app profile folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_app_profiles(self, folder, app_id, app_name, username, local_state):
        for child in folder.get_children():
            try:
                if child.is_folder() and child.get_child_by_name('History'):
                    self.__load_app_profile(child, app_id, app_name, username, local_state)
            except Exception as e:
                mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load profile from app profile folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_app_profile(self, folder, app_id, app_name, username, local_state):
        try:
            p = pymobius.app.chromium.profile.Profile(folder)
            p.app_id = app_id
            p.app_name = app_name
            p.username = username
            p.creation_time = folder.creation_time
            p.local_state = local_state
            p.local_state_path = local_state.get('path')

            self.__profiles.append(p)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load "Local State" file from "User Data" folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_local_state(self, folder):

        try:
            # try to find Local State file
            f = folder.get_child_by_path('Local State')

            if not f or f.is_reallocated():
                mobius.core.logf("WRN File 'Local State' not found. ")
                return {}

            # try to create a new reader
            reader = f.new_reader()
            if not reader:
                return {}

            # decode Local State json file
            fp = mobius.core.io.text_reader(reader)
            j = json.load(fp)

            if not j or not isinstance(j, dict):
                return {}

            local_state = j
            local_state['path'] = f.path

            # set v10_encrypted_key and v20_encrypted_key, if found
            os_crypt = local_state.get('os_crypt')

            if os_crypt:
                encrypted_key = os_crypt.get('encrypted_key')

                if encrypted_key:
                    v10_encrypted_key = base64.b64decode(encrypted_key)

                    if v10_encrypted_key.startswith(b'DPAPI'):
                        local_state['v10_encrypted_key'] = v10_encrypted_key
                        mobius.core.logf('INF v10_encrypted_key found:\n' + pymobius.dump(v10_encrypted_key))

                encrypted_key = os_crypt.get('app_bound_encrypted_key')

                if encrypted_key:
                    v20_encrypted_key = base64.b64decode(encrypted_key)

                    if v20_encrypted_key.startswith(b'APPB'):
                        local_state['v20_encrypted_key'] = v20_encrypted_key
                        mobius.core.logf('INF v20_encrypted_key found:\n' + pymobius.dump(v20_encrypted_key))

            # return data
            return local_state

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get database schema version
# @param db Database object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_meta_version(db):
    version = None

    stmt = '''
       SELECT value
         FROM meta
        WHERE key = "version"'''

    try:
        cursor = db.execute(stmt)
        row = cursor.fetchone()

        if row:
            version = int(row[0])

    except Exception as e:
        mobius.core.logf('WRN app.chromium: %s' % str(e))

    return version


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve file into temp directory
# @param f File
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve_db_file(f):
    if not f or f.is_reallocated():
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

    return path


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get database schema version
# @param f File object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_db_schema_version(f):
    version = None

    path = retrieve_db_file(f)
    if path:
        db = sqlite3.connect(path)
        if db:
            version = get_meta_version(db)
            db.close()

    return version


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Convert chromium timestamp to date/time
# @author Eduardo Aguiar
# @param timestamp Numerical value
# @return Date/time
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_datetime(timestamp):
    if timestamp in (None, 0, 86400000000):
        return None

    elif timestamp < 4294967296:
        return mobius.core.datetime.new_datetime_from_unix_timestamp(timestamp)

    elif timestamp < 10000000000000000:
        return mobius.core.datetime.new_datetime_from_unix_timestamp(int(timestamp / 1000000))

    else:
        return mobius.core.datetime.new_datetime_from_nt_timestamp(timestamp * 10)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Return select column statement, according to a given condition
# @param flag Condition
# @param value_yes What to add if flag is True
# @param value_no What to add if flag is False
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def add_column(flag, value_yes, value_no='NULL'):
    if flag:
        return ', ' + value_yes
    else:
        return ', ' + value_no
