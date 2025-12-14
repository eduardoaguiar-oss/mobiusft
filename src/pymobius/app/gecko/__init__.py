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
import os.path
import tempfile
import pymobius.operating_system
import pymobius.app.gecko.profile
import mobius

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Gecko based browsers
# Columns: app_id, app_name, profiles path, multi-profile (True/False)
# @see https://www.pje.jus.br/wiki/index.php/Navegador_PJe
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
GECKO_APPS = [
  ('firefox.bcltmp', 'Firefox from Avast Browser Cleanup', '%localappdata%/Temp/BCLTMP/firefox', True),
  ('firefox', 'Firefox', '%appdata%/Mozilla/Firefox/Profiles', True),
  ('firefox.portable', 'Firefox Portable', '%localappdata%/Temp/FirefoxPortable/profile', False),
  ('geckofx', 'GeckoFX', '%localappdata%/GeckoFX', True),
  ('navegadorpje', 'Navegador PJe', '%root%/NavegadorPJe/Firefox/Data/profile', False),
  ('netexpress', 'Bradesco Net Express', '%appdata%/NetExpress60/Navegador/Profiles', True),
]

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Gecko based model class
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class model (object):

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Initialize object
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def __init__ (self, item):
    self.__item = item

    self.__profiles = []
    self.__profiles_loaded = False

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Get profiles
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def get_profiles (self):
    if not self.__profiles_loaded:
      self.__load_profiles ()

    return self.__profiles

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load profiles from disk
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def __load_profiles (self):
    for opsys in pymobius.operating_system.scan (self.__item):
      for user_profile in opsys.get_profiles ():
        self.__retrieve_from_user_profile (user_profile)

      for folder in opsys.get_root_folders ():
        self.__retrieve_from_root_folder (folder)

    self.__profiles_loaded = True

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load profiles from root folder
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def __retrieve_from_root_folder (self, root_folder):
    apps = [ row for row in GECKO_APPS if row[2].startswith ('%root%') ]

    for app_id, app_name, app_path, flag_multiprofile in apps:
      folder = root_folder.get_child_by_path (app_path[7:])

      if folder:
        if flag_multiprofile:
          self.__load_app_profiles (folder, app_id, app_name, '')
        else:
          self.__load_app_profile (folder, app_id, app_name, '')

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load profiles from user profile dir
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def __retrieve_from_user_profile (self, user_profile):
    apps = [ row for row in GECKO_APPS if row[2].startswith ('%localappdata%') or row[2].startswith ('%appdata%') ]

    for app_id, app_name, app_path, flag_multiprofile in apps:
      folder = user_profile.get_entry_by_path (app_path)

      if folder:
        if flag_multiprofile:
          self.__load_app_profiles (folder, app_id, app_name, user_profile.username)
        else:
          self.__load_app_profile (folder, app_id, app_name, user_profile.username)

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load profiles from app profile folder
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def __load_app_profiles (self, folder, app_id, app_name, username):
    for child in folder.get_children ():
      try:
        if not child.is_reallocated () and child.is_folder ():
          self.__load_app_profile (child, app_id, app_name, username)
      except Exception as e:
        mobius.core.logf ('WRN app.gecko: %s' % str (e))

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load profile app profile folder
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def __load_app_profile (self, folder, app_id, app_name, username):
    if not any (folder.get_child_by_name ('%s.sqlite' % name) for name in ['cookies', 'places', 'downloads', 'formhistory', 'permissions']):
      return

    try:
      p = pymobius.app.gecko.profile.Profile (folder)
      p.app_id = app_id
      p.app_name = app_name
      p.username = username
      p.creation_time = folder.creation_time

      if folder.name == 'profile':
        p.name = 'Default'
      else:
        p.name = folder.name

      self.__profiles.append (p)

    except Exception as e:
      mobius.core.logf ('WRN app.gecko: %s' % str (e))

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve file into temp directory
# @param f File
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve_db_file (f):

  if not f or f.is_reallocated ():
    return

  reader = f.new_reader ()
  if not reader:
    return

  # create temporary .sqlite local file
  ext = os.path.splitext (f.name)[1]
  fd, path = tempfile.mkstemp (suffix=ext)

  f = mobius.core.io.new_file_by_path (path)
  fp = f.new_writer ()
  fp.write (reader.read ())

  return path
