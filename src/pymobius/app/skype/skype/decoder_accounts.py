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
# @brief Decode "accounts" table
# @param db Database object
# @return accounts
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(db):
    accounts = []

    SQL_STATEMENT = '''
       SELECT skypename,
              fullname,
              birthday,
              gender,
              languages,
              country,
              province,
              city,
              phone_home,
              phone_office,
              phone_mobile,
              emails,
              homepage,
              about,
              profile_timestamp,
              lastonline_timestamp,
              lastused_timestamp,
              avatar_timestamp,
              mood_timestamp,
              displayname,
              mood_text,
              timezone,
              ipcountry,
              avatar_image,
              skypeout_balance_currency,
              skypeout_balance
         FROM accounts'''

    for row in db.execute(SQL_STATEMENT):
        account = pymobius.Data()
        account.type = 1  # main.db
        account.birthday = None
        account.avatar_image = None
        account.id = row[0]
        account.fullname = row[1]
        account.gender = row[3]
        account.languages = row[4]
        account.country = row[5]
        account.province = row[6]
        account.city = row[7]
        account.phone_home = row[8]
        account.phone_office = row[9]
        account.phone_mobile = row[10]
        account.homepage = row[12]
        account.about = row[13]
        account.profile_timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(row[14])
        account.last_online_timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(row[15] or 0)
        account.last_used_timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(row[16] or 0)
        account.avatar_timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(row[17] or 0)
        account.mood_timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(row[18] or 0)
        account.displayname = row[19]
        account.mood_text = row[20]
        # account.timezone = self.__get_timezone (row[21])
        account.ipcountry = row[22]
        account.balance_currency = row[24]
        account.balance = row[25]

        # birthday
        if row[2]:
            value = row[2]
            account.birthday = '%04d-%02d-%02d' % (value // 10000, (value // 100) % 100, value % 100)

        # emails
        if row[11]:
            account.emails = row[11].split(' ')
        else:
            account.emails = []

        # avatar image
        if row[23]:
            image_data = row[23]
            account.avatar_image = image_data[1:]

        accounts.append(account)

    return accounts
