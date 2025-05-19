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
import base64

import mobius
import pymobius


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Handle Password evidence
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def handler(e):
    metadata = dict(e.metadata)
    data = metadata.get('Data')

    if not data:
        return

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Main attributes
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence = pymobius.Data()
    evidence.attrs = {
        'account': metadata.get('Account'),
        'evidence_source': f"UFDR evidence #{e.id}",
    }

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Set metadata
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence.metadata = mobius.core.pod.map()
    evidence.metadata.set('evidence-id', e.id)
    evidence.metadata.set('source-idx', e.source_index)
    evidence.metadata.set('extraction-id', e.extraction_id)
    evidence.metadata.set('deleted-state', e.deleted_state)
    evidence.metadata.set('decoding-confidence', e.decoding_confidence)
    evidence.metadata.set('source', metadata.get('Source'))
    evidence.metadata.set('label', metadata.get('Label'))
    evidence.metadata.set('service', metadata.get('Service'))
    evidence.metadata.set('service-identifier', metadata.get('ServiceIdentifier'))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Handle passwords
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    pwd_type = metadata.get('Type')

    if pwd_type == 'Default':
        evidence.type = 'password'
        value = base64.b64decode(data)

        evidence.attrs['value'] = value.decode('utf-8')
        evidence.attrs['description'] = metadata.get('Label')
        evidence.attrs['password_type'] = (metadata.get('Label') or
                                       metadata.get('Service') or
                                       metadata.get('ServiceIdentifier') or
                                       'Default')
        yield evidence

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Handle encryption-keys
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    elif pwd_type == 'Key':
        evidence.type = 'encryption-key'
        value = base64.b64decode(data)

        evidence.attrs['value'] = value
        evidence.attrs['description'] = metadata.get('Label')
        evidence.attrs['key_type'] = (metadata.get('Label') or
                                  metadata.get('Service') or
                                  metadata.get('ServiceIdentifier') or
                                  metadata.get('Source') or
                                  'Generic')
        evidence.metadata.set('access-group', metadata.get('AccessGroup'))
        yield evidence

    else:
        mobius.core.logf(f"DEV unhandled password type: {pwd_type}. Values: {metadata}")