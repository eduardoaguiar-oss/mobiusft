# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023 Eduardo Aguiar
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
import datetime
import sqlite3
import tempfile
import os
import mobius

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Extension metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
EXTENSION_ID = 'skype-agent-sqlite'
EXTENSION_NAME = 'Skype Agent SQLite'
EXTENSION_AUTHOR = 'Eduardo Aguiar'
EXTENSION_VERSION = '0.1.8'
EXTENSION_DESCRIPTION = 'Handle Skype sqlite log file format'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Skype agent sqlite (Skype 5.x)
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SkypeAgentSQLite(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief check if file format is compatible
    # @param stream read stream
    # @return True/False
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def check_signature(self, stream):
        stream.seek(0)
        data = stream.read(13)

        return data == 'SQLite format'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief parse sqlite log file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def parse_logfile(self, agent, stream):
        self.__agent = agent

        # create temporary file
        stream.seek(0)
        path = tempfile.mktemp(suffix='.sqlite')
        fp = open(path, 'w')
        fp.write(stream.read(stream.size))
        fp.close()

        # open database
        db = sqlite3.connect(path)

        # retrieve data
        self.retrieve_contacts(db)
        self.retrieve_voicemails(db)

        # close database
        db.close()
        os.remove(path)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief retrieve data from contacts table
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def retrieve_contacts(self, db):
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
              main_phone
         FROM contacts'''

        for row in db.execute(SQL_STATEMENT):
            contact = self.__agent.new_contact()
            contact.skypename = row[0]
            contact.fullname = row[1]
            contact.gender = row[3]
            contact.languages = row[4]
            contact.country = row[5]
            contact.province = row[6]
            contact.city = row[7]
            contact.phone_home = row[8]
            contact.phone_office = row[9]
            contact.phone_mobile = row[10]
            contact.emails = row[11]
            contact.homepage = row[12]
            contact.about = row[13]
            contact.profile_timestamp = self.__get_datetime(row[14])
            contact.last_online_timestamp = self.__get_datetime(row[15])
            contact.last_used_timestamp = self.__get_datetime(row[16])
            contact.avatar_timestamp = self.__get_datetime(row[17])
            contact.mood_timestamp = self.__get_datetime(row[18])
            contact.displayname = row[19]
            contact.mood_text = row[20]
            contact.timezone = self.__get_timezone(row[21])
            contact.ipcountry = row[22]
            contact.phone = row[24]

            # birthday
            value = row[2]
            if value:
                contact.birthday = datetime.date(value // 10000, (value // 100) % 100, value % 100)

            # avatar image
            image_data = row[23]
            if image_data:
                contact.avatar_image = image_data[1:]

            # timeline events
            if contact.profile_timestamp:
                self.__agent.add_timeline(contact.profile_timestamp, 'account %s saved' % contact.skypename)

            if contact.last_online_timestamp:
                self.__agent.add_timeline(contact.last_online_timestamp,
                                          'account %s last time online' % contact.skypename)

            if contact.last_used_timestamp:
                self.__agent.add_timeline(contact.last_used_timestamp, 'account %s last time used' % contact.skypename)

            if contact.avatar_timestamp:
                self.__agent.add_timeline(contact.avatar_timestamp,
                                          'account %s avatar image changed' % contact.skypename)

            if contact.mood_timestamp:
                self.__agent.add_timeline(contact.mood_timestamp, 'account %s mood text changed' % contact.skypename)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief retrieve data from voicemails table
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def retrieve_voicemails(self, db):
        SQL_STATEMENT = '''
       SELECT partner_handle,
              partner_dispname,
              status,
              timestamp,
              duration,
              path,
              size,
              subject
         FROM voicemails'''

        for row in db.execute(SQL_STATEMENT):
            voicemail = self.__agent.new_voicemail()
            voicemail.skypename = row[0]
            voicemail.displayname = row[1]
            voicemail.timestamp = self.__get_datetime(row[3])
            voicemail.duration = self.__get_time(row[4])
            voicemail.path = row[5]
            voicemail.size = row[6]
            voicemail.subject = row[7]

            # timeline event
            if voicemail.timestamp:
                user = self.__get_username(voicemail.skypename, voicemail.displayname)
                self.__agent.add_timeline(voicemail.timestamp,
                                          f'user {user} received a voicemail (duration={voicemail.duration}, file={voicemail.path})')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get datetime from int value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_datetime(self, value):
        if value:
            date = datetime.datetime.utcfromtimestamp(value)
        else:
            date = None

        return date

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get user fullname
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_username(self, username, displayname):
        name = displayname or username
        if username and username != displayname:
            name += ' (%s)' % username
        return name

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get time
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_time(self, value):
        value = value or 0
        ss = value % 60
        mm = (value // 60) % 60
        hh = (value // 3600)

        return f'{hh:02d}:{mm:02d}:{ss:02d}'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get timezone
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_timezone(self, value):
        if value:
            value = (value - 86400) // 60
            mm = value % 60
            hh = (value // 60)
            timezone = '%s%02d:%02d' % ('+' if hh >= 0 else '-', abs(hh), mm)
        else:
            timezone = None

        return timezone


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    mobius.core.add_resource('skype.agent.sqlite', 'Skype Agent: SQLite', SkypeAgentSQLite)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('skype.agent.sqlite')
