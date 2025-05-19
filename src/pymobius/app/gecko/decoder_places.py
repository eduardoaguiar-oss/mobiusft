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
import traceback

import mobius
import pymobius
import pymobius.app.gecko


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Load data from places.sqlite file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(f):
    data = pymobius.Data()
    data.history = []
    data.bookmarks = []

    try:
        path = pymobius.app.gecko.retrieve_db_file(f)

        if path:
            db = sqlite3.connect(path)
            decode_places(db, data, f)
            db.close()

            os.remove(path)
            f.set_handled()

    except Exception as e:
        mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # return data object
    return data


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from places.sqlite table
# @see https://developer.mozilla.org/en-US/docs/Mozilla/Tech/Places/Database
# Tested versions: 14.0.1, 45.0.1, 53.0.2, 59.0.2
#
# moz_places: This is the main table of URIs and is managed by the history service
# (see also History service design). Any time a Places component wants to reference
# a URL, whether visited or not, it refers to this table. Each entry has an optional
# reference to the moz_favicon table to identify the favicon of the page. No two
# entries may have the same value in the url column.
#
# moz_historyvisits: One entry in this table is created each time you visit a page.
# It contains the date, referrer, and other information specific to that visit. It
# contains a reference to the moz_places table which contains the URL and other global
# statistics.
#
# moz_bookmarks: This table contains bookmarks, folders, separators and tags, and
# defines the hierarchy. The hierarchy is defined via the parent column, which points
# to the moz_bookmarks record which is the parent. The position column numbers each
# of the peers beneath a given parent starting with 0 and incrementing higher with
# each addition. The fk column provides the id number of the corresponding record
# in moz_places.
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_places(db, data, f):
    evidence_source = pymobius.get_evidence_source_from_file(f)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # retrieve URL history
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    cursor = db.cursor()

    stmt = '''
       SELECT p.id,
              p.url,
              p.title,
              p.typed,
              h.visit_date,
              h.visit_type
         FROM moz_places p, moz_historyvisits h
        WHERE h.place_id = p.id
     ORDER BY h.visit_date'''

    for row in cursor.execute(stmt):
        entry = pymobius.Data()
        entry.id = row[0]
        entry.url = row[1]
        entry.title = row[2]
        entry.typed = row[3] == 1
        entry.timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(row[4] // 1000000)
        entry.visit_type = row[5]
        entry.evidence_source = evidence_source

        data.history.append(entry)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # retrieve bookmark folders
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    cursor = db.cursor()

    stmt = '''
       SELECT id,
              title
         FROM moz_bookmarks b
        WHERE type = 2'''

    folders = dict(cursor.execute(stmt))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # retrieve bookmarks
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    cursor = db.cursor()

    stmt = '''
       SELECT b.id,
              b.parent,
              b.title,
              b.dateAdded,
              b.lastModified,
              p.url
         FROM moz_bookmarks b,
              moz_places p
        WHERE b.fk = p.id
          AND b.type = 1
     ORDER BY b.id'''

    for row in cursor.execute(stmt):
        b = pymobius.Data()
        b.id = row[0]
        b.folder = folders.get(row[1])
        b.name = row[2]
        b.creation_time = mobius.core.datetime.new_datetime_from_unix_timestamp((row[3] or 0) // 1000000)
        b.last_modification_time = mobius.core.datetime.new_datetime_from_unix_timestamp((row[4] or 0) // 1000000)
        b.url = row[5]
        b.path = f.path
        b.evidence_source = evidence_source

        data.bookmarks.append(b)
