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
import pymobius.app.chromium
import sqlite3


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Load data from Login Data file
# Versions tested: 1,5,7,12,13,16-19,21-22,24-29,31-34,40
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(path, f):
    # create data object
    data = pymobius.Data()
    data.stored_passwords = []

    # get meta version
    db = sqlite3.connect(path)
    data.version = pymobius.app.chromium.get_meta_version(db)

    if data.version:
        mobius.core.logf(f'INF app.chromium: Login Data.version = {data.version}')
        decode_login_data(db, data, f)

    db.close()

    # return data object
    return data


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from logins table
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_login_data(db, data, f):
    stmt = '''SELECT _rowid_, origin_url, action_url, username_value,
                   password_value, blacklisted_by_user, signon_realm,
                   date_created'''

    # no: 1, yes: 5 (GAP)
    stmt += pymobius.app.chromium.add_column(data.version > 4, 'times_used')

    # no: 24, yes: 25 (OK)
    stmt += pymobius.app.chromium.add_column(data.version > 24, 'date_last_used')

    # no: 29, yes: 31 (GAP)
    stmt += pymobius.app.chromium.add_column(data.version > 30, 'date_password_modified')

    stmt += ' FROM logins'

    cursor = db.cursor()

    for row in cursor.execute(stmt):
        entry = pymobius.Data()
        entry.id = row[0]
        entry.origin_url = row[1]
        entry.action_url = row[2]
        entry.username = row[3]
        entry.password = row[4]
        entry.has_password = row[5] == 0
        entry.signon_realm = row[6]
        entry.date_created = pymobius.app.chromium.get_datetime(row[7])
        entry.times_used = row[8]
        entry.date_last_used = pymobius.app.chromium.get_datetime(row[9])
        entry.date_password_modified = pymobius.app.chromium.get_datetime(row[10])
        entry.source = 'File %s' % mobius.core.io.to_win_path(f.path)
        data.stored_passwords.append(entry)
