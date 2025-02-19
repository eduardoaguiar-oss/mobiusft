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
import pymobius.app.chromium
import pymobius.app.gecko
import pymobius.app.itubego
import pymobius.registry.main
import pymobius.registry.search_assistant
import pymobius.registry.wordwheelquery


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ANT_ID = 'autofill'
ANT_NAME = 'Autofill data'
ANT_VERSION = '1.3'
EVIDENCE_TYPE = 'autofill'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: autofill
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
        if not self.__item.has_datasource():
            return

        self.__entries = []
        self.__retrieve_chromium()
        self.__retrieve_gecko()
        self.__retrieve_itubego()
        self.__retrieve_win_registry()
        self.__save_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Chromium based browsers
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_chromium(self):

        try:
            model = pymobius.app.chromium.model(self.__item)

            for profile in model.get_profiles():
                for h in profile.get_form_history():
                    if not h.is_encrypted:
                        entry = pymobius.Data()
                        entry.field_name = h.fieldname
                        entry.value = h.value
                        entry.app_id = profile.app_id
                        entry.app_name = profile.app_name
                        entry.username = profile.username

                        entry.metadata = mobius.pod.map()
                        entry.metadata.set('first_used_time', h.first_used_time)
                        entry.metadata.set('last_used_time', h.last_used_time)
                        entry.metadata.set('app_id', profile.app_id)
                        entry.metadata.set('profile_name', profile.name)
                        entry.metadata.set('profile_path', profile.path)

                        self.__entries.append(entry)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Gecko based browsers
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_gecko(self):

        try:
            model = pymobius.app.gecko.model(self.__item)

            for profile in model.get_profiles():
                for h in profile.get_form_history():
                    entry = pymobius.Data()
                    entry.field_name = h.fieldname
                    entry.value = h.value
                    entry.app_id = profile.app_id
                    entry.app_name = profile.app_name
                    entry.username = profile.username

                    entry.metadata = mobius.pod.map()
                    entry.metadata.set('first_used_time', h.first_used_time)
                    entry.metadata.set('last_used_time', h.last_used_time)
                    entry.metadata.set('app_id', profile.app_id)
                    entry.metadata.set('profile_name', profile.name)
                    entry.metadata.set('profile_path', profile.path)

                    self.__entries.append(entry)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from iTubeGo app
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_itubego(self):

        try:
            model = pymobius.app.itubego.model(self.__item)

            for profile in model.get_profiles():
                for e in profile.get_history_entries():
                    entry = pymobius.Data()
                    entry.field_name = "url"
                    entry.value = e.url
                    entry.app_id = profile.app_id
                    entry.app_name = profile.app_name
                    entry.username = profile.username

                    entry.metadata = mobius.pod.map()
                    entry.metadata.set('evidence-source', e.evidence_source)
                    entry.metadata.set('is_deleted', e.is_deleted)
                    entry.metadata.set('record_idx', e.idx)
                    entry.metadata.set('app_id', profile.app_id)
                    entry.metadata.set('profile_name', profile.name)
                    entry.metadata.set('profile_path', profile.path)

                    self.__entries.append(entry)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Windows' Registry
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_win_registry(self):

        try:
            ant = pymobius.registry.main.Ant(self.__item)
            for reg in ant.get_data():

                # WordWheelQuery
                for e in pymobius.registry.wordwheelquery.get(reg):
                    entry = pymobius.Data()
                    entry.username = e.username
                    entry.app_id = e.app_id
                    entry.app_name = e.app_name
                    entry.field_name = e.fieldname
                    entry.value = e.value

                    entry.metadata = mobius.pod.map()
                    entry.metadata.set('evidence-source', e.evidence_source)
                    entry.metadata.set('mrulistex_index', e.idx)
                    self.__entries.append(entry)

                # Search Assistant
                for e in pymobius.registry.search_assistant.get(reg):
                    entry = pymobius.Data()
                    entry.username = e.username
                    entry.app_id = e.app_id
                    entry.app_name = e.app_name
                    entry.field_name = e.fieldname
                    entry.value = e.value

                    entry.metadata = mobius.pod.map()
                    entry.metadata.set('evidence-source', e.evidence_source)
                    self.__entries.append(entry)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        transaction = self.__item.new_transaction()

        # save evidences
        for e in self.__entries:
            c = self.__item.new_evidence(EVIDENCE_TYPE)
            c.field_name = e.field_name
            c.value = e.value
            c.app_name = e.app_name
            c.username = e.username
            c.metadata = e.metadata

        # commit data
        transaction.commit()
