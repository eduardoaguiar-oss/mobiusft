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
# @brief Handle contact evidence
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def handler(e):
    evidence = pymobius.Data()
    evidence.type = 'contact'
    metadata = dict(e.metadata)
    accounts = set()
    emails = set()
    phones = set()
    user_id = None
    web_addresses = set()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Main attributes
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence.attrs = {
        'name': metadata.get('Name'),
        'username': metadata.get('Account'),
        'evidence_source': f"UFDR evidence #{e.id}",
    }

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Set metadata
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence.metadata = mobius.pod.map()
    evidence.metadata.set('evidence-id', e.id)
    evidence.metadata.set('source-idx', e.source_index)
    evidence.metadata.set('extraction-id', e.extraction_id)
    evidence.metadata.set('deleted-state', e.deleted_state)
    evidence.metadata.set('decoding-confidence', e.decoding_confidence)
    evidence.metadata.set('time-created', metadata.get('TimeCreated'))
    evidence.metadata.set('time-modified', metadata.get('TimeModified'))
    evidence.metadata.set('contact-type', metadata.get('Type'))
    evidence.metadata.set('source', metadata.get('Source'))
    evidence.metadata.set('group', metadata.get('Group'))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Additional Info
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for entry in metadata.get('AdditionalInfo', []):
        entry_metadata = dict(entry.metadata)
        key = entry_metadata.get('Key')
        value = entry_metadata.get('Value')

        if key:
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
            mobius.core.logf(f"DEV unhandled contact.addresses: {entry_metadata}")

    evidence.attrs['addresses'] = list(sorted(addresses))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Entries
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
            mobius.core.logf(f"DEV unhandled contact.entry: {entry.type}. Values found: {entry_metadata}")

    if user_id:
        evidence.attrs['id'] = user_id

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
    # Photos
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for entry in metadata.get('Photos', []):
        entry_metadata = dict(entry.metadata)
        if entry_metadata:
            mobius.core.logf(f"DEV unhandled contact.photos: {entry_metadata}")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # TimesContacted
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    times_contacted = set()

    t = metadata.get('TimeContacted')
    if t:
        times_contacted.add(t)

    for entry in metadata.get('TimesContacted') or []:
        entry_metadata = dict(entry.metadata)
        mobius.core.logf(f"DEV unhandled contact.timescontacted: {entry_metadata}")

    evidence.metadata.set('times-contacted', list(sorted(times_contacted)))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Fill attributes and return evidence
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence.attrs['accounts'] = list(sorted(accounts))
    evidence.attrs['emails'] = list(sorted(emails))
    evidence.attrs['phones'] = list(sorted(phones))
    evidence.attrs['web_addresses'] = list(sorted(web_addresses))

    yield evidence
