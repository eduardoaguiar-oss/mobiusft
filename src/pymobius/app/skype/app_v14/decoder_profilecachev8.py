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
import json
import traceback

import mobius
import pymobius


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get member ID from Skype string
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_member_id(s):
    if s:
        return s.split(':', 1)[1]
    return None


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode "profilecachev8" table, searching for account
# @param db Database object
# @param user_id User ID
# @return accounts
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(db, user_id):
    profiles = []
    names = {}

    # Load profiles from profilecachev8 table
    SQL_STATEMENT = '''
       SELECT nsp_pk,
              nsp_data
         FROM profilecachev8'''

    for row in db.execute(SQL_STATEMENT):
        try:
            profile = decode_profile(row)

            if profile:
                profiles.append(profile)
                names[profile.id] = profile.fullname

        except Exception as e:
            mobius.core.logf('WRN %s %s' % (str(e), traceback.format_exc()))

    return profiles, names


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode "profilecachev8" table row
# @param row Table row
# @return profile
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_profile(row):
    nsp_pk = row[0]
    nsp_data = json.loads(row[1])

    profile = pymobius.Data()
    profile.type = 2  # s4l db
    profile.mri = nsp_data.get('mri')
    profile.id = get_member_id(profile.mri)
    profile.fullname = nsp_data.get('fullName') or nsp_data.get('displayNameOverride')
    profile.birthday = nsp_data.get('birthday')
    profile.country = nsp_data.get('country')
    profile.province = nsp_data.get('province')
    profile.city = nsp_data.get('city')
    profile.emails = nsp_data.get('emails', [])
    print(profile.emails)
    profile.thumb_url = nsp_data.get('thumbUrl')
    profile.gender = nsp_data.get('gender')
    profile.mood_text = nsp_data.get('mood')
    profile.phones = []

    for p in nsp_data.get('phones', []):
        profile.phones.append(p.get('number').strip())

    return profile
