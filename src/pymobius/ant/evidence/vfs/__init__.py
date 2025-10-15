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
import mobius.framework
import pymobius.ant.turing

from . import accounts
from . import autofill
from . import bookmarked_urls
from . import calls
from . import chat_messages
from . import cookies
from . import installed_programs
from . import opened_files
from . import received_files
from . import sent_files
from . import trash_can_entries
from . import visited_urls
from . import wireless_connections
from . import wireless_networks

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ANT_ID = 'evidence.vfs'
ANT_NAME = 'Evidence Loader Agent - VFS'
ANT_VERSION = '1.1'

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ants for each evidence type
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ANTS = [
    pymobius.ant.turing.Ant,
    accounts.Ant,
    autofill.Ant,
    bookmarked_urls.Ant,
    calls.Ant,
    chat_messages.Ant,
    cookies.Ant,
    installed_programs.Ant,
    opened_files.Ant,
    received_files.Ant,
    sent_files.Ant,
    trash_can_entries.Ant,
    visited_urls.Ant,
    wireless_connections.Ant,
    wireless_networks.Ant,
]

LOADERS = [
    "app-emule",
    "app-emuletorrent",
    "app-shareaza",
]


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: VFS Evidence
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item, profile):
        self.id = ANT_ID
        self.name = ANT_NAME
        self.version = ANT_VERSION

        self.__item = item
        self.__datasource = item.get_datasource()
        self.__profile = profile
        self.__vfs = self.__datasource.get_vfs()
        self.__step_number = ''
        self.__step_name = ''
        self.__ant = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get status
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_status(self):
        status = mobius.core.pod.map()
        status.set('step_number', self.__step_number)
        status.set('step_name', self.__step_name)

        if self.__ant:
            status.update(self.__ant.get_status())
            
        return status
    
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        mobius.core.logf(f"INF ant {self.id} started")

        # run sub-ants
        for idx, ant_class in enumerate(ANTS, 1):
            ant = ant_class(self.__item)

            self.__step_number = f"1.{idx}"
            self.__step_name = ant.name
            
            mobius.core.logf(f"INF ant.run started: {ant.name}")

            try:
                ant.run()
            except Exception as e:
                mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

            mobius.core.logf(f"INF ant.run ended: {ant.name}")

        # run vfs_processor
        self.__step_number = "2"
        self.__step_name = "VFS Processor"
        
        self.__ant = mobius.framework.ant.vfs_processor(self.__item, self.__profile)
        self.__ant.run()
        self.__ant = None
        
        # run evidence loader ants
        for idx, loader in enumerate(LOADERS, 1):
            mobius.core.logf(f"INF evidence loader started: {loader}")

            try:
                self.__item.reset_ant(f"evidence.{loader}")
                ant = mobius.framework.evidence_loader(loader, self.__item)
                
                self.__step_number = f"3.{idx}"
                self.__step_name = f"{loader} loader"
                
                ant.run()
            except Exception as e:
                mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

            mobius.core.logf(f"INF evidence loader ended: {loader}")

        mobius.core.logf(f"INF ant {self.id} ended")
