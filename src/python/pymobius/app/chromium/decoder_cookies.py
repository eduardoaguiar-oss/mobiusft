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
import os
import sqlite3
import traceback

import mobius
import pymobius
import pymobius.app.chromium


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Load data from Cookies file
# Tested versions: 4-5, 7-16
# @see Chromium source code (sqlite_persistent_cookie_store.cc)
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(f):
    cookies = []

    try:
        cookies = decode_file(f)

    except Exception as e:
        mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

    return cookies


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from Cookies database
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_file(f):
    cookies = []

    # check if file is valid
    if not f or f.is_reallocated():
        return cookies

    # try to retrieve file
    mobius.core.logf(f'DBG cookie file: {f.path}')
    path = pymobius.app.chromium.retrieve_db_file(f)

    if not path:
        return cookies

    # get meta version
    db = sqlite3.connect(path)
    db.text_factory = str
    version = pymobius.app.chromium.get_meta_version(db)

    if version:
        mobius.core.logf(f"INF Cookies.version = {version}")

        if version < 7:
            cookies = decode_cookies_4(db, f)

        else:
            cookies = decode_cookies_7(db, f, version)

    else:
        mobius.core.logf('WRN Unknown Cookies version')

    db.close()

    # remove temp file
    os.remove(path)
    f.set_handled()

    return cookies


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from cookies table
# Tested versions: 4-5
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_cookies_4(db, f):
    cookies = []
    evidence_source = pymobius.get_evidence_source_from_file(f)
    cursor = db.cursor()

    stmt = '''
       SELECT _rowid_,
              host_key,
              name,
              value,
              last_access_utc,
              creation_utc,
              expires_utc
         FROM cookies'''

    for row in cursor.execute(stmt):
        entry = pymobius.Data()
        entry.id = row[0]
        entry.domain = (row[1] or '').lstrip('.')
        entry.name = row[2]
        entry.value = row[3]
        entry.is_encrypted = False
        entry.last_access_time = mobius.datetime.new_datetime_from_nt_timestamp(row[4] * 10)
        entry.creation_time = mobius.datetime.new_datetime_from_nt_timestamp(row[5] * 10)
        entry.expiration_time = mobius.datetime.new_datetime_from_nt_timestamp(row[6] * 10)
        entry.last_update_time = None
        entry.evidence_source = evidence_source
        entry.is_deleted = f.is_deleted()

        cookies.append(entry)

    return cookies


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from cookies table
# Tested versions: 7-16,18
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_cookies_7(db, f, version):
    cookies = []
    evidence_source = pymobius.get_evidence_source_from_file(f)
    cursor = db.cursor()

    stmt = '''
       SELECT _rowid_,
              host_key,
              name,
              encrypted_value,
              last_access_utc,
              creation_utc,
              expires_utc'''

    stmt += pymobius.app.chromium.add_column(version >= 18, 'last_update_utc')
    stmt += ' FROM cookies'

    for row in cursor.execute(stmt):
        try:
            entry = pymobius.Data()
            entry.id = row[0]
            entry.domain = (row[1] or '').lstrip('.')
            entry.name = row[2]
            entry.value = row[3]
            entry.is_encrypted = True
            entry.last_access_time = pymobius.app.chromium.get_datetime(row[4])
            entry.creation_time = pymobius.app.chromium.get_datetime(row[5])
            entry.expiration_time = pymobius.app.chromium.get_datetime(row[6])
            entry.last_update_time = pymobius.app.chromium.get_datetime(row[7])
            entry.evidence_source = evidence_source
            entry.is_deleted = f.is_deleted()
            cookies.append(entry)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    return cookies
