# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008-2026 Eduardo Aguiar
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
import mobius
import pymobius.app.skype.message_parser

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Message types
# @see https://skpy.t.allofti.me/protocol/chat.html
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
MESSAGE_TYPE_A = {
    2: 'PlainText',
    4: 'CONFERENCE_CALL_STARTED',
    10: 'CHAT_MEMBER_ADDED',
    12: 'CHAT_MEMBER_REMOVED',
    13: 'CHAT_ENDED',
    30: 'Event/CallStarted',
    39: 'Event/CallEnded',
    50: 'AUTHORIZATION_REQUESTED',
    51: 'AUTHORIZATION_GIVEN',
    53: 'BLOCKED',
    60: 'EMOTICON_SENT',
    61: 'RichText',
    63: 'RichText/Contacts',
    64: 'RichText/Sms',
    67: 'VOICE_MSG_SENT',
    68: 'RichText/Files',
    70: 'VIDEO_MESSAGE',
    110: 'BIRTHDATE',
    201: 'RichText/UriObject',
    253: 'VIDEO_SHARED',
}

SYSTEM_MESSAGES = {
    4: 'Conference call started',
    10: 'Chat member added',
    12: 'Chat member removed',
    13: 'Chat ended',
    30: 'Call started',
    39: 'Call ended',
    50: 'Authorization requested',
    51: 'Authorization given',
    53: 'User blocked',
    63: 'Contacts info sent',
    64: 'SMS sent',
    68: 'Files sent',
}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get participants from chatname
# chatname format = #participant1/$participant2;hash
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_participants_from_chatname(chatname):
    participants = set()

    if chatname and chatname[0] == '#':
        pos = chatname.find('/$')

        if pos != -1:
            participants.add(chatname[1:pos])

            pos2 = chatname.find(';', pos)
            if pos2 != -1:
                participants.add(chatname[pos + 2:pos2])

    return participants


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode "messages" table
# @param db Database object
# @return messages, names
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(db):
    names = {}
    messages = []

    # get account, name from Messages table
    SQL_STATEMENT = '''
     SELECT DISTINCT author, from_dispname
                FROM messages'''

    for row in db.execute(SQL_STATEMENT):
        account_id = row[0]
        account_name = row[1]
        names[account_id] = account_name

    # get conversation participants
    participants = {}

    SQL_STATEMENT = '''
     SELECT convo_id,
            identity
       FROM participants'''

    for row in db.execute(SQL_STATEMENT):
        convo_id = row[0]
        identity = row[1]
        participants.setdefault(convo_id, set()).add(identity)

    # get messages
    SQL_STATEMENT = '''
       SELECT chatname,
              timestamp,
              author,
              from_dispname,
              body_xml,
              chatmsg_status,
              chatmsg_type,
              type,
              convo_id
         FROM messages'''

    for idx, row in enumerate(db.execute(SQL_STATEMENT)):
        message = pymobius.Data()
        message.id = row[0]
        message.chatname = row[0]  # @deprecated
        message.timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(row[1])
        message.sender_id = row[2]
        message.sender_name = row[3]
        message.status = row[5]
        message.chatmsg_type = row[6]
        message.raw_text = (row[4] or '').rstrip()
        message.type = MESSAGE_TYPE_A.get(row[7], 'Unknown type (%d)' % row[7])

        if row[7] not in MESSAGE_TYPE_A:
            mobius.core.logf('app.skype: unknown MESSAGE_TYPE_A %d. Timestamp: %s. Message: %s' % (
            row[7], message.timestamp, message.raw_text))

        # recipients
        convo_id = row[8]

        if convo_id == 0:
            recipients = get_participants_from_chatname(message.chatname)
        else:
            recipients = set(p for p in participants.get(convo_id, []))

        recipients.discard(message.sender_id)  # discard sender from recipients

        message.recipients = [(account_id, names.get(account_id)) for account_id in recipients]

        # text
        parser = pymobius.app.skype.message_parser.MessageParser(message.raw_text)
        text = SYSTEM_MESSAGES.get(message.type)

        if text:
            text = text.replace('_', ' ')
            text = text.capitalize()
            parser.add_element({'type': 'system', 'text': text})

        message.text = parser.parse()

        # add message to list
        messages.append(message)

    return messages, names
