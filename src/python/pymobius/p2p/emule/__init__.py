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
import traceback
import urllib.parse

import mobius
import pymobius
import pymobius.operating_system
import pymobius.p2p
import pymobius.p2p.application
import pymobius.registry.installed_programs
from pymobius.p2p.local_file import *

from . import CTag
from . import decoder_ac_searchstrings_dat
from . import decoder_cancelled_met
from . import decoder_ip_to_country
from . import decoder_key_index_dat
from . import decoder_known_met
from . import decoder_part_met
from . import decoder_part_met_txtsrc
from . import decoder_preferences_dat
from . import decoder_preferences_ini
from . import decoder_preferenceskad_dat
from . import decoder_stored_searches_met


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Versions examined: Emule 0.50a and DreaMule 3.2
#
# Emule main forensic files:
# @see http://www.emule-project.net/home/perl/help.cgi?l=1&rm=show_topic&topic_id=106
#
# . AC_SearchStrings.dat: Stores search strings
#
# . Cancelled.met: Files cancelled before completing downloading
#
# . Clients.met: Credit control file. Control credits of each peer (dl and ul)
#
# . Downloads.txt/bak: Summary of .part.met files (part name and url)
#
# . Known.met: Shared files, downloading files, downloaded files
#
# . Preferences.dat: Stores the user GUID for eDonkey network
#
# . PreferencesKad.dat: Stores Kademlia network client ID
#
# . Sharedir.dat: Stores the paths to all shared directories
#
# . StoredSearches.met: Stores open searches (ongoing searches)
#
# . *.part.met: information about a file being downloaded (not in known.met)
#
# DreaMule forensic files:
# . *.part.met.txtsrc: list of sources, with IP and expiration date/time
#
# Kademlia forensic files:
# . key_index.dat: stores a chunk of Kademlia's Distributed Hash Table,
#   including search hash value, hits (files) and peers sharing those files
#
# All Date/Times are stored in Coordinated Universal Time (UTC).
# @see https://msdn.microsoft.com/pt-br/library/windows/desktop/ms724397(v=vs.85).aspx
#
# According to eMule Homepage: "Your Incoming and Temporary directory are always shared"
# @see https://www.emule-project.net/home/perl/help.cgi?l=1&topic_id=112&rm=show_topic
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieves Emule P2P activity data
# @author Eduardo Aguiar
# This function is planned to run in an independent thread. The idea here
# is to gather all activity data and only then write data to the model
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve(model):
    try:
        ant = Ant(model)
        ant.run()
    except Exception as e:
        mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief forensics: P2P Emule
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, model):
        self.__model = model
        self.__item = model.item
        self.__data = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief run
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):

        # create local data model
        self.__data = pymobius.Data()
        self.__data.accounts = []
        self.__data.searches = []
        self.__data.local_files = []
        self.__data.remote_files = []

        # create app
        self.__data.application = pymobius.p2p.application.application()
        self.__data.application.id = 'emule'
        self.__data.application.name = 'Emule'

        # retrieve data
        self.__retrieve_app_data()
        self.__normalize_data()

        # update P2P model
        self.__model.applications.append(self.__data.application)
        self.__model.accounts += self.__data.accounts
        self.__model.searches += self.__data.searches
        self.__model.local_files += self.__data.local_files
        self.__model.remote_files += self.__data.remote_files

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief retrieve data from disk files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_app_data(self):
        self.__ip_table = None

        # get item datasource
        datasource = self.__item.get_datasource()

        if not datasource:
            raise Exception("Datasource is not set")

        elif not datasource.is_available():
            raise Exception("Datasource is not available")

        elif datasource.get_type() != 'vfs':
            raise Exception("Datasource type is not VFS")

        vfs = datasource.get_vfs()

        # decode "<USER_FOLDER>/"
        path_list = []

        for opsys in pymobius.operating_system.scan(self.__item):
            for user_profile in opsys.get_profiles():
                try:
                    username = user_profile.username
                    self.__incoming_dir = None
                    self.__temp_dir = None

                    # AppData/Local
                    folder = user_profile.get_entry_by_path('%localappdata%/eMule')
                    if folder:
                        self.__retrieve_appdatalocal(username, folder)

                    # Incoming dir
                    if self.__incoming_dir:
                        path_list.append((username, self.__incoming_dir[3:].replace('\\', '/')))

                    else:
                        folder = user_profile.get_entry_by_path('%downloads%/eMule/Incoming')
                        if folder:
                            path_list.append((username, folder.path[3:].replace('\\', '/')))

                    # Temp dir
                    if self.__temp_dir:
                        path_list.append((username, self.__temp_dir[3:].replace('\\', '/')))

                    else:
                        folder = user_profile.get_entry_by_path('%downloads%/eMule/Temp')
                        if folder:
                            path_list.append((username, folder.path[3:].replace('\\', '/')))

                except Exception as e:
                    mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

        # decode root folder
        for entry in vfs.get_root_entries():
            if entry.is_folder():

                # /Program Files/Emule/Config"
                folder = entry.get_child_by_path('Program Files/Emule/Config', False)
                if folder:
                    self.__retrieve_program_files_folder(folder)

                # .part folder
                for username, path in path_list:
                    folder = entry.get_child_by_path(path, False)

                    if folder:
                        self.__retrieve_part_control_files(folder, username)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve program files folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_program_files_folder(self, folder):

        for f in folder.get_children():
            fname = f.name.lower()

            # Ignore reallocated entries
            if f.is_reallocated():
                pass

            # ip-to-country.csv
            elif fname == 'ip-to-country.csv':
                self.__ip_table = decoder_ip_to_country.retrieve(f)
                pymobius.p2p.set_handled(self.__item, f)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from APPDATALOCAL folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_appdatalocal(self, username, folder):

        # config folder
        config_folder = folder.get_child_by_name('config', False)
        if config_folder and config_folder.is_folder():
            # retrieve 'preferences.ini' first
            f = config_folder.get_child_by_name('preferences.ini', False)
            self.__retrieve_preferences_ini(self.__item, self.__data, f, username)

            # retrieve other files
            self.__retrieve_config_files(config_folder, username)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from config files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_config_files(self, folder, username):

        for f in folder.get_children():
            try:
                self.__retrieve_config_file(f, username)

            except Exception as e:
                mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from config file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_config_file(self, f, username):
        fname = f.name.lower()

        # Ignore reallocated entries
        if f.is_reallocated():
            pass

        # AC_SearchStrings.dat
        elif fname == 'ac_searchstrings.dat':
            self.__retrieve_ac_searchstrings_dat(self.__item, self.__data, f, username)

        # AC_SearchMetURLs.dat
        elif fname == 'ac_servermeturls.dat':
            pymobius.p2p.set_handled(self.__item, f)

        # Cancelled.met
        elif fname == 'cancelled.met':
            self.__retrieve_cancelled_met(self.__item, self.__data, f, username)

        # Clients.met
        elif fname == 'clients.met':
            pymobius.p2p.set_handled(self.__item, f)

        # downloads.bak
        elif fname == 'downloads.bak':
            pymobius.p2p.set_handled(self.__item, f)

        # downloads.txt
        elif fname == 'downloads.txt':
            pymobius.p2p.set_handled(self.__item, f)

        # ip-to-country.csv
        elif fname == 'ip-to-country.csv':
            self.__ip_table = decoder_ip_to_country.retrieve(f)
            pymobius.p2p.set_handled(self.__item, f)

        # key_index.dat
        elif fname == 'key_index.dat':
            self.__retrieve_key_index_dat(self.__item, self.__data, f, username)

        # Known.met
        elif fname == 'known.met':
            self.__retrieve_known_met(self.__item, self.__data, f, username)

        # Known2_64.met
        elif fname == 'known2_64.met':
            pymobius.p2p.set_handled(self.__item, f)

        # load_index.dat
        elif fname == 'load_index.dat':
            pymobius.p2p.set_handled(self.__item, f)

        # Nodes.dat
        elif fname == 'nodes.dat':
            pymobius.p2p.set_handled(self.__item, f)

        # Preferences.dat
        elif fname == 'preferences.dat':
            self.__retrieve_preferences_dat(self.__item, self.__data, f, username)

        # Preferences.ini
        elif fname == 'preferences.ini':
            pass  # already handled

        # PreferencesKad.dat
        elif fname == 'preferenceskad.dat':
            self.__retrieve_preferenceskad_dat(self.__item, self.__data, f, username)

        # src_index.dat
        elif fname == 'src_index.dat':
            pymobius.p2p.set_handled(self.__item, f)

        # StoredSearches.met
        elif fname == 'storedsearches.met':
            self.__retrieve_stored_searches_met(self.__item, self.__data, f, username)

        else:
            t = 'file' if f.is_file() else 'folder'
            mobius.core.logf(f"p2p.emule: Unhandled Config/{f.name} {t}")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from .part.* control files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_part_control_files(self, folder, username):
        if not folder or not folder.is_folder():
            return

        # decode .part.met.txtsrc files first... (DreaMule only)
        sources = {}

        for f in folder.get_children():
            path = f.path.lower()

            if path.endswith('.part.met.txtsrc'):
                peers = decoder_part_met_txtsrc.decode(f)

                if peers:
                    part_name = f.name[:-7]
                    sources[part_name] = peers

        # decode .part.met files
        part_met = set()

        for f in folder.get_children():
            path = f.path.lower()

            if path.endswith('.part.met'):
                source_list = sources.get(f.name, [])
                decoder_part_met.retrieve(self.__item, self.__data, f, source_list, username)
                part_met.add(path)

        # handle other control files
        for f in folder.get_children():
            path = f.path.lower()

            if path.endswith('.part.met'):
                pass  # already handled

            elif path.endswith('.part.met.txtsrc'):
                pass  # already handled

            elif path.endswith('.part.met.bak'):
                if path[:-4] not in part_met:
                    source_list = sources.get(f.name, [])
                    decoder_part_met.retrieve(self.__item, self.__data, f, source_list, username)
                else:
                    pymobius.p2p.set_handled(self.__item, f)

            elif path.endswith('.part.stats'):
                pymobius.p2p.set_handled(self.__item, f)

            elif path.endswith('.sd'):  # Shareaza control file
                pymobius.p2p.shareaza.decoder_sd.retrieve(self.__item, self.__data, f, None)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from AC_SearchStrings.dat
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_ac_searchstrings_dat(self, item, model, f, username):
        data = decoder_ac_searchstrings_dat.decode(f)
        if not data:
            return

        for text in data:
            search = pymobius.p2p.search.search()
            search.app_id = 'emule'
            search.app = 'Emule'
            search.username = username
            search.text = text
            search.count = -1
            search.add_metadata('Application', 'Emule')
            search.add_metadata('Search Term', text)
            search.add_metadata('AC_SearchStrings.dat path', f.path)
            model.searches.append(search)

        # set file handled
        pymobius.p2p.set_handled(item, f)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from cancelled.met
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_cancelled_met(self, item, model, f, username):
        data = decoder_cancelled_met.decode(f)
        if not data:
            return

        # set file handled
        pymobius.p2p.set_handled(item, f)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from key_index.dat
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_key_index_dat(self, item, model, f, username):
        data = decoder_key_index_dat.decode(f)
        if not data:
            return

        for key in data.keys:
            for source in key.sources:
                for name in source.names:
                    for ip, timestamp in name.ips:
                        rf = pymobius.p2p.remote_file.remote_file()
                        rf.username = username
                        rf.timestamp = timestamp
                        rf.app_id = 'emule'
                        rf.app = 'Emule'

                        rf.set_hash('ed2k', source.hash_ed2k)
                        rf.add_metadata('key_index.dat path', f.path)

                        # process tags
                        pymobius.p2p.emule.CTag.set_file_attributes(rf, name.tags)

                        # peer data
                        rf.peer.ip = ip

                        # add to model
                        model.remote_files.append(rf)

        # set file handled
        pymobius.p2p.set_handled(item, f)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from known.met
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_known_met(self, item, model, f, username):
        data = decoder_known_met.decode(f)
        if not data:
            return

        # process entries
        for entry in data.entries:
            lf = self.__new_local_file_from_cknowfile(entry)
            lf.username = username

            # path
            if lf.name and self.__incoming_dir:
                lf.path = self.__incoming_dir + '\\' + lf.name

            lf.add_metadata('Known.met path', f.path)
            model.local_files.append(lf)

        # set file handled
        pymobius.p2p.set_handled(item, f)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from preferences.dat
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_preferences_dat(self, item, model, f, username):
        data = decoder_preferences_dat.decode(f)
        if not data:
            return

        # create account
        account = pymobius.p2p.account.account()
        account.username = username
        account.app_id = 'emule'
        account.app = 'Emule'
        account.network = 'eDonkey'
        account.guid = data.guid
        model.accounts.append(account)

        # set file handled
        pymobius.p2p.set_handled(item, f)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from preferences.ini
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_preferences_ini(self, item, model, f, username):
        self.__incoming_dir = None
        self.__temp_dir = None

        if not f:
            return

        data = decoder_preferences_ini.decode(f)
        if not data:
            return

        self.__incoming_dir = data.incoming_dir
        self.__temp_dir = data.temp_dir

        # set file handled
        pymobius.p2p.set_handled(item, f)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from preferencesKad.dat
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_preferenceskad_dat(self, item, model, f, username):
        data = decoder_preferenceskad_dat.decode(f)
        if not data:
            return

        # create account
        account = pymobius.p2p.account.account()
        account.username = username
        account.app_id = 'emule'
        account.app = 'Emule'
        account.network = 'Kademlia'
        account.guid = data.client_id.upper()
        model.accounts.append(account)

        # set file handled
        pymobius.p2p.set_handled(item, f)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from StoredSearches.met
    # @todo Find a suitable timestamp for cfiles, to add them to remote files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_stored_searches_met(self, item, model, f, username):
        data = decoder_stored_searches_met.decode(f)
        if not data:
            return

        for csearch in data.csearches:
            search = pymobius.p2p.search.search()
            search.app_id = 'emule'
            search.app = 'Emule'
            search.username = username
            search.text = csearch.expression

            # add remote files
            for cfile in csearch.files:
                info = CTag.get_metadata(cfile.tags)

                rf = pymobius.p2p.remote_file.remote_file()
                rf.app_id = 'emule'
                rf.app = 'Emule'
                rf.name = info.get('name')
                rf.size = info.get('size')
                rf.username = username

                # hashes (ED2K, AICH)
                rf.set_hash('ed2k', cfile.hash_ed2k)
                if 'hash_aich' in info:
                    rf.set_hash('aich', info.get('hash_aich'))

                # tags
                self.__set_file_attributes(rf, info)

                # peer data
                if cfile.user_ip != '0.0.0.0':
                    rf.peer.ip = cfile.user_ip
                    rf.peer.port = cfile.user_port

                rf.add_metadata('Search Term', search.text)
                search.add_file(rf)

            # add metadata
            search.add_metadata('Case Item', item.name)
            search.add_metadata('Application', 'Emule')
            search.add_metadata('Search ID', '%08x' % csearch.search_id)
            search.add_metadata('Search Term', csearch.expression)
            search.add_metadata('Special Title', csearch.special_title)
            search.add_metadata('Filetype', csearch.filetype)
            search.add_metadata('Hits', search.get_file_count())
            search.add_metadata('StoredSearches.met path', f.path)

            # add to search list
            model.searches.append(search)

        # set file handled
        pymobius.p2p.set_handled(item, f)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Create new local file from CKnownFile structure
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __new_local_file_from_cknowfile(self, entry):

        # get info from CTags
        info = CTag.get_metadata(entry.tags)

        # create local files
        f = local_file()
        f.app_id = 'emule'
        f.app = 'Emule'
        f.name = info.get('name')
        f.size = info.get('size', 0)

        # flags
        f.flag_shared = STATE_ALWAYS
        f.flag_corrupted = STATE_YES if info.get('corrupted_parts') else STATE_NO
        f.flag_downloaded = STATE_YES
        f.flag_uploaded = STATE_YES if info.get('uploaded_bytes') > 0 or info.get('times_accepted', 0) > 0 else STATE_NO
        f.flag_completed = STATE_YES  # @see CPartFile::PerformFileCompleteEnd

        # hashes (ED2K, AICH)
        f.set_hash('ed2k', entry.hash_ed2k)
        if 'hash_aich' in info:
            f.set_hash('aich', info.get('hash_aich'))

        # metadata
        self.__set_file_attributes(f, info)

        return f

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set file attributes from tags
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __set_file_attributes(self, f, info):

        def add_metadata(f, name, value):
            if value is not None:
                f.add_metadata(name, value)

        f.name = info.get('name')
        f.size = info.get('size', -1)

        add_metadata(f, 'File type', info.get('filetype'))
        add_metadata(f, 'File format', info.get('fileformat'))
        add_metadata(f, 'Last seen complete date/time (UTC)', info.get('last_seen_time'))
        add_metadata(f, '.part file path', info.get('part_path'))
        add_metadata(f, 'Bytes actually downloaded', info.get('bytes_downloaded'))
        add_metadata(f, 'Description', info.get('description'))
        add_metadata(f, 'Version', info.get('version'))
        add_metadata(f, '.part file name', info.get('part_name'))
        add_metadata(f, 'Sources', info.get('sources'))
        add_metadata(f, 'Download priority', info.get('dl_priority'))
        add_metadata(f, 'Upload priority', info.get('ul_priority'))
        add_metadata(f, 'Compression gain in bytes', info.get('compression_gain'))
        add_metadata(f, 'Corrupted loss in bytes', info.get('corrupted_loss'))
        add_metadata(f, 'KAD last published date/time (UTC)', info.get('kad_last_published_time'))
        add_metadata(f, 'Flags', info.get('flags'))
        add_metadata(f, 'Download active time', info.get('download_active_time'))
        add_metadata(f, 'Complete sources', info.get('complete_sources'))
        add_metadata(f, 'Last shared date/time (UTC)', info.get('last_shared_time'))
        add_metadata(f, 'Times requested', info.get('times_requested'))
        add_metadata(f, 'Times accepted', info.get('times_accepted'))
        add_metadata(f, 'Category', info.get('category'))
        add_metadata(f, 'Max sources', info.get('max_sources'))
        add_metadata(f, 'Last data update date/time (UTC)', info.get('last_update_time'))
        add_metadata(f, 'Media artist', info.get('media_artist'))
        add_metadata(f, 'Media album', info.get('media_album'))
        add_metadata(f, 'Media title', info.get('media_title'))
        add_metadata(f, 'Media length', info.get('media_length'))
        add_metadata(f, 'Media bitrate', info.get('media_bitrate'))
        add_metadata(f, 'Media Codec', info.get('media_codec'))
        add_metadata(f, 'File comment', info.get('file_comment'))
        add_metadata(f, 'File rating', info.get('file_rating'))

        # other metadata
        value = info.get('uploaded_bytes', 0)
        if value > 0:
            f.add_metadata('Bytes uploaded', value)

        value = info.get('not_counted_uploaded_bytes', 0)
        if value > 0:
            f.add_metadata('Bytes uploaded (not counted)', value)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief normalize retrieved data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __normalize_data(self):

        # remove duplicated search entries, preferring searches with len (files) > 0
        hit_searches = set((s.text for s in self.__data.searches if s.has_files()))
        self.__data.searches = [s for s in self.__data.searches if s.has_files() or s.text not in hit_searches]

        # add country to remote_files, when possible
        if self.__ip_table:
            for f in self.__data.remote_files:
                if f.peer.ip:
                    try:
                        f.peer.country_code, f.peer.country = self.__ip_table.get(f.peer.ip)
                    except Exception as e:
                        mobius.core.logf('WRN ' + str(e) + ' ' + traceback.format_exc())

        # add URL to remote_files
        for f in self.__data.remote_files:
            if not f.url and f.name and f.size != -1:
                f.url = 'ed2k://|file|%s|%d|%s|/' % (urllib.parse.quote(f.name), f.size, f.get_hash('ed2k'))

        # sort and remove duplicated remote files
        remote_files = [(f.timestamp, f) for f in self.__data.remote_files]
        timestamp, peer_ip, size, name = None, None, None, None
        self.__data.remote_files = []

        for timestamp, f in sorted(remote_files, key=lambda x: x[0]):
            if (timestamp, peer_ip, size, name) != (f.timestamp, f.peer.ip, f.size, f.name):
                self.__data.remote_files.append(f)
                timestamp, peer_ip, size, name = f.timestamp, f.peer.ip, f.size, f.name
