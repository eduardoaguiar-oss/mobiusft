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
import mobius
import pymobius


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode "calls" table
# @param db Database object
# @return calls
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(db, user_id):
    calls = []

    # get call members
    members = {}

    SQL_STATEMENT = '''
       SELECT call_db_id,
              identity
         FROM callmembers'''

    for row in db.execute(SQL_STATEMENT):
        call_id = row[0]
        identity = row[1]
        members.setdefault(call_id, set()).add(identity)

    # get calls
    SQL_STATEMENT = '''
       SELECT id,
              begin_timestamp,
              host_identity,
              duration,
              is_incoming,
              is_conference,
              is_unseen_missed
         FROM calls'''

    for row in db.execute(SQL_STATEMENT):
        call = pymobius.Data()
        call.id = row[0]
        call.timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(row[1])
        call.duration = row[3] or 0
        call.is_incoming = (row[4] == 1)
        call.is_conference = (row[5] > 0)
        call.is_missed = (row[6] == 1)
        call.members = members.get(call.id, set())
        call.members.add(user_id)

        # check direction
        if call.is_incoming:
            call.from_id = row[2]

        else:
            call.from_id = user_id

        call.to_id_list = list(call.members.difference([call.from_id]))
        calls.append(call)

    return calls
