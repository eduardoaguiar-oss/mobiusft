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
import pymobius.operating_system
import pymobius.app.skype.app.profile
import pymobius.app.skype.app_v14.profile

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Skype model class
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
  # @brief Load profiles
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def __load_profiles (self):
    for opsys in pymobius.operating_system.scan (self.__item):
      for user_profile in opsys.get_profiles ():
        # Skype App
        folder = user_profile.get_entry_by_path ('%localappdata%/Packages/Microsoft.SkypeApp_kzf8qxf38zg5c/LocalState')
        self.__load_profiles_from_skype_app_folder (folder, user_profile.username)

    self.__profiles_loaded = True

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load profiles from SkypeApp folder
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def __load_profiles_from_skype_app_folder (self, folder, username):
    if not folder:
      return

    for child in folder.get_children ():
      if child.is_reallocated ():
        pass

      elif child.is_folder ():
        skype_db = child.get_child_by_name ('skype.db')

        if skype_db:
          p = pymobius.app.skype.app.profile.Profile (child, self.__item)
          p.username = username
          self.__profiles.append (p)

      elif child.name.startswith ('s4l-') and child.name.endswith ('.db'):
        p = pymobius.app.skype.app_v14.profile.Profile (child, self.__item)
        p.username = username
        self.__profiles.append (p)
