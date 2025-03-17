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

from . import ufdr
from . import vfs
from .post import ip_addresses_from_cookies as post_ip_addresses_from_cookies
from .post import kff_alert as post_kff_alert
from .post import searched_texts_from_visited_urls as post_searched_texts_from_visited_urls

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ANT_ID = 'evidence'
ANT_NAME = 'Evidence Finder Agent'
ANT_VERSION = '1.0'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Profiles (id, name)
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PROFILES = [
    ('ufdr.general', 'UFDR General'),
    ('ufdr.pedo', 'UFDR Pedo'),
    ('vfs.general', 'VFS General'),
    ('vfs.pedo', 'VFS Pedo'),
]


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Post-processing ants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ANTS = [
    post_ip_addresses_from_cookies.Ant,
    post_searched_texts_from_visited_urls.Ant,
    post_kff_alert.Ant,
]


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: Evidence
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # @param item Item object
    # @param profile_id Profile ID
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item, profile_id):
        self.id = ANT_ID
        self.name = ANT_NAME
        self.version = ANT_VERSION
        self.__item = item
        self.__profile_id = profile_id

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        mobius.core.logf(f"INF ant {self.id} started. Profile: {self.__profile_id}")

        # create new connections to case db and config db (for multi-threading)
        case_connection = self.__item.new_connection()

        # remove old evidences
        transaction = self.__item.new_transaction()
        self.__item.remove_evidences()
        transaction.commit()

        # run children
        self.__run_ants()

        # set ant run
        transaction = self.__item.new_transaction()
        self.__item.set_ant(ANT_ID, ANT_NAME, ANT_VERSION)
        transaction.commit()

        mobius.core.logf(f"INF ant {self.id} ended")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Reset ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def reset(self):
        # create new connections to case db and config db (for multi-threading)
        case_connection = self.__item.new_connection()

        datasource = self.__item.get_datasource()

        if not datasource:
            return

        elif datasource.get_type() == 'ufdr':
            ant = ufdr.Ant(self.__item)
            ant.reset()

        elif datasource.get_type() == 'vfs':
            ant = vfs.Ant(self.__item)
            ant.reset()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run children ants
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __run_ants(self):

        # run main ant, according to datasource type
        datasource = self.__item.get_datasource()

        if not datasource:
            return

        elif datasource.get_type() == 'ufdr':
            ant = ufdr.Ant(self.__item)
            ant.run()

        elif datasource.get_type() == 'vfs':
            ant = vfs.Ant(self.__item)
            ant.run()

        # run post-processing ants
        for ant_class in ANTS:
            ant = ant_class(self.__item)
            mobius.core.logf(f"DBG ant.run started: {ant.name}")

            try:
                ant.run()
            except Exception as e:
                mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

            mobius.core.logf(f"DBG ant.run ended: {ant.name}")
