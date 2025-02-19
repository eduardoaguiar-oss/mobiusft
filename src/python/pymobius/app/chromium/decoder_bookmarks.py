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
import json
import traceback

import mobius
import pymobius


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(f):
    bookmarks = []

    try:
        bookmarks = decode_file(f)

    except Exception as e:
        mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

    return bookmarks


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Load bookmarks from Bookmarks file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_file(f):
    bookmarks = []

    # get reader
    if not f or f.is_reallocated():
        return bookmarks

    reader = f.new_reader()

    if not reader or reader.size == 0:
        return bookmarks

    # decode json file
    fp = mobius.io.text_reader(reader)
    data = json.load(fp)

    # check version
    version = data.get('version')
    mobius.core.logf(f"INF Bookmarks.version = {version}")

    if version != 1:
        mobius.core.logf(f"DEV unhandled version={version}")
        return bookmarks

    # decode
    roots = data.get('roots')

    for folder in roots.values():
        if isinstance(folder, dict) and 'children' in folder:
            bookmarks += retrieve_bookmarks_from_folder(folder, f)

    f.set_handled()

    return bookmarks


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve bookmarks from folder, recursively
# @param folder Bookmark folder
# @return Bookmarks
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve_bookmarks_from_folder(folder, f):
    bookmarks = []
    folder_name = folder.get('name')
    evidence_source = pymobius.get_evidence_source_from_file(f)

    for child in folder.get('children'):
        if isinstance(child, dict):
            type = child.get('type')

            if type == 'url':
                b = pymobius.Data()
                b.id = child.get('id')
                b.name = child.get('name')
                b.creation_time = mobius.datetime.new_datetime_from_nt_timestamp(int(child.get('date_added', '0')) * 10)
                b.url = child.get('url')
                b.folder = folder_name
                b.path = f.path
                b.last_visited_time = None
                b.evidence_source = evidence_source

                meta_info = child.get('meta_info')
                if meta_info:
                    b.last_visited_time = mobius.datetime.new_datetime_from_nt_timestamp(
                        int(meta_info.get('last_visited_desktop', '0')) * 10)

                bookmarks.append(b)

            elif type == 'folder':
                bookmarks += retrieve_bookmarks_from_folder(child, f)

            else:
                mobius.core.logf(f"DEV unhandled bookmark type: {type}")

    return bookmarks
