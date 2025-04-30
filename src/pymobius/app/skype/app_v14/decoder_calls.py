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
# @brief Update skype name dictionary
# @param names Name dictionary
# @param p Participant data
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def update_names(names, p):
    if p:
        p_name = p.get('displayName')
        p_id = get_member_id(p.get('id'))

        if p_id and p_name:
            names[p_id] = p_name


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode "calls" table
# @param db Database object
# @return calls
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(db, user_id):
    calls = []
    names = {}

    # Load calls from calllogs table
    SQL_STATEMENT = '''
       SELECT nsp_pk,
              nsp_data
         FROM calllogs'''

    for row in db.execute(SQL_STATEMENT):
        try:
            call = decode_call(row, names)
            if call:
                calls.append(call)

        except Exception as e:
            mobius.core.logf('WRN ' + str(e) + ' ' + traceback.format_exc())

    return calls, names


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decode "calls" table row
# @param row Table row
# @param names Skype names dict
# @return call
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode_call(row, names):
    nsp_pk = row[0]
    nsp_data = json.loads(row[1])

    call = pymobius.Data()
    call.id = nsp_data.get('callId')
    call.is_incoming = nsp_data.get('callDirection') == "incoming"
    call.is_missed = nsp_data.get('callState') == "missed"
    call.is_conference = nsp_data.get('callType') == "multiParty"
    call.from_id = get_member_id(nsp_data.get('originator'))
    call.timestamp = mobius.datetime.new_datetime_from_iso_string(nsp_data.get('startTime'))

    # to_id_list
    if call.is_conference:
        call.to_id_list = []

        for i in nsp_data.get('participants') or []:
            p_id = get_member_id(i)
            if p_id != call.from_id:
                call.to_id_list.append(p_id)

    else:
        call.to_id_list = [get_member_id(nsp_data.get('target'))]

    call.members = set([call.from_id] + call.to_id_list)

    # calculate call duration
    if 'endTime' in nsp_data:
        end_time = mobius.datetime.new_datetime_from_iso_string(nsp_data.get('endTime'))
        delta = end_time - call.timestamp
        call.duration = delta.days * 86400 + delta.seconds

    else:
        call.duration = 0

    # update names dictionary
    update_names(names, nsp_data.get('originatorParticipant'))
    update_names(names, nsp_data.get('targetParticipant'))

    for p in nsp_data.get('participantList') or []:
        update_names(names, p)

    return call
