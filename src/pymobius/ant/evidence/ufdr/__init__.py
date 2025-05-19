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
import traceback

import mobius
import pymobius
import pymobius.datasource.ufdr.parser

from . import autofill
from . import call
from . import chat
from . import contact
from . import cookie
from . import credit_card
from . import file_download
from . import financial_account
from . import installed_application
from . import instant_message
from . import password
from . import searched_item
from . import user_account
from . import visited_page
from . import web_bookmark
from . import wireless_network

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ANT_ID = 'evidence.ufdr'
ANT_NAME = 'Evidence Finder Agent - UFDR'
ANT_VERSION = '1.0'
SAVE_THRESHOLD = 131072

HANDLERS = {
    'Autofill': autofill.handler,
    'Call': call.handler,
    'Chat': chat.handler,
    'Contact': contact.handler,
    'Cookie': cookie.handler,
    'CreditCard': credit_card.handler,
    'FileDownload': file_download.handler,
    'FinancialAccount': financial_account.handler,
    'InstalledApplication': installed_application.handler,
    'InstantMessage': instant_message.handler,
    'Password': password.handler,
    'SearchedItem': searched_item.handler,
    'UserAccount': user_account.handler,
    'VisitedPage': visited_page.handler,
    'WebBookmark': web_bookmark.handler,
    'WirelessNetwork': wireless_network.handler,
}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: UFDR Evidences
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.id = ANT_ID
        self.name = ANT_NAME
        self.version = ANT_VERSION

        self.__item = item
        self.__evidences = []
        self.__file_hashes = {}
        self.__unhandled_types = set()
        self.__control = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set control object
    # @param control Control object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_control(self, control):
        self.__control = control

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__control.log(f"INF ant {self.id} started")

        try:
            # get UFDR path
            datasource = self.__item.get_datasource()
            uri = mobius.core.io.uri(datasource.get_url())
            path = uri.get_path('utf-8')

            # run UFDR parser
            parser = pymobius.datasource.ufdr.parser.UFDRParser(path, self)
            parser.run()

            # save remaining evidences
            self.__save_evidences()

            # log evidence_types found
            for e in sorted(self.__unhandled_types):
                mobius.core.logf(f"DEV unhandled evidence type: {e}")

            # log unknown_datatypes found
            for dt in sorted(parser.get_unknown_datatypes()):
                mobius.core.logf(f"DEV unknown datatype: {dt}")

        except Exception as e:
            self.__control.log(f'WRN {str(e)}\n{traceback.format_exc()}')

        self.__control.log(f"INF ant {self.id} ended")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Reset ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def reset(self):
        self.__item.reset_ant('evidence')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_element_end event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_element_end(self, tag):
        # stop scanning at <project></decodedData>
        return tag == 'project.decodedData'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_tagged_file event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_tagged_file(self, f):
        hashes = []

        hash_md5 = f.file_metadata.get('MD5')
        if hash_md5 and hash_md5 != 'N/A':
            hashes.append(('md5', hash_md5))

        hash_sha2_256 = f.file_metadata.get('SHA256')
        if hash_sha2_256 and hash_sha2_256 != 'N/A':
            hashes.append(('sha2_256', hash_sha2_256))

        if hashes:
            self.__file_hashes[f.id] = hashes

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_evidence event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_evidence(self, e):
        print()
        print(f'   ID: {e.id}')
        print(f'   Type: {e.type}')
        print(f'   Source idx: {e.source_index}')
        print(f'   Extraction ID: {e.extraction_id}')
        print(f'   Deleted state: {e.deleted_state}')
        print(f'   Decoding confidence: {e.decoding_confidence}')
        print(f'   Is carved: {e.is_carved}')
        print(f'   Is related: {e.is_related}')

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Get evidence handler
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        handler = HANDLERS.get(e.type)
        if not handler:
            return

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Add evidences generated by handlers
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        try:
            for evidence in handler(e):

                # set evidence.hashes, if possible
                file_id = evidence.attrs.get('file_id')
                if file_id:
                    hashes = self.__file_hashes.get(file_id)
                    if hashes:
                        evidence.attrs['hashes'] = hashes

                # add evidence to list
                self.__evidences.append(evidence)

                # save evidences
                if len(self.__evidences) > SAVE_THRESHOLD:
                    self.__save_evidences()
        except Exception as e:
            self.__control.log(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save evidences into case DB
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_evidences(self):
        print('SAVING!!!')
        transaction = self.__item.new_transaction()

        for e in self.__evidences:
            evidence = self.__item.new_evidence(e.type)

            for key, value in e.attrs.items():
                setattr(evidence, key, value)

            evidence.metadata = e.metadata

        transaction.commit()

        self.__evidences = []
