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
import os
import sqlite3

import mobius
import pymobius.app.gecko


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Load data from formhistory.sqlite file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(f):
    path = pymobius.app.gecko.retrieve_db_file(f)
    form_history = []

    if path:
        db = sqlite3.connect(path)
        form_history = decode_form_history(db)
        db.close()

        os.remove(path)
        f.set_handled()

    return form_history


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from formhistory.sqlite table
# Tested versions:
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_form_history(db):
    # check moz_formhistory table schema
    stmt = '''SELECT *
              FROM moz_formhistory
              LIMIT 1'''

    cursor = db.cursor()
    cursor.execute(stmt)
    cols = set(d[0] for d in cursor.description)

    # call function according to columns available
    if len(cols) == 3:  # cols = (id, fieldname, value)
        return decode_form_history_01(db)

    else:
        return decode_form_history_14(db)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from formhistory.sqlite table
# Tested versions:
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_form_history_01(db):
    form_history = []

    stmt = '''
       SELECT id,
              fieldname,
              value
         FROM moz_formhistory'''

    cursor = db.cursor()

    for row in cursor.execute(stmt):
        entry = pymobius.Data()
        entry.id = row[0]
        entry.fieldname = row[1]
        entry.value = row[2]
        entry.use_count = None
        entry.first_used_time = None
        entry.last_used_time = None

        form_history.append(entry)

    return form_history


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from formhistory.sqlite table
# Tested versions: 14.0.1, 50.0.2, 53.0.2, 59.0.2
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_form_history_14(db):
    form_history = []

    stmt = '''
       SELECT id,
              fieldname,
              value,
              timesused,
              firstused,
              lastused
         FROM moz_formhistory'''

    cursor = db.cursor()

    for row in cursor.execute(stmt):
        entry = pymobius.Data()
        entry.id = row[0]
        entry.fieldname = row[1]
        entry.value = row[2]
        entry.use_count = row[3]
        entry.first_used_time = mobius.datetime.new_datetime_from_unix_timestamp(row[4] // 1000000)
        entry.last_used_time = mobius.datetime.new_datetime_from_unix_timestamp(row[5] // 1000000)

        form_history.append(entry)

    return form_history
