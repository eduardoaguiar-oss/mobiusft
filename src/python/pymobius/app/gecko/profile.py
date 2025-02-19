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
import pymobius
import mobius
from . import decoder_cookies
from . import decoder_downloads
from . import decoder_formhistory
from . import decoder_places

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Gecko based profile class
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Profile (object):

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Initialize object
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def __init__ (self, folder):
    self.app_id = None
    self.app_name = None
    self.creation_time = folder.creation_time
    self.__cookies_loaded = False
    self.__downloads_loaded = False
    self.__form_history_loaded = False
    self.__places_loaded = False

    self.__cookies = []
    self.__downloads = []
    self.__form_history = []
    self.__history = []
    self.__searches = []
    self.__bookmarks = []

    # beta.util.dirtree (folder)	# development only

    self.__roaming_folder = folder
    self.path = mobius.io.to_win_path (folder.path)

    if folder.name.lower ().endswith ('.default'):
      self.name = folder.name[:-8]

    else:
      self.name = folder.name

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Get cookies from cookies.sqlite
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def get_cookies (self):

    if not self.__cookies_loaded:
      f = self.__roaming_folder.get_child_by_path ('cookies.sqlite')

      if f and not f.is_reallocated ():
        data = decoder_cookies.decode (f)
        self.__cookies = data.cookies

      self.__cookies_loaded = True

    return self.__cookies

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Get download history from downloads.sqlite
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def get_downloads (self):

    if not self.__downloads_loaded:
      f = self.__roaming_folder.get_child_by_path ('downloads.sqlite')

      if f and not f.is_reallocated ():
        data = decoder_downloads.decode (f)
        self.__downloads = data.downloads

      self.__downloads_loaded = True

    return self.__downloads

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Get form history
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def get_form_history (self):

    if not self.__form_history_loaded:
      self.__load_form_history ()

    return self.__form_history

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Get search history
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def get_searches (self):

    if not self.__form_history_loaded:
      self.__load_form_history ()

    return self.__searches

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Get URL history
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def get_history (self):
    if not self.__places_loaded:
      self.__load_places ()

    return self.__history

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Get bookmarks
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def get_bookmarks (self):
    if not self.__places_loaded:
      self.__load_places ()

    return self.__bookmarks

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Retrieve data from places.sqlite file
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def __load_places (self):
    if self.__places_loaded:
      return

    self.__history = []
    self.__bookmarks = []

    f = self.__roaming_folder.get_child_by_path ('places.sqlite')

    if f and not f.is_reallocated ():
      data = decoder_places.decode (f)

      for h in data.history:
        h.profile_path = self.path
        h.profile_name = self.name
        h.username = self.username
        h.app_id = self.app_id
        h.app_name = self.app_name
        self.__history.append (h)

      for b in data.bookmarks:
        b.profile_path = self.path
        b.profile_name = self.name
        b.username = self.username
        b.app_id = self.app_id
        b.app_name = self.app_name
        self.__bookmarks.append (b)

    self.__places_loaded = True

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load form history from formhistory.sqlite
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def __load_form_history (self):
    f = self.__roaming_folder.get_child_by_path ('formhistory.sqlite')

    if f and not f.is_reallocated ():
      self.__form_history = []
      self.__searches = []

      for entry in decoder_formhistory.decode (f):

        if entry.fieldname == 'searchbar-history':
          search = pymobius.Data ()
          search.id = entry.id
          search.text = entry.value
          search.timestamp = entry.first_used_time
          self.__searches.append (search)

          if entry.last_used_time and entry.first_used_time != entry.last_used_time:
            search = pymobius.Data ()
            search.id = entry.id
            search.text = entry.value
            search.timestamp = entry.last_used_time
            self.__searches.append (search)

        else:
          self.__form_history.append (entry)

    self.__form_history_loaded = True

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Get any file (development only)
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def get_file (self, f):
    if not f:
      return

    reader = f.new_reader ()
    if not reader:
      return

    # create temporary .sqlite local file
    fp = open (f.name, 'wb')
    fp.write (reader.read ())
    fp.close ()
