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
import sqlite3

import mobius
import pymobius
import pymobius.app.chromium


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Load data from Web Data file
# Versions tested: 40, 43, 45, 52, 56, 58, 60, 61, 64, 65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(path):
    # create data object
    data = pymobius.Data()
    data.form_history = []

    # get meta version
    db = sqlite3.connect(path)
    data.version = pymobius.app.chromium.get_meta_version(db)

    if data.version:
        mobius.core.logf(f"INF app.chromium: Web Data.version = {data.version}")

        decode_autofill(db, data)
        decode_autofill_profiles(db, data)
        decode_autofill_profile_emails(db, data)
        decode_autofill_profile_names(db, data)
        decode_autofill_profile_phones(db, data)

    db.close()

    # return data object
    return data


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from autofill table
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_autofill(db, data):
    if data.version < 55:
        decode_autofill_40(db, data)

    else:
        decode_autofill_55(db, data)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from autofill table
# Versions tested: 40, 43, 45, 48, 52
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_autofill_40(db, data):
    cursor = db.cursor()

    stmt = '''
       SELECT name, value
         FROM autofill'''

    for row in cursor.execute(stmt):
        entry = pymobius.Data()
        entry.id = len(data.form_history) + 1
        entry.fieldname = row[0]
        entry.value = row[1]
        entry.use_count = 0
        entry.first_used_time = None
        entry.last_used_time = None
        entry.is_encrypted = False

        data.form_history.append(entry)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from autofill table
# Versions tested: 55-56,58,60-61,64-65,67,70-72,74,76-78,80-84,86-88,90-92,96-98,100,104
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_autofill_55(db, data):
    cursor = db.cursor()

    stmt = '''
       SELECT _rowid_,
              name,
              value,
              count,
              date_created,
              date_last_used
         FROM autofill'''

    for row in cursor.execute(stmt):
        entry = pymobius.Data()
        entry.id = len(data.form_history) + 1
        entry.fieldname = row[1]
        entry.value = row[2]
        entry.use_count = row[3]
        entry.first_used_time = mobius.datetime.new_datetime_from_unix_timestamp(row[4])
        entry.last_used_time = mobius.datetime.new_datetime_from_unix_timestamp(row[5])
        entry.is_encrypted = isinstance(entry.value, bytes)

        data.form_history.append(entry)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from autofill_profiles table
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_autofill_profiles(db, data):
    if data.version < 55:
        decode_autofill_profiles_40(db, data)

    elif data.version < 61:
        decode_autofill_profiles_55(db, data)

    elif data.version < 113:
        decode_autofill_profiles_61(db, data)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from autofill_profiles table
# Versions tested: 40, 43, 45, 52
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_autofill_profiles_40(db, data):
    cursor = db.cursor()

    stmt = '''
       SELECT company_name,
              address_line_1,
              address_line_2,
              city,
              state,
              zipcode,
              country,
              country_code
         FROM autofill_profiles'''

    for row in cursor.execute(stmt):
        for idx, name in enumerate(
                ['company_name', 'address_line_1', 'address_line_2', 'city', 'state', 'zipcode', 'country',
                 'country_code']):
            if row[idx]:
                entry = pymobius.Data()
                entry.id = len(data.form_history) + 1
                entry.fieldname = name
                entry.value = row[idx]
                entry.use_count = 0
                entry.first_used_time = None
                entry.last_used_time = None
                entry.is_encrypted = False

                data.form_history.append(entry)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from autofill_profiles table
# Versions tested: 55, 56, 58, 60
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_autofill_profiles_55(db, data):
    cursor = db.cursor()

    stmt = '''
       SELECT company_name,
              street_address,
              dependent_locality,
              city,
              state,
              zipcode,
              country_code
         FROM autofill_profiles'''

    for row in cursor.execute(stmt):
        for idx, name in enumerate(
                ['company_name', 'street_address', 'dependent_locality', 'city', 'state', 'zipcode', 'country_code']):
            if row[idx]:
                entry = pymobius.Data()
                entry.id = len(data.form_history) + 1
                entry.fieldname = name
                entry.value = row[idx]
                entry.use_count = 0
                entry.first_used_time = None
                entry.last_used_time = None
                entry.is_encrypted = isinstance(entry.value, bytes)

                data.form_history.append(entry)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from autofill_profiles table
# Versions tested: 61,64,65,67,70-72,74,76-78,80-84,86-88,90-92,96-98,100,104,107,110-112
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_autofill_profiles_61(db, data):
    cursor = db.cursor()

    stmt = '''
       SELECT company_name,
              street_address,
              dependent_locality,
              city,
              state,
              zipcode,
              country_code,
              use_date,
              use_count
         FROM autofill_profiles'''

    for row in cursor.execute(stmt):
        for idx, name in enumerate(
                ['company_name', 'street_address', 'dependent_locality', 'city', 'state', 'zipcode', 'country_code']):
            if row[idx]:
                entry = pymobius.Data()
                entry.id = len(data.form_history) + 1
                entry.fieldname = name
                entry.value = row[idx]
                entry.use_count = 0
                entry.first_used_time = None
                entry.last_used_time = mobius.datetime.new_datetime_from_unix_timestamp(row[7])
                entry.is_encrypted = isinstance(entry.value, bytes)

                if row[8] == 1:
                    entry.first_used_time = entry.last_used_time

                data.form_history.append(entry)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from autofill_profile_emails table
# Versions tested: 40,43,45,48,55-56,58,60-61,64-65,67,70-72,74,76-78,80-84,86-88,90-92,96-98,100,104
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_autofill_profile_emails(db, data):
    cursor = db.cursor()

    stmt = '''
       SELECT DISTINCT email
         FROM autofill_profile_emails
        WHERE email <> ""'''

    for row in cursor.execute(stmt):
        entry = pymobius.Data()
        entry.id = len(data.form_history) + 1
        entry.fieldname = 'email'
        entry.value = row[0]
        entry.use_count = 0
        entry.first_used_time = None
        entry.last_used_time = None
        entry.is_encrypted = isinstance(entry.value, bytes)

        data.form_history.append(entry)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from autofill_profile_names table
# Versions tested: 40,43,45,48,55-56,58,60-61,64-65,67,70-72,74,76-78,80-84,86-88,90-92,96-98,100,104
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_autofill_profile_names(db, data):
    cursor = db.cursor()

    stmt = '''
       SELECT first_name, middle_name, last_name
         FROM autofill_profile_names'''

    for row in cursor.execute(stmt):
        for idx, name in enumerate(['first_name', 'middle_name', 'last_name']):
            if row[idx]:
                entry = pymobius.Data()
                entry.id = len(data.form_history) + 1
                entry.fieldname = name
                entry.value = row[idx]
                entry.use_count = 0
                entry.first_used_time = None
                entry.last_used_time = None
                entry.is_encrypted = isinstance(entry.value, bytes)

                data.form_history.append(entry)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from autofill_profile_phones table
# Versions tested: 40,43,45,48,55-56,58,60-61,64-65,67,70-72,74,76-78,80-84,86-88,90-92,96-98,100,104
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_autofill_profile_phones(db, data):
    cursor = db.cursor()

    stmt = '''
       SELECT DISTINCT number
         FROM autofill_profile_phones
        WHERE number <> ""'''

    for row in cursor.execute(stmt):
        entry = pymobius.Data()
        entry.id = len(data.form_history) + 1
        entry.fieldname = 'phone_number'
        entry.value = row[0]
        entry.use_count = 0
        entry.first_used_time = None
        entry.last_used_time = None
        entry.is_encrypted = isinstance(entry.value, bytes)

        data.form_history.append(entry)
