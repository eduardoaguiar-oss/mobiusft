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
import binascii
import traceback
import xml.dom.minidom

import mobius
import mobius.core.os
import pymobius
import pymobius.registry.main
import pymobius.registry.network_list
from pymobius.registry import get_data_as_dword

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ANT_ID = 'wireless-networks'
ANT_NAME = 'Wireless networks'
ANT_VERSION = '1.0'
EVIDENCE_TYPE = 'wireless-network'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: wireless-network
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
        self.__dpapi_master_keys = {}
        self.__profiles = {}
        self.__evidences = []

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        if not self.__item.has_datasource():
            return

        self.__evidences = []
        self.__retrieve_win()
        self.__save_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Windows
    # @see https://www.securedglobe.net/single-post/2017/01/09/michael-haephrati-the-secrets-of-wi-fi-credentials
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_win(self):

        try:
            self.__retrieve_dpapi_encryption_keys()
            ant = pymobius.registry.main.Ant(self.__item)

            for registry in ant.get_data():
                self.__retrieve_evidences_from_registry(registry)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve DPAPI encryption keys
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_dpapi_encryption_keys(self):
        for key in self.__item.get_evidences('encryption-key'):
            if key.key_type == 'dpapi.sys':
                self.__dpapi_master_keys[key.id] = key.value

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve evidences from registry
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_evidences_from_registry(self, registry):
        try:
            for key in registry.get_key_by_mask(f"HKLM\\SOFTWARE\\Microsoft\\WlanSvc\\Interfaces\\*"):
                self.__retrieve_evidences_from_interfaces_key(key)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve evidences from Interfaces key
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_evidences_from_interfaces_key(self, key):
        try:
            interface_guid = key.name

            for profile_key in key.get_key_by_mask(f"Profiles\\*"):
                self.__retrieve_evidences_from_profiles_key(interface_guid, profile_key)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve evidences from Profiles key
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_evidences_from_profiles_key(self, interface_guid, key):
        profile_guid = key.name

        # create evidence
        evidence = pymobius.Data()
        evidence.type = 'wireless-network'

        evidence.attrs = {
            'bssid': None,
            'ssid': None,
            'password': None,
            'evidence_source': None,
        }

        evidence.metadata = mobius.core.pod.map()
        evidence.metadata.set('interface-guid', interface_guid[1:-1])
        evidence.metadata.set('profile-guid', profile_guid[1:-1])

        # retrieve data from MetaData key
        metadata_key = key.get_key_by_name("MetaData")
        self.__retrieve_evidence_data_from_metadata_key(metadata_key, evidence)

        # read data from XML file
        self.__retrieve_evidence_data_from_datasource(interface_guid, profile_guid, evidence)

        # add evidence to evidence list
        self.__evidences.append(evidence)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve evidence data from datasource
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_evidence_data_from_metadata_key(self, key, evidence):
        if not key:
            return

        # has connected?
        has_connected = get_data_as_dword(key.get_data_by_name("Has Connected"))
        if has_connected is not None:
            evidence.metadata.set('has-connected', bool(has_connected))

        # connected BSSID
        data = key.get_data_by_name("Connected Bssids")
        if data:
            evidence.attrs['bssid'] = mobius.core.encoder.hexstring(data.data[:6], ':')

        # creator SID
        data = key.get_data_by_name("CreatorSid")
        if data:
            decoder = mobius.core.decoder.data_decoder(data.data)
            evidence.metadata.set('creator-sid', decoder.get_sid())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve evidence data from datasource
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_evidence_data_from_datasource(self, interface_guid, profile_guid, evidence):
        datasource = self.__item.get_datasource()
        vfs = datasource.get_vfs()

        for entry in vfs.get_root_entries():
            if entry.is_folder():
                f = entry.get_child_by_path(f"ProgramData/Microsoft/Wlansvc/Profiles/Interfaces/{interface_guid}/{profile_guid}.xml")
                self.__retrieve_evidence_data_from_xml_file(f, evidence)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve profile from Wlansvc .xml file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_evidence_data_from_xml_file(self, f, evidence):

        # read data from file
        if not f or f.is_reallocated():
            return

        reader = f.new_reader()
        if not reader:
            return

        # create evidence
        evidence.attrs['evidence_source'] = pymobius.get_evidence_source_from_file(f)

        # retrieve evidence data
        data = reader.read()
        self.__retrieve_evidence_data_from_xml_data(data, evidence)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve evidence data from .xml file data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_evidence_data_from_xml_data(self, data, evidence):
        doc = xml.dom.minidom.parseString(data)
        root = doc.childNodes[0]

        for node in root.childNodes:
            if node.nodeType == node.ELEMENT_NODE and node.tagName == 'name':
                evidence.attrs['ssid'] = node.firstChild.data.strip()

            elif node.nodeType == node.ELEMENT_NODE and node.tagName == 'connectionType':
                evidence.metadata.set('connection-type', node.firstChild.data.strip())

            elif node.nodeType == node.ELEMENT_NODE and node.tagName == 'connectionMode':
                evidence.metadata.set('connection-mode', node.firstChild.data.strip())

            elif node.nodeType == node.ELEMENT_NODE and node.tagName == 'MSM':
                for child in node.getElementsByTagName("authEncryption"):
                    self.__retrieve_auth_encryption(child, evidence)

                for child in node.getElementsByTagName("sharedKey"):
                    self.__retrieve_shared_key(child, evidence)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from authEncryption XML node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_auth_encryption(self, node, evidence):
        for child in node.childNodes:
            if child.nodeType == child.ELEMENT_NODE and child.tagName == 'authentication':
                evidence.metadata.set('authentication', child.firstChild.data.strip())

            elif child.nodeType == child.ELEMENT_NODE and child.tagName == 'encryption':
                evidence.metadata.set('encryption', child.firstChild.data.strip())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from sharedKey XML node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_shared_key(self, node, evidence):
        for child in node.childNodes:
            if child.nodeType == child.ELEMENT_NODE and child.tagName == 'keyType':
                evidence.metadata.set('key-type', child.firstChild.data.strip())

            elif child.nodeType == child.ELEMENT_NODE and child.tagName == 'protected':
                evidence.metadata.set('is-protected', child.firstChild.data.strip() == 'true')

            elif child.nodeType == child.ELEMENT_NODE and child.tagName == 'keyMaterial':
                key_material = child.firstChild.data.strip()
                self.__retrieve_password(key_material, evidence)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve password from key_material
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_password(self, key_material, evidence):
        blob = mobius.core.os.win.dpapi.blob(binascii.unhexlify(key_material))
        key_value = self.__dpapi_master_keys.get(blob.master_key_guid)

        if key_value and blob.decrypt(key_value):
            value = blob.plain_text.rstrip(b'\x00')

            if b'\x00' not in value:
                evidence.attrs['password'] = value.decode('utf-8')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        transaction = self.__item.new_transaction()

        # save evidences
        for e in self.__evidences:
            evidence = self.__item.new_evidence(e.type)

            for key, value in e.attrs.items():
                setattr(evidence, key, value)

            evidence.metadata = e.metadata

        # commit data
        transaction.commit()
