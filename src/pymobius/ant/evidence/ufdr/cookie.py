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
# @brief Handle Cookie evidence
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def handler(e):
    metadata = dict(e.metadata)

    last_update_time = None
    if metadata.get('CreationTime') == metadata.get('LastAccessTime'):
        last_update_time = metadata.get('CreationTime')

    evidence = pymobius.Data()
    evidence.type = 'cookie'

    evidence.attrs = {
        'name': metadata.get('Name'),
        'value': metadata.get('Value'),
        'is_encrypted': False,
        'is_deleted': bool(e.deleted_state == "Deleted"),
        'domain': metadata.get('Domain'),
        'creation_time': metadata.get('CreationTime'),
        'last_access_time': metadata.get('LastAccessTime'),
        'expiration_time': metadata.get('Expiry'),
        'last_update_time': last_update_time,
        'username': None,
        'app_name': metadata.get('Source'),
        'evidence_source': f"UFDR evidence #{e.id}",
    }

    evidence.metadata = mobius.core.pod.map()
    evidence.metadata.set('evidence-id', e.id)
    evidence.metadata.set('source-idx', e.source_index)
    evidence.metadata.set('extraction-id', e.extraction_id)
    evidence.metadata.set('deleted-state', e.deleted_state)
    evidence.metadata.set('decoding-confidence', e.decoding_confidence)
    evidence.metadata.set('related-application', metadata.get('RelatedApplication'))
    evidence.metadata.set('path', metadata.get('Path'))
    evidence.metadata.set('service-identifier', metadata.get('ServiceIdentifier'))
    evidence.metadata.set('user-mapping', metadata.get('UserMapping'))

    yield evidence
