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
import pymobius.app.gecko
import sqlite3
import mobius
import os

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Load data from downloads.sqlite file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode (f):

  # create data object
  data = pymobius.Data ()
  data.downloads = []

  path = pymobius.app.gecko.retrieve_db_file (f)

  if path:
    db = sqlite3.connect (path)
    decode_downloads (db, data)
    db.close ()

    os.remove (path)
    f.set_handled ()

  # return data object
  return data

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from downloads.sqlite table
# Tested versions: 14.0.1
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_downloads (db, data):
  cursor = db.cursor ()

  stmt = '''
       SELECT id,
              name,
              source,
              target,
              starttime,
              endtime,
              state,
              currbytes,
              maxbytes,
              referrer,
              mimetype
         FROM moz_downloads'''

  for row in cursor.execute (stmt):
    url = mobius.io.uri (row[3])

    entry = pymobius.Data ()
    entry.id = row[0]
    entry.name = row[1]
    entry.source = row[2]
    entry.target = url.get_path ().replace ('/', '\\')
    entry.start_time = mobius.datetime.new_datetime_from_unix_timestamp (row[4] // 1000000)
    entry.end_time = mobius.datetime.new_datetime_from_unix_timestamp (row[5] // 1000000)
    entry.state = row[6]
    entry.bytes_downloaded = row[7]
    entry.size = row[8]
    entry.referrer = row[9]
    entry.mimetype = row[10]

    data.downloads.append (entry)
