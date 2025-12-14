# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import mobius
import pymobius
import pymobius.app.skype.message_parser

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Message types
# @see https://skpy.t.allofti.me/protocol/chat.html
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
MESSAGE_TYPE_B = {
    2: 'RichText',
    3: 'RichText/UriObject',
    6: 'RichText/GenericFile',
    8: 'VIDEO_SHARED',
    9: 'RichText/Media_AudioMsg',
    10: 'EVENT',
    501: 'PopCard',
    1001: 'ThreadActivity/DeleteMember'
}

SYSTEM_MESSAGES = {
}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode "messages" table
# @param db Database object (skype.db)
# @return messages, names
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(db, account_id, account_name):
    names = {}
    messages = []

    # get contact Ids and names
    SQL_STATEMENT = '''
       SELECT mri, display_name
         FROM contacts'''

    for row in db.execute(SQL_STATEMENT):
        mri = row[0]
        display_name = row[1]
        names[mri] = display_name

    # get messages
    SQL_STATEMENT = '''
       SELECT m.id,
              m.originalarrivaltime,
              m.author,
              m.content,
              m.sendingstatus,
              m.messagetype,
              c.type,
              c.id
         FROM messages m,
              conversations c
        WHERE c.dbid = m.convdbid'''

    for row in db.execute(SQL_STATEMENT):
        message = pymobius.Data()
        message.id = row[0]
        message.chatname = row[0]  # @deprecated
        message.timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(row[1] // 1000)
        message.sender_id = row[2].split(':', 1)[1]
        message.sender_name = names.get(row[2])
        message.status = row[4]
        message.raw_text = (row[3] or '').rstrip()
        message.type = MESSAGE_TYPE_B.get(row[5], 'Unknown type (%d)' % row[5])

        if row[5] not in MESSAGE_TYPE_B:
            mobius.core.logf('DEV app.skype: unknown MESSAGE_TYPE_B %d. Text: %s' % (row[5], message.raw_text))

        # recipients
        if message.sender_id == account_id:
            recipient_id = row[7]
            recipient_mri = '%s:%s' % (row[6], row[7])
            recipient_name = names.get(recipient_mri)
        else:
            recipient_id = account_id
            recipient_name = account_name

        message.recipients = [(recipient_id, recipient_name)]

        # text
        parser = pymobius.app.skype.message_parser.MessageParser(message.raw_text)
        text = SYSTEM_MESSAGES.get(message.type)

        if text:
            text = text.replace('_', ' ')
            text = text.capitalize()
            parser.add_element({'type': 'system', 'text': text})

        if message.type == 'PopCard':
            message.text = parser.parse_popcard()

        else:
            message.text = parser.parse()

        # add message
        messages.append(message)

    return messages, names
