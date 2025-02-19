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


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Handle UserAccount evidence
# @todo Photos,ServerAddress
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def handler(e):
    metadata = dict(e.metadata)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # create evidence
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence = pymobius.Data()
    evidence.type = 'user-account'
    account_type = (metadata.get('ServiceType') or
                    metadata.get('ServiceIdentifier') or
                    metadata.get('Source') or
                    'Generic')

    if account_type.startswith('android://') and '@' in account_type:
        account_type = account_type.rsplit('@', 2)[1]
        account_type = '.'.join(account_type.split('.')[::-1])

    evidence.attrs = {
        'account_type': account_type,
        'id': metadata.get('Username'),
        'name': metadata.get('Name'),
        'evidence_source': f"UFDR evidence #{e.id}",
    }

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # password
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    password = metadata.get('Password')

    if password is not None:
        evidence.attrs['password'] = password
        evidence.attrs['password_found'] = 'yes'
    else:
        evidence.attrs['password_found'] = 'no'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # metadata
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence.metadata = mobius.pod.map()
    evidence.metadata.set('evidence-id', e.id)
    evidence.metadata.set('source-idx', e.source_index)
    evidence.metadata.set('extraction-id', e.extraction_id)
    evidence.metadata.set('deleted-state', e.deleted_state)
    evidence.metadata.set('decoding-confidence', e.decoding_confidence)
    evidence.metadata.set('creation-time', metadata.get('TimeCreated'))
    evidence.metadata.set('source', metadata.get('Source'))
    evidence.metadata.set('service-identifier', metadata.get('ServiceIdentifier'))
    evidence.metadata.set('user-mapping', metadata.get('UserMapping'))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Additional info
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for info in metadata.get('AdditionalInfo', []):
        info_metadata = dict(info.metadata)
        key = info_metadata.get('Key')
        value = info_metadata.get('Value')
        evidence.metadata.set(key, value)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Entries
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    accounts = set()
    emails = set()
    phones = set()
    user_id = None
    web_addresses = set()

    for entry in metadata.get('Entries', []):
        entry_metadata = dict(entry.metadata)
        value = f"{entry_metadata.get('Value')} ({entry_metadata.get('Category') or '-'})"

        if entry.type == 'UserID':
            accounts.add(value)
            if not user_id:
                user_id = entry_metadata.get('Value')

        elif entry.type == 'EmailAddress':
            emails.add(value)

        elif entry.type == 'PhoneNumber':
            phones.add(value)

        elif entry.type == 'WebAddress':
            web_addresses.add(value)

        else:
            mobius.core.logf(f"DEV unhandled user_account.entry: {entry.type}. Values found: {entry_metadata}")

    if user_id:
        evidence.attrs['id'] = user_id

    evidence.attrs['accounts'] = list(sorted(accounts))
    evidence.attrs['emails'] = list(sorted(emails))
    evidence.attrs['phones'] = list(sorted(phones))
    evidence.attrs['web_addresses'] = list(sorted(web_addresses))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # entries
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    entries = metadata.get('Entries', [])

    for entry in entries:
        entry_metadata = dict(entry.metadata)
        key = f"{entry_metadata.get('Domain')}/{entry_metadata.get('Category')}"
        value = entry_metadata.get('Value')
        evidence.metadata.set(key, value)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Addresses
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    addresses = set()

    for entry in metadata.get('Addresses', []):
        entry_metadata = dict(entry.metadata)
        address = entry_metadata.get('Street1')

        if address:
            addresses.add(address)
        else:
            mobius.core.logf(f"DEV unhandled user_account.addresses: {entry_metadata}")

    evidence.attrs['addresses'] = list(sorted(addresses))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Notes
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    notes = set()

    for value in metadata.get('Notes', []):
        notes.add(value)

    evidence.attrs['notes'] = list(sorted(notes))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Organizations
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    organizations = set()

    for entry in metadata.get('Organizations', []):
        entry_metadata = dict(entry.metadata)
        value = entry_metadata.get('Name')

        if value:
            organizations.add(value)

    evidence.attrs['organizations'] = list(sorted(organizations))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Return evidence
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    yield evidence
