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
import traceback

import mobius

from . import ufdr
from . import vfs
from .post import ip_addresses_from_cookies as post_ip_addresses_from_cookies
from .post import kff_alert as post_kff_alert
from .post import searched_texts_from_visited_urls as post_searched_texts_from_visited_urls
from .post import user_accounts_from_cookies as post_user_accounts_from_cookies

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ANT_ID = 'evidence'
ANT_NAME = 'Evidence Finder Agent'
ANT_VERSION = '1.1'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Post-processing ants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ANTS = [
    post_ip_addresses_from_cookies.Ant,
    post_searched_texts_from_visited_urls.Ant,
    post_kff_alert.Ant,
    post_user_accounts_from_cookies.Ant,
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
        self.__started_time = datetime.datetime.now()
        self.__phase_number = 0
        self.__phase_name = ''
        self.__step_number = 0
        self.__step_name = ''
        self.__ant = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get processing status
    # @return Status dictionary
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_status(self):
        status = mobius.core.pod.map()
        status.set('profile_id', self.__profile_id)
        status.set('started_time', self.__started_time)
        status.set('current_time', datetime.datetime.now())
        status.set('phase_number', f"{self.__phase_number} of 3")
        status.set('phase_name', self.__phase_name)
        status.set('step_number', self.__step_number)
        status.set('step_name', self.__step_name)

        if self.__ant:
            ant_status = self.__ant.get_status()
            status.update(ant_status)
            
        return status
    
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        mobius.core.logf(f"INF ant {self.id} started")

        # create new connections to case db (for multi-threading)
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
        case_connection = self.__item.new_connection()
        self.__item.reset_ant('evidence')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run children ants
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __run_ants(self):

        # run main ant, according to datasource type
        datasource = self.__item.get_datasource()

        if not datasource:
            return

        self.__phase_number = 1
        self.__phase_name = f"Evidence Loader"

        if datasource.get_type() == 'ufdr':
            self.__ant = ufdr.Ant(self.__item, self.__profile_id)
            self.__ant.run()

        elif datasource.get_type() == 'vfs':
            self.__ant = vfs.Ant(self.__item, self.__profile_id)
            self.__ant.run()

        # run post-processing ant
        self.__phase_number = 2
        self.__phase_name = f"ant.post_processor"
        self.__ant = None

        ant = mobius.framework.ant.post_processor(self.__item)
        mobius.core.logf(f"INF ant.post_processor started")
        ant.run()
        mobius.core.logf(f"INF ant.post_processor ended")

        # run post-processing ants (from resources)
        self.__phase_number = 3
        self.__phase_name = f"Post-processing ants"
        ants = ANTS + [r.value for r in mobius.core.get_resources('evidence.post')]

        for idx, ant_class in enumerate(ants, 1):
            ant = ant_class(self.__item)

            self.__step_number = f"{idx}"
            self.__step_name = f"Post-processor {ant.name}"

            mobius.core.logf(f"INF Post-processing ant started: {ant.name}")

            try:
                ant.run()
            except Exception as e:
                mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

            mobius.core.logf(f"INF Post-processing ant ended: {ant.name}")
