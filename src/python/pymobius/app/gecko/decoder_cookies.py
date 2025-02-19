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
import datetime
import os
import sqlite3

import mobius
import pymobius
import pymobius.app.gecko


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Load data from cookies.sqlite file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(f):
    data = pymobius.Data()
    data.cookies = []

    path = pymobius.app.gecko.retrieve_db_file(f)

    if path:
        db = sqlite3.connect(path)
        db.text_factory = str
        decode_cookies(db, data, f)
        db.close()

        os.remove(path)
        f.set_handled()

    # return data object
    return data


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from cookies.sqlite table
# Tested versions: 14.0.1, 45.0.1, 50.0.2, 53.0.2, 59.0.2
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_cookies(db, data, f):
    evidence_source = pymobius.get_evidence_source_from_file(f)
    cursor = db.cursor()

    # check if column "basedomain" exists
    column_names = set([row[1].lower() for row in cursor.execute('PRAGMA table_info (moz_cookies)')])

    if 'basedomain' in column_names:
        stmt = '''
       SELECT id,
              basedomain,
              name,
              value,
              lastaccessed,
              creationtime,
              expiry
         FROM moz_cookies'''

    else:
        stmt = '''
       SELECT id,
              host,
              name,
              value,
              lastaccessed,
              creationtime,
              expiry
         FROM moz_cookies'''

    # do query
    cursor = db.cursor()

    # process rows
    for row in cursor.execute(stmt):
        entry = pymobius.Data()
        entry.id = row[0]
        entry.name = row[2]
        entry.value = bytes(row[3], 'utf-8')
        entry.domain = row[1] or ''
        entry.evidence_source = evidence_source

        if row[4]:
            entry.last_access_time = mobius.datetime.new_datetime_from_unix_timestamp(row[4] // 1000000)
        else:
            entry.last_access_time = None

        if row[5]:
            entry.creation_time = mobius.datetime.new_datetime_from_unix_timestamp(row[5] // 1000000)
        else:
            entry.creation_time = None

        if row[6]:
            entry.expiration_time = mobius.datetime.new_datetime_from_unix_timestamp(row[6])
        else:
            entry.expiration_time = None

        entry.evidence_path = f.path
        entry.is_deleted = f.is_deleted()

        data.cookies.append(entry)
