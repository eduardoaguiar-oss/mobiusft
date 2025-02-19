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
# import pymobius.app.emule.profile
import traceback

import mobius
import pymobius


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief scan profiles inside user folder
# @param user_profile user profile object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve_profiles(user_profile):
    profiles = []

    pfolder = user_profile.get_appdata_local_folder('Google/Chrome/User Data/Default')

    if pfolder:
        p = pymobius.app.emule.profile.Profile(pfolder)
        p.username = user_profile.username
        p.name = user_profile.username
        profiles.append(p)

    return profiles


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve(opsys):

    # build data structure
    data = pymobius.Data()
    data.profiles = []
    data.versions = []

    try:
        data.versions = retrieve_versions(opsys)

        # get profiles from user folder
        # for user_profile in opsys.get_profiles ():
        #   data.profiles += retrieve_profiles (user_profile)

    except Exception as e:
        mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

    return data


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve Emule data
# @param opsys Operating System object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve_versions(opsys):
    versions = []

    # get installed versions
    for program in opsys.get_installed_programs():
        program_name = program.display_name.lower()

        if program_name.startswith('emule plus '):
            versions.append(program.display_name[11:])

        elif program_name.startswith('emule '):
            versions.append(program.display_name[6:])

        elif program_name.startswith('dreamule '):
            versions.append(program.display_name)

        elif program_name == 'emule':
            versions.append(program.version)

    # return installed versions
    return versions
