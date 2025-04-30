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
import datetime
import os.path
import re
import traceback

import mobius
import pymobius
from . import decoder_cookies
from . import decoder_index_dat


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
RE_MAIN_HISTORY = re.compile('Visited: (.*?)@(.*)')
RE_DATE_HISTORY = re.compile(':([0-9]+?): (.*?)@(.*)')
HISTORY_FOLDER = 'AppData/Local/Microsoft/Windows/History'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Internet Explorer Profile class
# @author Eduardo Aguiar
# @see https://kb.digital-detective.net/display/BF/Internet+Explorer
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Profile(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # @todo check <profile>/Roaming/Microsoft/Windows/Cookies
    # @todo check <profile>/Roaming/Microsoft/Windows/History
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item, user_profile):
        self.app_id = 'internet_explorer'
        self.app_name = 'Internet Explorer'
        self.path = user_profile.path.replace('/', '\\')
        self.username = user_profile.username
        self.creation_time = user_profile.folder.creation_time
        self.name = 'Default'

        self.__item = item
        self.__user_profile = user_profile

        self.__cookies = []
        self.__history = []
        self.__downloads = []
        self.__favorites = []

        self.__cookies_loaded = False
        self.__history_loaded = False
        self.__downloads_loaded = False
        self.__favorites_loaded = False

        self.__cookies_folder = user_profile.get_control_folder('cookies', 'Cookies')
        self.__history_folder = user_profile.get_control_folder('history', HISTORY_FOLDER)
        self.__downloads_folder = user_profile.get_appdata_folder('Microsoft/Windows/IEDownloadHistory')
        self.__favorites_folder = user_profile.get_entry_by_path('%favorites%')

        if self.__cookies_folder or self.__history_folder or self.__downloads_folder or self.__favorites_folder:
            self.is_valid = True
        else:
            self.is_valid = False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get history
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_history(self):
        if not self.__history_loaded:
            self.__load_history()

        return self.__history

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get cookies
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_cookies(self):
        if not self.__cookies_loaded:
            self.__load_cookies()

        return self.__cookies

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get form history
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_form_history(self):
        return []

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get downloads
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_downloads(self):
        if not self.__downloads_loaded:
            self.__load_downloads()

        return self.__downloads

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get favorites
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_favorites(self):
        if not self.__favorites_loaded:
            self.__load_favorites()

        return self.__favorites

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve URL history from history files
    # Daily history files store timestamp both in primary_time (UTC) and
    # secondary_time (local time).
    # Weekly history files stores timestamp in secondary_time as local time
    # This function tries to update weekly timestamp to UTC.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_history(self):
        self.__history = []

        if self.__history_folder:
            self.__history = self.__load_history_folder(self.__history_folder)

        # Get timezone info per day, using history daily files and history main files
        # @see http://www.stevebunting.org/udpd4n6/forensics/index_dat2.htm
        tz = {}
        for entry in self.__history:
            if entry.file_type == 'daily':
                tz[entry.secondary_time.date()] = (entry.primary_time - entry.secondary_time).seconds

            elif entry.file_type == 'main':
                local_time = entry.tags.get(0x18)
                if local_time:
                    tz[local_time.date()] = (entry.primary_time - local_time).seconds

        # Set timestamp according to history file type
        for entry in self.__history:
            if entry.file_type == 'weekly':
                d = entry.timestamp.date()
                s = tz.get(d)

                if s == None:
                    entry.timestamp_utc = False
                else:
                    entry.timestamp -= datetime.timedelta(seconds=s)
                    entry.timestamp_utc = True

            else:
                entry.timestamp_utc = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Scan history folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_history_folder(self, folder):
        self.__history_loaded = True
        history = []

        for child in folder.get_children():
            if child.is_reallocated():
                pass

            elif child.is_folder():
                history += self.__load_history_folder(child)

            else:
                history += self.__load_history_from_file(child)

        return history

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse files inside history folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_history_from_file(self, f):
        history = []
        lname = f.name.lower()

        if lname == 'index.dat':
            history = self.__load_history_from_index_dat(f)

        elif lname == 'desktop.ini':  # @todo check desktop.ini (is valid?)
            f.set_handled()

        else:
            mobius.core.logf('app.internet_explorer.profile: Unhandled file ' + f.path)

        return history

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse index.dat history files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_history_from_index_dat(self, f):

        # decode file
        history = []
        decoder = decoder_index_dat.decoder(f.new_reader())

        if not decoder.is_valid:
            return history

        # get file type according to folder name
        folder_name = os.path.basename(os.path.dirname(f.path))

        if folder_name.lower().startswith('mshist'):
            y1 = int(folder_name[8:12])
            m1 = int(folder_name[12:14])
            d1 = int(folder_name[14:16])
            y2 = int(folder_name[16:20])
            m2 = int(folder_name[20:22])
            d2 = int(folder_name[22:24])
            td = datetime.date(y2, m2, d2) - datetime.date(y1, m1, d1)

            if td.days == 1:
                file_type = 'daily'

            elif td.days == 7:
                file_type = 'weekly'

            else:
                mobius.core.logf('WRN Ill-formed MSHist folder name (%s)' % folder_name)

        else:
            file_type = 'main'

        # format records according to location field
        # @see http://www.stevebunting.org/udpd4n6/forensics/index_dat2.htm
        records = [r for r in decoder.get_records() if r.type == 'URL']

        for r in records:

            # timestamp
            if file_type in ('main', 'daily'):
                r.timestamp = r.primary_time

            else:
                r.timestamp = r.secondary_time

            # parse location
            match = RE_MAIN_HISTORY.match(r.location)

            if match:
                r.username = match.group(1)
                r.url = match.group(2)

            else:
                match = RE_DATE_HISTORY.match(r.location)

                if match:
                    r.url = match.group(3)
                    r.username = match.group(2)

                else:
                    r.url = r.location
                    r.username = None

            # metadata
            r.index_dat_path = f.path.replace('/', '\\')
            r.index_dat_creation_time = f.creation_time
            r.index_dat_last_modification_time = f.modification_time
            r.file_type = file_type
            r.title = r.tags.get(0x10)

            #print '    ', r.location
            #print '    ', r.primary_time
            #print '    ', r.secondary_time
            #for tag_id, tag_value in sorted (r.tags.items ()):
            #  print '      %02x: %s' % (tag_id, tag_value)

            history.append(r)

        # set file handled
        f.set_handled()

        return history

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve cookies from cookies files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_cookies(self):
        self.__cookies_loaded = True
        self.__cookies = []

        if self.__cookies_folder:
            self.__cookies = self.__load_cookies_folder(self.__cookies_folder)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Scan cookies folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_cookies_folder(self, folder):
        cookies = []

        for child in folder.get_children():
            if child.is_reallocated():
                pass

            elif child.is_folder():
                cookies += self.__load_cookies_folder(child)

            else:
                cookies += self.__load_cookies_from_file(child)

        return cookies

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse files inside cookies folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_cookies_from_file(self, f):
        cookies = []
        lname = f.name.lower()

        if lname.endswith('.txt') or lname.endswith('.cookie'):
            c = decoder_cookies.decode(f)

            if c != None:
                cookies += c
                f.set_handled()

        elif lname == 'index.dat':
            decoder = decoder_index_dat.decoder(f.new_reader())

            if decoder.is_valid:
                f.set_handled()

        else:
            mobius.core.logf('app.internet_explorer: Unhandled file ' + f.path)

        return cookies

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve download history
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_downloads(self):
        self.__downloads_loaded = True
        self.__downloads = []

        if self.__downloads_folder:
            self.__downloads = self.__load_downloads_folder(self.__downloads_folder)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Scan IEDownloadHistory folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_downloads_folder(self, folder):
        downloads = []

        for child in folder.get_children():

            if child.is_reallocated():
                pass

            elif child.is_folder():
                downloads += self.__load_downloads_folder(child)

            else:
                downloads += self.__load_downloads_from_file(child)

        return downloads

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse files inside IEDownloadHistory folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_downloads_from_file(self, f):
        downloads = []
        lname = f.name.lower()

        if lname == 'index.dat':
            mobius.core.logf('DEV WANTED: ' + f.path)
            decoder = decoder_index_dat.decoder(f.new_reader())

            if decoder.is_valid:
                for r in decoder.get_records():
                    mobius.core.logf('DEV r.location=' + r.location)
                    mobius.core.logf('DEV r.tags=' + ', '.join(r.tags))

                f.set_handled()

        else:
            mobius.core.logf('app.internet_explorer: Unhandled file ' + f.path)

        return downloads

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve favorites
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_favorites(self):
        self.__favorites = []

        if self.__favorites_folder:
            self.__load_favorites_folder(self.__favorites_folder)

        self.__favorites_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve favorites from folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_favorites_folder(self, folder, name=''):

        try:
            for child in folder.get_children():
                if child.is_reallocated():
                    pass

                elif child.is_folder():
                    self.__load_favorites_folder(child, name + '/' + child.name if name else child.name)

                elif child.name.endswith('.url'):
                    self.__load_favorites_from_file(child, name)
        except Exception as e:
            mobius.core.logf(
                'WRN app.internet_explorer (__load_favorites_folder): ' + str(e) + ' ' + traceback.format_exc())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve favorites from file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_favorites_from_file(self, f, folder_name):
        reader = f.new_reader()

        if not reader:
            return

        fp = mobius.io.line_reader(reader, 'utf-8', '\r\n')
        url = None

        for line in fp:
            line = line.strip()

            if line.startswith('URL='):
                url = line[4:]

        if url:
            favorite = pymobius.Data()
            favorite.name = f.name[:-4]
            favorite.path = f.path
            favorite.folder = folder_name
            favorite.creation_time = f.creation_time
            favorite.last_modification_time = f.modification_time
            favorite.url = url
            favorite.evidence_source = pymobius.get_evidence_source_from_file(f)

            self.__favorites.append(favorite)
            f.set_handled()
