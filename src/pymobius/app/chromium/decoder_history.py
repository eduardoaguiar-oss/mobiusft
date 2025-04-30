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
import os.path
import sqlite3

import mobius
import pymobius
import pymobius.app.chromium


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Load data from History file
# Versions tested: 20, 22, 28, 29, 32, 33, 36-45, 48, 50-51, 53
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(path):
    # create data object
    data = pymobius.Data()
    data.history = []
    data.downloads = []

    # get meta version
    db = sqlite3.connect(path)
    data.version = pymobius.app.chromium.get_meta_version(db)

    if data.version:
        mobius.core.logf(f'INF app.chromium: History.version = {data.version}')

        decode_history(db, data)
        decode_downloads(db, data)

    db.close()

    # return data object
    return data


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Load URL history
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_history(db, data):
    cursor = db.cursor()

    stmt = '''
       SELECT v.id,
              u.url,
              u.title,
              v.visit_time
         FROM urls u, visits v
        WHERE v.url = u.id
     ORDER BY v.visit_time'''

    for row in cursor.execute(stmt):
        entry = pymobius.Data()
        entry.id = row[0]
        entry.url = row[1]
        entry.title = row[2]
        entry.timestamp = mobius.datetime.new_datetime_from_nt_timestamp(row[3] * 10)

        data.history.append(entry)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Load downloads
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_downloads(db, data):
    if data.version <= 23:
        decode_downloads_20(db, data)

    elif data.version <= 28:
        decode_downloads_28(db, data)

    else:
        decode_downloads_29(db, data)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Load downloads
# Versions tested: 20, 22
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_downloads_20(db, data):
    cursor = db.cursor()

    stmt = '''
       SELECT id,
              full_path,
              url,
              start_time,
              end_time,
              state,
              received_bytes,
              total_bytes
         FROM downloads'''

    for row in cursor.execute(stmt):
        entry = pymobius.Data()
        entry.id = row[0]
        entry.target = row[1]

        if entry.target.endswith('.crdownload'):
            entry.target = entry.target[:-11]

        entry.source = row[2]
        entry.name = os.path.basename(entry.target.replace('\\', '/'))
        entry.start_time = mobius.datetime.new_datetime_from_unix_timestamp(row[3])
        entry.end_time = mobius.datetime.new_datetime_from_unix_timestamp(row[4])
        entry.state = row[5]
        entry.bytes_downloaded = row[6]
        entry.size = row[7]
        entry.referrer = None
        entry.mimetype = None

        data.downloads.append(entry)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Load downloads
# Versions tested: 28
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_downloads_28(db, data):
    cursor = db.cursor()

    stmt = '''
       SELECT d.id,
              d.target_path,
              u.url,
              d.target_path,
              d.start_time,
              d.end_time,
              d.state,
              d.received_bytes,
              d.total_bytes,
              d.referrer
         FROM downloads d,
              downloads_url_chains u
        WHERE d.id = u.id'''

    for row in cursor.execute(stmt):
        entry = pymobius.Data()
        entry.id = len(data.downloads) + 1
        entry.name = os.path.basename(row[1].replace('\\', '/'))
        entry.source = row[2]
        entry.target = row[3]
        entry.start_time = mobius.datetime.new_datetime_from_nt_timestamp(row[4] * 10)
        entry.end_time = mobius.datetime.new_datetime_from_nt_timestamp(row[5] * 10)
        entry.state = row[6]
        entry.bytes_downloaded = row[7]
        entry.size = row[8]
        entry.referrer = row[9]
        entry.mimetype = None

        data.downloads.append(entry)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Load downloads
# Versions tested: 29, 32, 33, 36-45, 48
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_downloads_29(db, data):
    cursor = db.cursor()

    stmt = '''
       SELECT d.id,
              d.target_path,
              u.url,
              d.target_path,
              d.start_time,
              d.end_time,
              d.state,
              d.received_bytes,
              d.total_bytes,
              d.referrer,
              d.mime_type
         FROM downloads d,
              downloads_url_chains u
        WHERE d.id = u.id'''

    for row in cursor.execute(stmt):
        entry = pymobius.Data()
        entry.id = len(data.downloads) + 1
        entry.name = os.path.basename(row[1].replace('\\', '/'))
        entry.source = row[2]
        entry.target = row[3]
        entry.start_time = mobius.datetime.new_datetime_from_nt_timestamp(row[4] * 10)
        entry.end_time = mobius.datetime.new_datetime_from_nt_timestamp(row[5] * 10)
        entry.state = row[6]
        entry.bytes_downloaded = row[7]
        entry.size = row[8]
        entry.referrer = row[9]
        entry.mimetype = row[10]

        data.downloads.append(entry)
