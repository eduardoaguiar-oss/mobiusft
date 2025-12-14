# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008-2026 Eduardo Aguiar
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
# @brief Decode "transfers" table
# @param db Database object
# @param account User account
# @return transfers
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(db, account):
    transfers = []

    # retrieve data
    SQL_STATEMENT = '''
       SELECT partner_handle,
              partner_dispname,
              status,
              starttime,
              finishtime,
              filepath,
              filename,
              filesize,
              bytestransferred,
              type
         FROM transfers'''

    for row in db.execute(SQL_STATEMENT):
        ft = pymobius.Data()
        ft.status = row[2]
        ft.start_time = mobius.core.datetime.new_datetime_from_unix_timestamp(row[3])
        ft.finish_time = mobius.core.datetime.new_datetime_from_unix_timestamp(row[4])
        ft.path = row[5]
        ft.filename = row[6]
        ft.size = row[7]
        ft.bytes_transferred = row[8]
        ft.type = row[9]

        # set from/to accounts
        if ft.type == 1:  # receive
            ft.from_skype_account = row[0]
            ft.from_skype_name = row[1]
            ft.to_skype_account = account.id
            ft.to_skype_name = account.name

        elif ft.type == 2:  # send
            ft.from_skype_account = account.id
            ft.from_skype_name = account.name
            ft.to_skype_account = row[0]
            ft.to_skype_name = row[1]

        transfers.append(ft)

    return transfers
