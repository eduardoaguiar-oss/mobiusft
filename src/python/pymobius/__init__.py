# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import re

import mobius
import pymobius.mediator

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mediator = pymobius.mediator.Mediator()  # global mediator
DEBUG = False
CAMELCASE = re.compile(r'(?<!^)(?=[A-Z])')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Generic data holder
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Data(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Create dict from object attributes
    # @return Dict
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def to_dict(self):
        attrs = {}

        for name, value in self.__dict__.items():
            if not name.startswith('__') and not callable(value):
                attrs[name] = value

        return attrs

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set object attributes from dict values
    # @para d Dict
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def from_dict(self, d):
        for name, value in d.items():
            setattr(self, name, value)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Generate data dump
# @param s data
# @param indent Indentation size
# @return data dump in hex and chars
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def dump(s, indent=0):
    i = 0
    l = len(s)
    text = ''
    text_indent = ' ' * indent

    while i < l:
        data = s[i: min(i + 16, l)]

        if i > 0:
            text += '\n'

        text += f'{text_indent}{i:08x}  '
        text += mobius.encoder.hexstring(data[:8], ' ')
        text += '  '
        text += mobius.encoder.hexstring(data[8:], ' ')
        text += '   ' * (16 - len(data))
        text += '  '
        text += ''.join(chr(c) if 31 < c < 127 else '.' for c in data)
        i += 16

    return text


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Convert generic ID into name
# @param i ID
# @return Name
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def id_to_name(i):
    v = i.replace('_', ' ').replace('-', ' ')

    if v and v[0].islower():
        v = v.capitalize()

    return v


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Convert camelCase ID to generic ID
# @param i ID
# @return Name
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def camel_case_id_to_id(i, sep='-'):
    return CAMELCASE.sub(sep, i).lower()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Convert value to bytes
# @param value
# @return Value as byte sequence
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def to_bytes(value):
    if value is None:
        return bytes()

    elif isinstance(value, bytes):
        return value

    elif isinstance(value, str):
        return value.encode('utf-8')

    return bytes(value)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Convert value to string
# @param value
# @return value as string
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def to_string(value):
    if value is None:
        return ''

    elif isinstance(value, str):
        return value

    elif isinstance(value, bytes):
        return value.decode('utf-8')

    else:
        return str(value)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get evidence source from file
# @param f File object
# @return Evidence source string
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_evidence_source_from_file(f):
    return f"File {f.path} (i-node: {f.inode})"


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Add file to item's kff ignored file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def set_handled(item, f):
    # create file reader
    if not f:
        return

    reader = f.new_reader()
    if not reader:
        return

    # calculate hash sha2-512
    h = mobius.crypt.hash('sha2-512')
    data = reader.read(65536)

    while data:
        h.update(data)
        data = reader.read(65536)

    # add to kff file
    case = item.case
    path = case.create_path(f'hashset/{item.uid:04d}-handled.ignore')
    fp = open(path, 'a')
    fp.write(f'{h.get_hex_digest()}\n')
    fp.close()
