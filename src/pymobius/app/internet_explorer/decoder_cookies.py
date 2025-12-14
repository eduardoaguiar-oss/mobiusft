# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import traceback

import mobius
import pymobius


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Internet Explorer cookie file decoder
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def decode(f):
    # check reader
    reader = f.new_reader()

    if not reader:
        return

    # decode cookie file
    cookies = []
    state = 0
    time_low = 0
    evidence_source = pymobius.get_evidence_source_from_file(f)

    try:
        fp = mobius.core.io.line_reader(reader, 'utf-8', '\n')

        for line in fp:
            line = line.rstrip()

            if state == 0:
                cookie = pymobius.Data()
                cookie.name = line
                cookie.evidence_path = f.path
                cookie.last_access_time = f.access_time
                cookie.is_deleted = f.is_deleted()
                cookie.evidence_source = evidence_source
                state = state + 1

            elif state == 1:
                cookie.value = bytes(line, 'cp1252')
                state = state + 1

            elif state == 2:
                cookie.domain = line[:-1]  # remove trailing '/'
                state = state + 1

            elif state == 3:
                cookie.flags = line
                state = state + 1

            elif state == 4:
                time_low = int(line)
                state = state + 1

            elif state == 5:
                time_high = int(line)
                timestamp = (time_high << 32) | time_low
                cookie.expiration_time = mobius.core.datetime.new_datetime_from_nt_timestamp(timestamp)
                state = state + 1

            elif state == 6:
                time_low = int(line)
                state = state + 1

            elif state == 7:
                time_high = int(line)
                timestamp = (time_high << 32) | time_low
                cookie.creation_time = mobius.core.datetime.new_datetime_from_nt_timestamp(timestamp)
                state = state + 1

            elif state == 8:
                cookies.append(cookie)
                state = 0

    except Exception as e:
        mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

    return cookies
