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
from ctypes import pythonapi

import mobius
import pymobius.ant.turing
from . import accounts
from . import autofill
from . import bookmarked_urls
from . import calls
from . import chat_messages
from . import cookies
from . import installed_programs
from . import ip_addresses
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
ANT_VERSION = '1.0'

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
    ip_addresses.Ant,
    opened_files.Ant,
    received_files.Ant,
    sent_files.Ant,
    trash_can_entries.Ant,
    visited_urls.Ant,
    wireless_connections.Ant,
    wireless_networks.Ant,
]

LOADERS = [
    "app-ares",
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
    def __init__(self, item):
        self.id = ANT_ID
        self.name = ANT_NAME
        self.version = ANT_VERSION

        self.__item = item
        self.__datasource = item.get_datasource()
        self.__vfs = self.__datasource.get_vfs()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        mobius.core.logf(f"INF ant {self.id} started")

        # run sub-ants
        for ant_class in ANTS:
            ant = ant_class(self.__item)
            mobius.core.logf(f"DBG ant.run started: {ant.name}")

            try:
                ant.run()
            except Exception as e:
                mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

            mobius.core.logf(f"DBG ant.run ended: {ant.name}")

        # run evidence loader ants
        for loader in LOADERS:
            mobius.core.logf(f"DBG ant.run started: evidence_loader.{loader}")
            ant = mobius.framework.evidence_loader(loader, self.__item)
            ant.run()
            mobius.core.logf(f"DBG ant.run ended: evidence_loader.{loader}")

        mobius.core.logf(f"INF ant {self.id} ended")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Reset ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def reset(self):
        for loader in LOADERS:
            self.__item.reset_ant(f'evidence.{loader}')

        self.__item.reset_ant('evidence')
