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
import mobius

ANT_ID = 'kff-alert'
ANT_NAME = 'KFF Alert tagger'
ANT_VERSION = '1.0'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Check if an evidence is an alert based
# @param e Evidence object
# @param kff KFF database object
# @return If any hash is found in the KFF database with an 'A' status, the function
# returns True, indicating that the event is an alert. Otherwise, it returns False.
#
# @details Check if an evidence is an alert based on the hashes associated with it.
# It iterates through the hashes and checks if any of them are found in the
# Known File Fingerprint (KFF) database with a status of 'A' (Alert).
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def is_alert(e, kff):
    if not e.has_attribute('hashes'):
        return False

    for hash_type, hash_value in e.get_attribute('hashes') or []:
        if kff.lookup(hash_type, hash_value) == 'A':
            return True

    return False

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: KFF Alert
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

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        kff = mobius.kff.kff()
        connection = kff.new_connection()

        self.__tag_evidences('received-file')
        self.__tag_evidences('sent-file')
        self.__tag_evidences('shared-file')
        self.__tag_evidences('local-file')
        self.__tag_evidences('p2p-remote-file')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Tag evidences
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __tag_evidences(self, evidence_id):
        kff = mobius.kff.kff()

        # Iterate through evidences obtained from an item using a specific evidence ID.
        # For each evidence, it checks if any of the hashes (of a specified type and value)
        # are classified as alert ('A') using a lookup method. If so, the evidence is
        # appended to the evidences list.
        evidences = [e for e in self.__item.get_evidences(evidence_id) if is_alert(e, kff)]

        # Set two tags, 'alert' and 'alert.kff', on each evidence in the evidences list.
        transaction = self.__item.new_transaction()

        for e in evidences:
            e.set_tag('alert')
            e.set_tag('alert.kff')

        transaction.commit()
