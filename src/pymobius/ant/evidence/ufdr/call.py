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
# @brief Handle Call evidence
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def handler(e):
    metadata = dict(e.metadata)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # sources and destinations
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    sources = []
    destinations = []

    for party in metadata.get('Parties', []):
        party_data = common.get_party_data(party)

        if party_data.role == 'From':
            sources.append(party_data.label)

        else:
            destinations.append(party_data.label)

    direction = metadata.get('Direction')

    if not destinations and direction == 'Incoming':
        destinations.append('<this-phone>')

    if not sources and direction == 'Outgoing':
        sources.append('<this-phone>')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # create evidence
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence = pymobius.Data()
    evidence.type = 'call'

    evidence.attrs = {
        'timestamp': metadata.get('TimeStamp'),
        'duration': metadata.get('Duration'),
        'app': metadata.get('Source'),
        'source': ','.join(sources),
        'destination': destinations,
        'username': metadata.get('Account'),
        'evidence_source': f"UFDR evidence #{e.id}"
    }

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # metadata
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence.metadata = mobius.core.pod.map()
    evidence.metadata.set('evidence-id', e.id)
    evidence.metadata.set('source-idx', e.source_index)
    evidence.metadata.set('extraction-id', e.extraction_id)
    evidence.metadata.set('deleted-state', e.deleted_state)
    evidence.metadata.set('decoding-confidence', e.decoding_confidence)
    evidence.metadata.set('type', metadata.get('Type'))
    evidence.metadata.set('direction', metadata.get('Direction'))
    evidence.metadata.set('status', metadata.get('Status'))
    evidence.metadata.set('is-video-call', metadata.get('VideoCall'))
    evidence.metadata.set('country-code', metadata.get('CountryCode'))
    evidence.metadata.set('network-code', metadata.get('NetworkCode'))
    evidence.metadata.set('network-name', metadata.get('NetworkName'))
    evidence.metadata.set('service-identifier', metadata.get('ServiceIdentifier'))
    evidence.metadata.set('user-mapping', metadata.get('UserMapping'))

    yield evidence
