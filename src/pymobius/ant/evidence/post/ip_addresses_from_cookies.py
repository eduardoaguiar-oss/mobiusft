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
import re
import traceback

import mobius
import pymobius

ANT_ID = 'ip-addresses-from-cookies'
ANT_NAME = 'IP Addresses from Cookies'
ANT_VERSION = '1.0'
EVIDENCE_TYPE = "ip-address"

RE_IPv4 = re.compile(r'\b(?:[0-9]{1,3}\.){3}[0-9]{1,3}\b')

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: IP Addresses
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.id = ANT_ID
        self.name = ANT_NAME
        self.version = ANT_VERSION
        self.__item = item
        self.__entries = []

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__entries = []
        self.__retrieve_cookies()
        self.__save_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from cookies
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_cookies(self):
        for c in self.__item.get_evidences('cookie'):
            if not c.is_encrypted and c.last_update_time:
                self.__retrieve_cookie(c)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from cookie
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_cookie(self, c):
        try:
            ip, cookie_metadata = self.__get_cookie_ip(c)

            if ip:
                entry = pymobius.Data()
                entry.timestamp = c.last_update_time
                entry.address = ip
                entry.app_id = c.app_id
                entry.app_name = c.app_name
                entry.username = c.username
                entry.evidence_source = f"Cookie {c.name}"

                metadata = mobius.core.pod.map()
                metadata.set('cookie-name', c.name)
                metadata.set('cookie-domain', c.domain)
                metadata.set('cookie-evidence-source', c.evidence_source)
                metadata.update(cookie_metadata)
                metadata.update(c.metadata)

                entry.metadata = metadata

                self.__entries.append(entry)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get IP address from cookie value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_cookie_ip(self, c):
        ip = None
        metadata = mobius.core.pod.map()

        if c.name == 'kt_ips':
            uri = mobius.core.io.uri(c.value.decode('utf-8'))
            value = uri.get_path('utf-8')

            if ',' not in value and (value.count('.') == 3 or value.count(':') > 1):
                ip = value
            else:
                mobius.core.logf(f"DEV cookie of interest: {c.name}. Value: {value}")

        elif c.name == 'LBSRC':
            value = c.value.decode('utf-8')

            if value[:1] == value[-1:] == '"':
                value = value[1:-1]
            if value.endswith('%1'):
                value = value[:-2]
            if ' ' in value:
                ip = value.rsplit(' ', 1)[1]

        elif c.name in ('MSCC', 'MS-CV'):
            if len (c.value) > 32:
               text = c.value[32:].decode('utf-8')

            else:
                text = c.value.decode('utf-8')
            
            if '-' in text:
               ip, country = text.split('-', 1)
               metadata.set('country-code', country)

            if c.value and not ip:
                mobius.core.logf(f"DEV cookie of interest: {c.name}. Value:")
                mobius.core.logf(f"DEV {pymobius.dump(c.value)}")
        
        elif c.name == 'voxusmediamanager__ip':
            if len (c.value) > 32:
               ip = c.value[32:].decode('utf-8')
               
        elif c.name == 'geode':
            data = c.value.decode('utf-8').split(':')
            if len (data) > 1:
                ip = data[1]
            if len (data) > 6:
                metadata.set('country-code', data[6])
            if len (data) > 7:
                metadata.set('region', data[7])
            
        else:
            value = c.value.decode('utf-8', errors='ignore')
            match = RE_IPv4.search(value)
            if match:
                mobius.core.logf(f"DEV cookie of interest: {c.name}. Value:")
                mobius.core.logf(f"DEV {pymobius.dump(c.value)}")
                
        

        return ip, metadata

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        transaction = self.__item.new_transaction()

        # save evidences
        for e in self.__entries:
            ip_address = self.__item.new_evidence(EVIDENCE_TYPE)
            ip_address.timestamp = e.timestamp
            ip_address.address = e.address
            ip_address.app_id = e.app_id
            ip_address.app_name = e.app_name
            ip_address.username = e.username
            ip_address.evidence_source = e.evidence_source
            ip_address.metadata = e.metadata

        # commit data
        transaction.commit()
