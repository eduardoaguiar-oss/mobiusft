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
import mobius
import pymobius
import pymobius.app.internet_explorer.decoder_cookies


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Microsoft Edge Profile class
# @author Eduardo Aguiar
# @see https://www.dataforensics.org/microsoft-edge-browser-forensics/
# @see https://www.forensafe.com/blogs/microsoftedge.html
# @see https://www.forensicscijournal.com/articles/jfsr-aid1022.pdf
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Profile(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, folder, item, username):
        self.app_id = 'edge'
        self.app_name = 'Microsoft Edge'
        self.path = mobius.core.io.to_win_path(folder.path)
        self.username = username
        self.creation_time = folder.creation_time
        self.name = username

        self.__profile_folder = folder
        self.__item = item

        self.__cookies = []
        self.__history = []
        self.__downloads = []
        self.__favorites = []

        self.__cookies_loaded = False
        self.__history_loaded = False
        self.__downloads_loaded = False
        self.__favorites_loaded = False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get history
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_history(self):
        if not self.__history_loaded:
            self.__load_history()

        return self.__history

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get cookies
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_cookies(self):
        if not self.__cookies_loaded:
            self.__load_cookies()

        return self.__cookies

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get form history
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_form_history(self):
        return []

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get downloads
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_downloads(self):
        if not self.__downloads_loaded:
            self.__load_downloads()

        return self.__downloads

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get favorites
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_favorites(self):
        if not self.__favorites_loaded:
            self.__load_favorites()

        return self.__favorites

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve URL history from history files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_history(self):
        self.__history = []

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve cookies from cookies files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_cookies(self):
        self.__cookies_loaded = True
        self.__cookies = []

        # MicrosoftEdge/Cookies folder
        folder = self.__profile_folder.get_child_by_path('MicrosoftEdge/Cookies')
        self.__cookies += self.__load_cookies_folder(folder)

        # #!XXX folders
        for child in self.__profile_folder.get_children():

            if child.name.startswith('#!'):
                folder = child.get_child_by_path('MicrosoftEdge/Cookies')
                self.__cookies += self.__load_cookies_folder(folder)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Scan cookies folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_cookies_folder(self, folder):
        cookies = []

        if folder:
            for child in folder.get_children():
                if child.is_reallocated():
                    pass

                elif child.is_folder():
                    cookies += self.__load_cookies_folder(child)

                else:
                    cookies += self.__load_cookies_from_file(child)

        return cookies

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse files inside cookies folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_cookies_from_file(self, f):
        cookies = []
        lname = f.name.lower()

        if lname.endswith('.cookie') or lname.endswith('.txt'):
            c = pymobius.app.internet_explorer.decoder_cookies.decode(f)

            if c != None:
                cookies += c
                f.set_handled()

        else:
            mobius.core.logf('DEV Unhandled file ' + f.path)

        return cookies

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve download history
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_downloads(self):
        self.__downloads_loaded = True
        self.__downloads = []
