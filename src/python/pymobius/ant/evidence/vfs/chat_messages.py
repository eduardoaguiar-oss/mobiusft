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

ANT_ID = 'chat-messages'
ANT_NAME = 'Chat Messages'
ANT_VERSION = '1.1'
EVIDENCE_TYPE = 'chat-message'

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
MESSAGE_STATUS = {1: 'Sending', 2: 'Sent', 3: 'Received', 4: 'Read'}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: chat-messages
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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

        # check if datasource is available
        datasource = self.__item.get_datasource()

        if not datasource:
            return

        if not datasource.is_available():
            raise Exception('Datasource is not available')

        # retrieve data
        self.__entries = []
        self.__retrieve_skype()
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
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Skype profile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_skype_profile(self, profile):

        try:
            for msg in profile.get_chat_messages():
                entry = pymobius.Data()
                entry.timestamp = msg.timestamp
                entry.text = msg.text[:]
                entry.app_name = 'Skype'
                entry.username = profile.username

                # get recipients
                entry.recipients = []

                for r_id, r_name in msg.recipients:
                    r = r_id
                    if r_name and r_id != r_name:
                        r += f' ({r_name})'
                    entry.recipients.append(r)

                entry.sender = msg.sender_id
                if msg.sender_name:
                    entry.sender += ' (' + msg.sender_name + ')'

                entry.metadata = mobius.pod.map()
                entry.metadata.set('chatname', msg.chatname)
                entry.metadata.set('raw_text', msg.raw_text)
                entry.metadata.set('message_type', msg.type)
                entry.metadata.set('sender_id', msg.sender_id)
                entry.metadata.set('sender_name', msg.sender_name)
                entry.metadata.set('recipient_id', r_id)
                entry.metadata.set('recipient_name', r_name)

                if msg.status:
                    entry.metadata.set('status', MESSAGE_STATUS.get(msg.status, f'Unknown: {msg.status:d}'))

                if hasattr(msg, 'conversation_id'):
                    entry.metadata.set('conversation_id', msg.conversation_id)

                entry.metadata.set('profile_name', profile.name)
                entry.metadata.set('profile_path', profile.path)
                entry.metadata.set('app_id', 'skype')

                self.__entries.append(entry)

        except Exception as e:
            mobius.core.logf(f"WRN {e}\n{traceback.format_exc()}")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        transaction = self.__item.new_transaction()

        # save evidences
        for e in self.__entries:
            msg = self.__item.new_evidence(EVIDENCE_TYPE)
            msg.timestamp = e.timestamp
            msg.sender = e.sender
            msg.recipients = e.recipients
            msg.text = e.text
            msg.app = e.app_name
            msg.username = e.username
            msg.metadata = e.metadata

        # commit data
        transaction.commit()
