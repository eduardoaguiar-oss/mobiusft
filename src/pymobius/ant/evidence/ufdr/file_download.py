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
# @brief Handle FileDownload evidence
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def handler(e):
    metadata = dict(e.metadata)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # file info
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    filename = None
    file_id = None

    file_info = metadata.get('File')
    if file_info:
        file_id = file_info.id
        filename = file_info.name

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # create evidence
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence = pymobius.Data()
    evidence.type = 'received-file'

    evidence.attrs = {
        'path': metadata.get('TargetPath'),
        'timestamp': metadata.get('StartTime'),
        'app_name': metadata.get('Source'),
        'filename': filename,
        'evidence_source': f"UFDR evidence #{e.id}",
    }

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # set metadata
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence.metadata = mobius.core.pod.map()
    evidence.metadata.set('evidence-id', e.id)
    evidence.metadata.set('source-idx', e.source_index)
    evidence.metadata.set('extraction-id', e.extraction_id)
    evidence.metadata.set('deleted-state', e.deleted_state)
    evidence.metadata.set('decoding-confidence', e.decoding_confidence)
    evidence.metadata.set('url', metadata.get('Url'))
    evidence.metadata.set('file-size', metadata.get('FileSize'))
    evidence.metadata.set('file-id', file_id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # additional info
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for info in metadata.get('AdditionalInfo', []):
        info_metadata = dict(info.metadata)
        key = info_metadata.get('Key')
        value = info_metadata.get('Value')
        evidence.metadata.set(key, value)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # remaining metadata
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    evidence.metadata.set('start-time', metadata.get('StartTime'))
    evidence.metadata.set('end-time', metadata.get('EndTime'))
    evidence.metadata.set('bytes-received', metadata.get('BytesReceived'))
    evidence.metadata.set('download-state', metadata.get('DownloadState'))
    evidence.metadata.set('last-accessed', metadata.get('LastAccessed'))
    evidence.metadata.set('service-identifier', metadata.get('ServiceIdentifier'))
    evidence.metadata.set('user-mapping', metadata.get('UserMapping'))

    yield evidence
