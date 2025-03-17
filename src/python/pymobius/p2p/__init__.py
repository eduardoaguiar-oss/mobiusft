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
import os.path

import mobius
import pymobius.p2p.emule
import pymobius.registry
import pymobius.registry.main

from . import model

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
STATE_NO = 0
STATE_YES = 1
STATE_ALWAYS = 2
STATE_UNKNOWN = -1

ANT_ID = 'p2p'
ANT_NAME = 'P2P Agent'
ANT_VERSION = '1.1'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Add file to item's kff ignored file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def set_handled(item, f):
    # create file reader
    if not f:
        return

    reader = f.new_reader()
    if not reader:
        return

    # calculate hash sha2-512
    h = mobius.crypt.hash('sha2-512')
    data = reader.read(65536)

    while data:
        h.update(data)
        data = reader.read(65536)

    # add to kff file
    case = item.case
    path = case.create_path(os.path.join('hashset', f'{item.uid:04d}-p2p.ignore'))
    fp = open(path, 'a')
    fp.write('%s\n' % h.get_hex_digest())
    fp.close()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief generate unique list from list
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def unique(rows):
    old_key = None
    objs = []

    for row in rows:
        obj = row[0]
        key = row[1:]

        if key != old_key:
            objs.append(obj)
            old_key = key

    return objs


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief System user
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class User(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, name):
        self.name = name
        self.appdata_dir = None
        self.local_appdata_dir = None


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief forensics: P2P
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.__item = item
        self.__model = None

        if item.has_ant(ANT_ID):
            self.__model = model.model(item)
            self.__is_data_available = self.__model.load()

        elif item.has_datasource():
            self.__is_data_available = False

        else:
            self.__is_data_available = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief check if data is available
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def is_data_available(self):
        return self.__is_data_available

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_data(self):
        if not self.__is_data_available:
            self.__retrieve_data()

        return self.__model

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief retrieve data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_data(self):
        datasource = self.__item.get_datasource()

        if not datasource.is_available():
            raise Exception('Datasource is not available')

        mobius.core.logf('DBG 1')
        self.__model = model.model(self.__item)

        # Clear p2p ignored kff, if any
        mobius.core.logf('DBG 2')
        case = self.__item.case
        path = case.get_path(os.path.join('hashset', f'{self.__item.uid:04d}-p2p.ignore'))

        if os.path.exists(path):
            os.remove(path)

        # Retrieve activity data
        mobius.core.logf('DBG 3')
        pymobius.p2p.emule.retrieve(self.__model)

        # Normalize data
        mobius.core.logf('DBG 4')
        self.__calculate_totals()
        self.__model.accounts = unique((a, a.network, a.guid) for a in self.__model.accounts)

        # Save data model
        mobius.core.logf('DBG 5')
        path = self.__item.create_data_path('p2p.json')
        self.__model.save(path)
        self.__is_data_available = True
        self.__item.set_ant(ANT_ID, ANT_NAME, ANT_VERSION)
        mobius.core.logf('DBG 6')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Calculate totals by application
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __calculate_totals(self):
        application = dict((a.id, a) for a in self.__model.applications)

        # Reset totals
        for app in application.values():
            app.total_searches = 0
            app.total_local_files = 0
            app.total_remote_files = 0

        # Calculate totals
        for search in self.__model.searches:
            application[search.app_id].total_searches += 1

        for local_file in self.__model.local_files:
            application[local_file.app_id].total_local_files += 1

        for remote_file in self.__model.remote_files:
            application[remote_file.app_id].total_remote_files += 1
