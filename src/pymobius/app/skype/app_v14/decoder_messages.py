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
import json
import traceback

import mobius
import pymobius
import pymobius.app.skype.message_parser

DEBUG = False

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Message types
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SYSTEM_MESSAGES = {
    'RichText/Media_AudioMsg': 'Audio message shared',
    'RichText/UriObject': 'Picture file shared',
    'RichText/Contacts': 'Contacts shared',
    'RichText/GenericFile': 'Generic file shared',
    'RichText/Media_CallRecording': 'Recording audio',
    'Notice': 'Notice received',
    'PopCard': 'PopCard received',
}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode "messagesv12" table
# @param db Database object (s4l-xxx.db)
# @param name Skype user name
# @return messages, names
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(db, name):
    messages = []
    names = {}

    if not db:
        return messages, names

    # load conversion member from conversationsv14 table
    conversations = {}

    SQL_STATEMENT = '''
       SELECT nsp_pk,
              nsp_data
         FROM conversationsv14'''

    for row in db.execute(SQL_STATEMENT):
        nsp_pk = row[0]
        nsp_data = json.loads(row[1])

        members = set()
        members.add(nsp_pk.split(':', 1)[1])
        members.add(name)

        conv_data = nsp_data.get('conv')

        if conv_data:
            for m in conv_data.get('memberConsumptionHorizonsSorted', []):
                member_id = m.get('id').split(':', 1)[1]
                members.add(member_id)

        conversations[nsp_pk[1:]] = members

    # Load messages from messagesv12 table
    SQL_STATEMENT = '''
       SELECT nsp_pk,
              nsp_data
         FROM messagesv12'''

    for row in db.execute(SQL_STATEMENT):
        try:
            message = decode_message(row)
            messages.append(message)

            # set message.recipients
            members = conversations.get(message.conversation_id, set([name]))
            message.recipients = set((m_id, names.get(m_id)) for m_id in members if m_id != message.sender_id)

            # add Skype name, if available
            if message.sender_name and message.sender_name != message.sender_id:
                names[message.sender_id] = message.sender_name

        except Exception as e:
            mobius.core.logf('WRN ' + str(e) + ' ' + traceback.format_exc())

    # Set skype names from skype account IDs
    for message in messages:

        if not message.sender_name:
            message.sender_name = names.get(message.sender_id)

        recipients = set()
        for r_id, r_name in message.recipients:
            if not r_name:
                r_name = names.get(r_id)
            recipients.add((r_id, r_name))

        message.recipients = recipients

    return messages, names


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode a row from messagesv12 table
# @param row Table row
# @return Message object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_message(row):
    message = pymobius.Data()

    if DEBUG:
        mobius.core.logf('DBG ' + str(row))

    # decode data
    nsp_pk = row[0]
    nsp_data = json.loads(row[1])

    if DEBUG:
        mobius.core.logf('DBG nsp_data')

        for key, value in sorted(nsp_data.items()):
            mobius.core.logf('DBG %s=%s' % (str(key), str(value)))

    # fill attributes
    message.id = nsp_pk
    message.chatname = nsp_pk  # @deprecated
    message.sender_id = None
    message.sender_name = None
    message.compose_time = None
    message.timestamp = None
    message.conversation_id = nsp_data.get('conversationId', '')
    message.status = 0  # None #row[5]
    message.type = nsp_data.get('messagetype')
    message.raw_text = nsp_data.get('content', '').rstrip()
    message.recipients = []

    mobius.core.logf('DBG conversation ID:' + message.conversation_id)

    # sender ID
    creator = nsp_data.get('creator', '')
    message.sender_id = creator.split(':', 1)[1]

    mobius.core.logf('DBG sender:' + message.sender_id)

    # compose time
    compose_time_data = nsp_data.get('composeTime')
    if compose_time_data:
        message.compose_time = mobius.core.datetime.new_datetime_from_unix_timestamp(int(compose_time_data) // 1000)

    # timestamp
    created_time_data = nsp_data.get('createdTime')
    if created_time_data:
        message.timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(int(created_time_data) // 1000)

    # decode _serverMessages
    smsg = nsp_data.get('_serverMessages')
    smsg_data = smsg[0]

    message.sender_name = smsg_data.get('imdisplayname')
    if DEBUG:
        mobius.core.logf('DBG SKYPE_NAME ' + message.sender_id + message.sender_name)

        for key, value in sorted(smsg_data.items()):
            mobius.core.logf('DBG %s=%s' % (str(key), str(value)))

    # parse text
    parser = pymobius.app.skype.message_parser.MessageParser(message.raw_text)
    text = SYSTEM_MESSAGES.get(message.type)

    if text:
        text = text.replace('_', ' ')
        text = text.capitalize()
        parser.add_element({'type': 'system', 'text': text})

    if message.type == 'PopCard':
        message.text = parser.parse_popcard()

    elif message.type == 'Notice':
        message.text = parser.parse_notice()

    else:
        message.text = parser.parse()

    return message
