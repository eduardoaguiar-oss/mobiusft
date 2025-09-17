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
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.id = ANT_ID
        self.name = ANT_NAME
        self.version = ANT_VERSION
        self.__item = item
        self.__profile_id = None
        self.__control = None
        self.__evidence_msg_path = self.__item.create_data_path('evidence.msg')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set control object
    # @param control Control object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_control(self, control):
        self.__control = control

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set profile ID
    # @param profile_id Profile ID
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_profile_id(self, profile_id):
        self.__profile_id = profile_id

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get processing status
    # @return Status dictionary
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_status(self):
        status = mobius.core.pod.map()
        status.set('id', self.id)
        status.set('name', self.name)
        status.set('version', self.version)
        status.set('profile_id', self.__profile_id)

        return status
    
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get log messages, if any
    # @return Messages
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_messages(self):
        messages = []

        f = mobius.core.io.new_file_by_path(self.__evidence_msg_path)
        if f.exists():
            reader = mobius.core.io.line_reader(f.new_reader())

            for line in reader:
                if '\t' in line:
                    messages.append(line.split('\t'))

        return messages

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Clear log messages
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def clear_messages(self):
        f = mobius.core.io.new_file_by_path(self.__evidence_msg_path)

        if f.exists():
            f.remove()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.log(f"INF ant {self.id} started")

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

        self.log(f"INF ant {self.id} ended")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Reset ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def reset(self):
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
            ant.set_control(self)
            ant.run()

        elif datasource.get_type() == 'vfs':
            ant = vfs.Ant(self.__item)
            ant.set_control(self)
            ant.run()

        # run post-processing ants
        ant = mobius.framework.ant.post_processor(self.__item)
        self.log(f"INF ant.post_processor started")
        ant.run()
        self.log(f"INF ant.post_processor ended")

        ants = ANTS + [r.value for r in mobius.core.get_resources('evidence.post')]

        for ant_class in ants:
            ant = ant_class(self.__item)
            self.log(f"INF Post-processing ant started: {ant.name}")

            try:
                ant.run()
            except Exception as e:
                self.log(f'WRN {str(e)}\n{traceback.format_exc()}')

            self.log(f"INF Post-processing ant ended: {ant.name}")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Write log
    # @param text Text message
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def log(self, text):
        mobius.core.logf(text)

        timestamp = datetime.datetime.utcnow()

        fp = open(self.__evidence_msg_path, 'a')
        fp.write(f"{timestamp}\t{text}\n")
        fp.close()

        if self.__control:
            self.__control.add_message(self.__item, timestamp, text)
