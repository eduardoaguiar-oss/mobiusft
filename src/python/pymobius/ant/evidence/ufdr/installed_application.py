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
# @brief Handle InstalledApplication evidence
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def handler(e):
    metadata = dict(e.metadata)

    evidence = pymobius.Data()
    evidence.type = 'installed-program'

    evidence.attrs = {
        'name': metadata.get('Name') or metadata.get('Identifier'),
        'version': metadata.get('Version'),
        'description': metadata.get('Description'),
        'evidence_source': f"UFDR evidence #{e.id}",
    }

    evidence.metadata = mobius.pod.map()
    evidence.metadata.set('evidence-id', e.id)
    evidence.metadata.set('source-idx', e.source_index)
    evidence.metadata.set('extraction-id', e.extraction_id)
    evidence.metadata.set('deleted-state', e.deleted_state)
    evidence.metadata.set('decoding-confidence', e.decoding_confidence)
    evidence.metadata.set('decoding-status', metadata.get('DecodingStatus'))
    evidence.metadata.set('app-guid', metadata.get('AppGUID'))
    evidence.metadata.set('categories', ','.join(metadata.get('Categories', [])))
    evidence.metadata.set('copyright', metadata.get('Copyright'))
    evidence.metadata.set('identifier', metadata.get('Identifier'))
    evidence.metadata.set('is-emulatable', metadata.get('IsEmulatable'))
    evidence.metadata.set('last-launched', metadata.get('LastLaunched'))
    evidence.metadata.set('operation-mode', metadata.get('OperationMode'))
    evidence.metadata.set('permissions', ','.join(metadata.get('Permissions', [])))
    evidence.metadata.set('purchase-date', metadata.get('PurchaseDate'))

    yield evidence
