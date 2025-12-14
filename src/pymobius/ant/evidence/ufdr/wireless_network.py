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


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Handle WirelessNetwork evidence
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def handler(e):
    metadata = dict(e.metadata)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Wireless connection
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    timestamps = {metadata.get('TimeStamp'),
                  metadata.get('LastAutoConnection'),
                  metadata.get('LastConnection')
                  }
    timestamps.discard(None)

    for timestamp in timestamps:
        evidence = pymobius.Data()
        evidence.type = 'wireless-connection'

        evidence.attrs = {
            'timestamp': timestamp,
            'bssid': metadata.get('BSSId'),
            'ssid': metadata.get('SSId'),
            'evidence_source': f"UFDR evidence #{e.id}",
        }

        evidence.metadata = mobius.core.pod.map()
        evidence.metadata.set('evidence-id', e.id)
        evidence.metadata.set('source-idx', e.source_index)
        evidence.metadata.set('extraction-id', e.extraction_id)
        evidence.metadata.set('deleted-state', e.deleted_state)
        evidence.metadata.set('decoding-confidence', e.decoding_confidence)
        evidence.metadata.set('current-password', metadata.get('Password'))
        evidence.metadata.set('current-security-mode', metadata.get('SecurityMode'))

        yield evidence

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # Wireless network
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence = pymobius.Data()
    evidence.type = 'wireless-network'

    evidence.attrs = {
        'bssid': metadata.get('BSSId'),
        'ssid': metadata.get('SSId'),
        'password': metadata.get('Password'),
        'evidence_source': f"UFDR evidence #{e.id}",
    }

    evidence.metadata = mobius.core.pod.map()
    evidence.metadata.set('evidence-id', e.id)
    evidence.metadata.set('source-idx', e.source_index)
    evidence.metadata.set('extraction-id', e.extraction_id)
    evidence.metadata.set('deleted-state', e.deleted_state)
    evidence.metadata.set('decoding-confidence', e.decoding_confidence)
    evidence.metadata.set('timestamp', metadata.get('TimeStamp'))
    evidence.metadata.set('last-connection', metadata.get('LastConnection'))
    evidence.metadata.set('last-auto-connection', metadata.get('LastAutoConnection'))
    evidence.metadata.set('end-time', metadata.get('EndTime'))
    evidence.metadata.set('security-mode', metadata.get('SecurityMode'))

    yield evidence
