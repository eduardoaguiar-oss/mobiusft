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
import copy

import mobius
import pymobius
from . import decoder_dht_dat
from . import decoder_resume_dat
from . import decoder_settings_dat
from . import decoder_torrent

APP_ID = 'utorrent'
APP_NAME = 'µTorrent'

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
STATE_NO = 0
STATE_YES = 1
STATE_ALWAYS = 2
STATE_UNKNOWN = -1


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief uTorrent Profile class
# @author Eduardo Aguiar
# @see https://www.forensicfocus.com/articles/forensic-analysis-of-the-%CE%BCtorrent-peer-to-peer-client-in-windows/
# @see https://robertpearsonblog.wordpress.com/2016/11/10/utorrent-forensic-artifacts/
# @see https://robertpearsonblog.wordpress.com/2016/11/11/utorrent-and-windows-10-forensic-nuggets-of-info/
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Profile(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, folder, item):
        self.__folder = folder
        self.__item = item

        # set profile attributes
        self.name = folder.name
        self.path = mobius.io.to_win_path(folder.path)
        self.folder = folder
        self.metadata = mobius.pod.map()
        self.username = None
        self.app_id = APP_ID
        self.app_name = APP_NAME

        # set data attributes
        self.__configuration_loaded = False
        self.__files_loaded = False
        self.__dht_loaded = False
        self.__configuration = None
        self.__downloaded_files = []
        self.__uploaded_files = []
        self.__shared_files = []
        self.__local_files = []
        self.__remote_files = []
        self.__ip_addresses = []
        self.__accounts = []

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get configuration
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_configuration(self):
        if not self.__configuration_loaded:
            self.__load_configuration()

        return self.__configuration

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get downloaded files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_downloaded_files(self):
        if not self.__files_loaded:
            self.__load_files()

        return self.__downloaded_files

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get uploaded files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_uploaded_files(self):
        if not self.__files_loaded:
            self.__load_files()

        return self.__uploaded_files

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get shared files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_shared_files(self):
        if not self.__files_loaded:
            self.__load_files()

        return self.__shared_files

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get local files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_local_files(self):
        if not self.__files_loaded:
            self.__load_files()

        return self.__local_files

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get remote files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_remote_files(self):
        if not self.__files_loaded:
            self.__load_files()

        return self.__remote_files

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get IP addresses
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ip_addresses(self):
        if not self.__dht_loaded:
            self.__load_dht_files()

        return self.__ip_addresses

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get accounts
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_accounts(self):
        if not self.__dht_loaded:
            self.__load_dht_files()

        return self.__accounts

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load configuration
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_configuration(self):

        # settings.dat
        f = self.folder.get_child_by_path('settings.dat', False)

        if f and not f.is_reallocated():
            self.__configuration = decoder_settings_dat.decode(f)

        # settings.dat.old
        if not self.__configuration:
            f = self.folder.get_child_by_path('settings.dat.old', False)

            if f and not f.is_reallocated():
                self.__configuration = decoder_settings_dat.decode(f)

        self.__configuration_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load files data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_files(self):
        entries = {}

        # read resume.dat.old
        f = self.folder.get_child_by_path('resume.dat.old', False)
        self.__load_resume_dat(f, entries)

        # read resume.dat
        f = self.folder.get_child_by_path('resume.dat', False)
        self.__load_resume_dat(f, entries)

        # read torrent files
        files = []

        for f in self.folder.get_children():
            if not f.is_reallocated() and f.name.endswith('.torrent'):
                files += decoder_torrent.decode(f) or []

        # complete files info with resume.dat entries info
        for f in files:
            f.username = self.username
            f.profile_path = self.path
            f.app_id = APP_ID
            f.app_name = APP_NAME

            entry = entries.get(f.torrent_name)

            if entry:
                f.is_completed = STATE_YES if entry.completed_time else STATE_NO
                f.flag_shared = STATE_YES if entry.seeded_seconds > 0 else STATE_NO
                f.flag_downloaded = STATE_YES if entry.bytes_downloaded > 0 else STATE_NO
                f.flag_uploaded = STATE_YES if entry.bytes_uploaded > 0 else STATE_NO
                f.added_time = entry.added_time
                f.completed_time = entry.completed_time
                f.metadata_time = entry.metadata_time
                f.last_seen_complete_time = entry.last_seen_complete_time
                f.download_url = entry.download_url
                f.caption = entry.caption
                f.peers = entry.peers
                f.metadata = entry.metadata
                f.resume_dat_path = entry.resume_dat_path
                f.resume_dat_modification_time = entry.resume_dat_modification_time

                if f.path:
                    f.path = entry.path + '\\' + f.path
                else:
                    f.path = entry.path

            else:
                f.is_completed = STATE_UNKNOWN
                f.flag_shared = STATE_UNKNOWN
                f.flag_downloaded = STATE_UNKNOWN
                f.flag_uploaded = STATE_UNKNOWN
                f.added_time = None
                f.completed_time = None
                f.metadata_time = None
                f.last_seen_complete_time = None
                f.download_url = None
                f.caption = None
                f.peers = []
                f.metadata = {}
                f.resume_dat_path = None
                f.resume_dat_modification_time = None
                f.path = f.path or f.name

        # files
        self.__downloaded_files = [f for f in files if f.flag_downloaded == STATE_YES]
        self.__uploaded_files = [f for f in files if f.flag_uploaded == STATE_YES]
        self.__shared_files = [f for f in files if f.flag_shared == STATE_YES]
        self.__local_files = list(files)

        # remote files
        self.__remote_files = []

        for f in files:
            for ip, port in f.peers:
                rf = copy.copy(f)
                rf.ip = ip
                rf.port = port
                rf.timestamp = f.metadata_time
                rf.hashes = []
                rf.app_id = APP_ID
                rf.app_name = APP_NAME
                self.__remote_files.append(rf)

        # set files loaded
        self.__files_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load files data from resume.dat file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_resume_dat(self, f, entries):
        if not f or f.is_reallocated():
            return

        resume_dat_entries = decoder_resume_dat.decode(f)
        if not resume_dat_entries:
            return

        # add file if it is either new or newer than existing file
        for e in resume_dat_entries:
            e_old = entries.get(e.name)

            if not e_old or e_old.metadata_time < e.metadata_time:
                entries[e.name] = e

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load dht.dat
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_dht_files(self):

        # dht.dat.old
        f = self.folder.get_child_by_path('dht.dat.old', False)
        self.__load_dht_dat(f)

        # dht.dat
        f = self.folder.get_child_by_path('dht.dat', False)
        self.__load_dht_dat(f)

        self.__dht_loaded = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Load dht.dat
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_dht_dat(self, f):

        # check if file is valid
        if not f or f.is_reallocated():
            return

        data = decoder_dht_dat.decode(f)
        if not data:
            return

        # add ip address if it is different
        if data.ip and data.timestamp and all(
                a.ip != data.ip or a.timestamp != data.timestamp for a in self.__ip_addresses):
            ip_data = pymobius.Data()
            ip_data.ip = data.ip
            ip_data.timestamp = data.timestamp
            ip_data.app_id = APP_ID
            ip_data.app_name = APP_NAME
            ip_data.dht_dat_path = mobius.io.to_win_path(f.path)
            ip_data.username = self.username
            ip_data.evidence_source = data.evidence_source
            self.__ip_addresses.append(ip_data)

        # add account if it is different
        if data.guid and all(a.guid != data.guid for a in self.__accounts):
            account = pymobius.Data()
            account.guid = data.guid
            account.app_id = APP_ID
            account.app_name = APP_NAME
            account.timestamp = data.timestamp
            account.ip = data.ip
            account.username = self.username
            account.evidence_source = pymobius.get_evidence_source_from_file(f)
            self.__accounts.append(account)
