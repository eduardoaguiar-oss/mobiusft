# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import mobius

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Extension metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
EXTENSION_ID = 'vfs-item-attributes'
EXTENSION_NAME = 'VFS item attributes'
EXTENSION_AUTHOR = 'Eduardo Aguiar'
EXTENSION_VERSION = '1.1'
EXTENSION_DESCRIPTION = 'Fill item attributes with VFS metadata'

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Associate disk metadata with case item attributes
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ATTR_MAP = {
    'file': [('name', 'name'),
             ('size', 'size'),
             ('access_time', 'access_time'),
             ('data_mtime', 'data_mtime'),
             ('metadata_mtime', 'metadata_mtime'),
             ('user', 'user'),
             ('group', 'group')
             ],
    'harddisk': [('serial', 'serial'),
                 ('firmware', 'firmware'),
                 ('vendor', 'vendor'),
                 ('model', 'model_name'),
                 ('wwn', 'wwn'),
                 ('sectors', 'lba'),
                 ('sector_size', 'sector_size'),
                 ('size', 'real_capacity'),
                 ('description', 'additional_info'),
                 ('evidence_description', 'additional_info'),
                 ('additional_info', 'additional_info'),
                 ('part_id', 'part_id'),
                 ('drive_vendor', 'vendor'),
                 ('drive_serial_number', 'serial'),
                 ('drive_model', 'model_name')
                 ],
    'memorycard': [('serial', 'serial'),
                   ('firmware', 'firmware_version'),
                   ('vendor', 'vendor'),
                   ('model', 'model'),
                   ('sectors', 'lba'),
                   ('sector_size', 'sector_size'),
                   ('size', 'real_capacity'),
                   ('description', 'additional_info'),
                   ('evidence_description', 'additional_info'),
                   ('additional_info', 'additional_info')
                   ],
    'pendrive': [('serial', 'serial'),
                 ('firmware', 'firmware_version'),
                 ('vendor', 'vendor'),
                 ('model', 'model'),
                 ('sectors', 'lba'),
                 ('sector_size', 'sector_size'),
                 ('size', 'real_capacity'),
                 ('description', 'additional_info'),
                 ('evidence_description', 'additional_info'),
                 ('additional_info', 'additional_info'),
                 ('drive_vendor', 'vendor'),
                 ('drive_serial_number', 'serial'),
                 ('drive_model', 'model_name')
                 ],
    'ssd': [('serial', 'serial'),
            ('firmware', 'firmware'),
            ('vendor', 'vendor'),
            ('model', 'model_name'),
            ('wwn', 'wwn'),
            ('sectors', 'lba'),
            ('sector_size', 'sector_size'),
            ('size', 'real_capacity'),
            ('description', 'additional_info'),
            ('evidence_description', 'additional_info'),
            ('additional_info', 'additional_info'),
            ('part_id', 'part_id'),
            ('drive_vendor', 'vendor'),
            ('drive_serial_number', 'serial'),
            ('drive_model', 'model_name')
            ],
}

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Format specific attributes
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
FORMAT = {
    'size': lambda v: '{:,}'.format(v).replace(',', '.') + ' bytes',
    'sector_size': lambda v: '{:,}'.format(v).replace(',', '.') + ' bytes',
    'sectors': lambda v: '{:,}'.format(v).replace(',', '.')
}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Set item attributes using disk attributes
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def set_item_attributes(item, attributes):
    d_metadata = attributes.to_python()

    # generate part_id
    model = d_metadata.get('model') or d_metadata.get('drive_model')
    if model:
        part_id = model.split(' ')[-1].split('-')[0]
        d_metadata['part_id'] = part_id

    # fill attributes
    for d_attr_id, attr_id in ATTR_MAP.get(item.category, []):
        value = d_metadata.get(d_attr_id)

        fmt = FORMAT.get(d_attr_id)
        if fmt:
            value = fmt(value)

        if value is not None and str(value):
            item.set_attribute(attr_id, str(value))

    item.expand_masks()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Event handlers
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
event_uid_1 = -1


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    global event_uid_1
    event_uid_1 = mobius.core.subscribe('datasource-modified', cb_datasource_modified)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    global event_uid_1
    mobius.core.unsubscribe(event_uid_1)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Handle datasource-modified event
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def cb_datasource_modified(item, datasource):
    if datasource and datasource.get_type() == "vfs":
        vfs = datasource.get_vfs()
        disks = vfs.get_disks()

        if len(disks) == 1:
            set_item_attributes(item, disks[0].get_attributes())
