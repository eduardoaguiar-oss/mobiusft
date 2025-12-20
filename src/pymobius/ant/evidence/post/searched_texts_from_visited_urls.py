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
import os.path
import traceback

import mobius
import pymobius

ANT_ID = 'search-texts-from-visited-urls'
ANT_NAME = 'Searched Texts from Visited URLs'
ANT_VERSION = '1.0'
EVIDENCE_TYPE = 'searched-text'

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Load URL search data from url_text_search file
# WWW_QUERIES: (Host, path) -> query variables ('*' = any host)
# WWW_PATHS: Host -> [(start1, end1)...(startn,endn)]
# WWW_FRAGMENTS: (Host, path) -> fragment variables
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
WWW_QUERIES = {}
WWW_PATHS = {}
WWW_FRAGMENTS = {}

app = mobius.core.application()
path = app.get_data_path('data/url_text_search.txt')

if os.path.exists(path):
    group = None
    fp = open(path, encoding='utf-8')

    for line in fp:
        line = line.rstrip()

        if line.startswith('['):
            group = line[1:-1]

        elif line:

            if group == 'query':
                host, path, var = line.split('\t')
                WWW_QUERIES.setdefault((host, path), []).append(var)

            elif group == 'path':
                data = line.split('\t')
                host = data[0]
                s = data[1]

                if len(data) > 2:
                    e = data[2]
                else:
                    e = ''

                WWW_PATHS.setdefault(host, []).append((s, e))

            if group == 'fragment':
                host, path, var = line.split('\t')
                WWW_FRAGMENTS.setdefault((host, path), []).append(var)

    fp.close()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Parse URL query
# @param q Query
# @return var/value dict
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def parse_query(q):
    values = {}

    for v in q.split('&'):
        try:
            if '=' in v:
                name, value = v.split('=', 1)
                value = value.replace('+', ' ')
                values[name] = value
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    return values


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: Searched Texts from Visited URLs
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
        self.__retrieve_browser_history()
        self.__save_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from browser history
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_browser_history(self):

        # retrieve data
        for h in self.__item.get_evidences('visited-url'):
            uri = mobius.core.io.uri(h.url)
            host = uri.get_host()

            text = (self.__retrieve_browser_history_fragment(h) or
                    self.__retrieve_browser_history_query(h) or
                    self.__retrieve_browser_history_path(h))

            if text:
                ts = pymobius.Data()
                ts.timestamp = h.timestamp
                ts.type = 'web/' + host
                ts.text = text
                ts.username = h.username
                ts.evidence_source = h.evidence_source

                ts.metadata = mobius.core.pod.map()
                ts.metadata.set('url', h.url)
                ts.metadata.set('host', host)
                ts.metadata.update(h.metadata)

                self.__entries.append(ts)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from browser history using subpath
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_browser_history_path(self, h):
        text = ''

        try:
            uri = mobius.core.io.uri(h.url)
            host = uri.get_host()

            try:
                path = uri.get_path('utf-8')
            except UnicodeDecodeError:
                path = uri.get_path('iso-8859-1')

            for l, r in WWW_PATHS.get(host, []):
                if path.startswith(l):
                    if r:
                        if path.endswith(r):
                            text = path[len(l):-len(r)]
                    else:
                        pos = path.find('/', len(l))

                        if pos == -1:
                            text = path[len(l):]
                        else:
                            text = path[len(l):pos]

                if text:
                    text = text.replace('+', ' ')
                    break
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

        return text

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from browser history using query variables
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_browser_history_query(self, h):
        text = ''

        try:
            uri = mobius.core.io.uri(h.url)
            host = uri.get_host()

            try:
                path = uri.get_path('utf-8')
            except UnicodeDecodeError:
                path = uri.get_path('iso-8859-1')

            try:
                query = uri.get_query('utf-8')
            except UnicodeDecodeError:
                query = uri.get_query('iso-8859-1')

            if query and ((host, path) in WWW_QUERIES or ('*', path) in WWW_QUERIES):
                qvars = WWW_QUERIES.get((host, path)) or WWW_QUERIES.get(('*', path)) or []
                values = parse_query(query)

                for var in qvars:
                    text = text or values.get(var)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

        return text

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from browser history using fragment variables
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_browser_history_fragment(self, h):
        text = ''

        try:
            uri = mobius.core.io.uri(h.url)
            host = uri.get_host()

            try:
                path = uri.get_path('utf-8')
            except UnicodeDecodeError:
                path = uri.get_path('iso-8859-1')

            try:
                fragment = uri.get_fragment('utf-8')
            except UnicodeDecodeError:
                fragment = uri.get_fragment('iso-8859-1')

            if fragment and ((host, path) in WWW_FRAGMENTS or ('*', path) in WWW_FRAGMENTS):
                qvars = WWW_FRAGMENTS.get((host, path)) or WWW_FRAGMENTS.get(('*', path)) or []
                values = parse_query(fragment)

                for var in qvars:
                    text = text or values.get(var)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

        return text

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        transaction = self.__item.new_transaction()

        # save text searches
        for ts in self.__entries:
            text_search = self.__item.new_evidence(EVIDENCE_TYPE)
            text_search.timestamp = ts.timestamp
            text_search.search_type = ts.type
            text_search.text = ts.text
            text_search.username = ts.username
            text_search.metadata = ts.metadata

        # commit data
        transaction.commit()
