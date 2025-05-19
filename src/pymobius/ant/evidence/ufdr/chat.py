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
import mobius
import pymobius

from . import common


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Handle Chat evidence
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def handler(e):
    metadata = dict(e.metadata)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # participants
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    participants = {}

    for party in metadata.get('Participants', []):
        party_data = common.get_party_data(party)
        participants[party_data.party_id] = party_data

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # chat messages
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for idx, message in enumerate(metadata.get('Messages', []), 1):
        message_metadata = dict(message.metadata)

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # message extra data
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        message_extra_data = message_metadata.get('MessageExtraData', [])
        text_type = 'text'
        tags = set()

        for me_data in message_extra_data:
            d = dict(me_data.metadata)
            label = d.get('Label')

            if label == 'Forwarded':
                party = d.get('OriginalSender')
                message_metadata['OriginalSender'] = common.get_party_data(party).label
                message_metadata['IsForwarded'] = True

            elif label == 'System':
                text_type = 'system'

            elif label in ('SelfDestruct', 'Edited'):
                tags.add(label)

            else:
                mobius.core.logf(f"DEV unhandled message extra data: {d}")

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # sender and recipients
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        from_is_phone_owner = False
        to_is_phone_owner = False
        from_party = message_metadata.get('From')
        sender = None
        recipients = []

        if from_party:
            from_party_data = common.get_party_data(from_party)
            sender_id = from_party_data.party_id
            sender = from_party_data.label
            from_is_phone_owner = from_party_data.is_phone_owner

            recipients = [party_data.label
                          for party_data in participants.values()
                          if party_data.party_id != sender_id]

            to_is_phone_owner = any(party_data.is_phone_owner
                                    for party_data in participants.values()
                                    if party_data.party_id != sender_id)

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Evidence: Chat Message
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        body = message_metadata.get('Body')
        if body:
            evidence = pymobius.Data()
            evidence.type = 'chat-message'

            evidence.attrs = {
                'timestamp': message_metadata.get('TimeStamp'),
                'sender': sender,
                'recipients': recipients,
                'text': [{'type': text_type, 'text': body}],
                'app': message_metadata.get('SourceApplication') or message_metadata.get('Source'),
                'username': metadata.get('Account'),
            }

            evidence.metadata = mobius.core.pod.map()
            evidence.metadata.set('evidence-id', message.id)
            evidence.metadata.set('source-idx', message.source_index)
            evidence.metadata.set('extraction-id', message.extraction_id)
            evidence.metadata.set('deleted-state', message.deleted_state)
            evidence.metadata.set('decoding-confidence', message.decoding_confidence)
            evidence.metadata.set('message-idx', idx)
            evidence.metadata.set('message-identifier', message_metadata.get('Identifier'))
            evidence.metadata.set('status', message_metadata.get('Status'))
            evidence.metadata.set('is-forwarded', bool(message_metadata.get('IsForwarded')))
            evidence.metadata.set('original-sender', message_metadata.get('OriginalSender'))
            evidence.metadata.set('tags', ', '.join(sorted(tags)))

            yield evidence

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Get message attachments
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        attachments : list = message_metadata.get('Attachments', [])

        attachment = message_metadata.get('Attachment')
        if attachment:
            attachments.append(attachment)

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Process message attachments
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for attachment in attachments:

            # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            # Evidence: Sent file
            # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            if from_is_phone_owner:
                evidence = handle_attachment(attachment, metadata, message)
                evidence.type = 'sent-file'
                evidence.metadata.set('message-idx', idx)
                evidence.metadata.set('sender', sender)
                evidence.metadata.set('recipients', '\n'.join(sorted(recipients)))
                yield evidence

            # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            # Evidence: Received file
            # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            if to_is_phone_owner:
                evidence = handle_attachment(attachment, metadata, message)
                evidence.type = 'received-file'
                evidence.metadata.set('message-idx', idx)
                evidence.metadata.set('sender', sender)
                evidence.metadata.set('recipients', '\n'.join(sorted(recipients)))
                yield evidence


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Handle attachment
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def handle_attachment(attachment, metadata, message):
    attachment_metadata = dict(attachment.metadata)
    message_metadata = dict(message.metadata)

    evidence = pymobius.Data()

    evidence.attrs = {
        'username': metadata.get('Account'),
        'timestamp': message_metadata.get('TimeStamp'),
        'filename': attachment_metadata.get('Filename'),
        'app_name': message_metadata.get('SourceApplication') or metadata.get('Source'),
        'evidence_source': f"UFDR evidence #{attachment.id}",
    }

    file_id = getattr(attachment, 'file_id', None)
    if file_id:
        evidence.attrs['file_id'] = file_id

    evidence.metadata = mobius.core.pod.map()
    evidence.metadata.set('evidence_id', attachment.id)
    evidence.metadata.set('source_idx', attachment.source_index)
    evidence.metadata.set('extraction_id', attachment.extraction_id)
    evidence.metadata.set('deleted_state', attachment.deleted_state)
    evidence.metadata.set('decoding_confidence', attachment.decoding_confidence)
    evidence.metadata.set('message_identifier', message_metadata.get('Identifier'))
    evidence.metadata.set('url', attachment_metadata.get('URL'))
    evidence.metadata.set('content_type', attachment_metadata.get('ContentType'))
    evidence.metadata.set('attachment_extracted_path',
                          attachment_metadata.get('attachment_extracted_path'))
    evidence.metadata.set('is_forwarded', bool(message_metadata.get('IsForwarded')))
    evidence.metadata.set('original_sender', message_metadata.get('OriginalSender'))

    return evidence
