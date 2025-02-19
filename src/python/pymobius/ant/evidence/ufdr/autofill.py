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
# @brief Handle autofill evidence
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def handler(e):
    metadata = dict(e.metadata)

    evidence = pymobius.Data()
    evidence.type = 'autofill'

    evidence.attrs = {
        'field_name': metadata.get('Key'),
        'value': metadata.get('Value'),
        'app_name': metadata.get('Source'),
        'username': None,
        'evidence_source': f"UFDR evidence #{e.id}"
    }

    evidence.metadata = mobius.pod.map()
    evidence.metadata.set('evidence_id', e.id)
    evidence.metadata.set('source_idx', e.source_index)
    evidence.metadata.set('extraction_id', e.extraction_id)
    evidence.metadata.set('deleted_state', e.deleted_state)
    evidence.metadata.set('decoding_confidence', e.decoding_confidence)
    evidence.metadata.set('account', metadata.get('Account'))
    evidence.metadata.set('timestamp', metadata.get('Timestamp'))
    evidence.metadata.set('last_used_date', metadata.get('LastUsedDate'))
    evidence.metadata.set('service_identifier', metadata.get('ServiceIdentifier'))
    evidence.metadata.set('user_mapping', metadata.get('UserMapping'))

    yield evidence
