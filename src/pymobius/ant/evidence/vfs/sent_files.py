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
import pymobius
import pymobius.app.skype

ANT_ID = 'sent-files'
ANT_NAME = 'Sent files'
ANT_VERSION = '1.0'
EVIDENCE_TYPE = 'sent-file'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: sent-files
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.id = ANT_ID
        self.name = ANT_NAME
        self.version = ANT_VERSION
        self.__item = item
        self.__entries = []

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__entries = []

        # check if datasource is available
        datasource = self.__item.get_datasource()

        if not datasource:
            return

        if not datasource.is_available():
            raise Exception('Datasource is not available')

        # retrieve data
        self.__retrieve_skype()

        # save data
        self.__save_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Skype
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_skype(self):
        try:
            model = pymobius.app.skype.model(self.__item)

            for profile in model.get_profiles():
                self.__retrieve_skype_profile(profile)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)} {traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Skype profile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_skype_profile(self, profile):
        DOWNLOAD_STATE = {0: 'Not initiated', 7: 'Cancelled', 8: 'Completed', 9: 'Error'}

        try:
            for ft in profile.get_file_transfers():
                if ft.type == 2:
                    entry = pymobius.Data()

                    entry.username = profile.username
                    entry.timestamp = ft.start_time
                    entry.filename = ft.filename
                    entry.path = ft.path
                    entry.app_id = 'skype'
                    entry.app_name = 'Skype'

                    entry.metadata = mobius.core.pod.map()
                    entry.metadata.set('size', ft.size)
                    entry.metadata.set('start-time', ft.start_time)
                    entry.metadata.set('end-time', ft.finish_time)
                    entry.metadata.set('bytes-sent', ft.bytes_transferred)
                    entry.metadata.set('transfer-state', DOWNLOAD_STATE.get(ft.status, 'Unknown (%d)' % ft.status))

                    sender = ft.from_skype_account
                    if ft.from_skype_name:
                        sender += f' ({ft.from_skype_name})'

                    receiver = ft.to_skype_account
                    if ft.to_skype_name:
                        receiver += f' ({ft.to_skype_name})'

                    entry.metadata.set('sender-account', sender)
                    entry.metadata.set('receiver-account', receiver)

                    self.__entries.append(entry)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)} {traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        transaction = self.__item.new_transaction()

        # save evidences
        for e in self.__entries:
            entry = self.__item.new_evidence(EVIDENCE_TYPE)

            entry.username = e.username
            entry.timestamp = e.timestamp
            entry.filename = e.filename
            entry.path = e.path
            entry.app_id = e.app_id
            entry.app_name = e.app_name
            entry.metadata = e.metadata

        # commit data
        transaction.commit()
