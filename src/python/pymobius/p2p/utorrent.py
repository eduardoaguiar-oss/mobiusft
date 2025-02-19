# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import traceback
import mobius
import pymobius.app.utorrent
import pymobius.p2p.account
import pymobius.p2p.application
import pymobius.p2p.local_file
import pymobius.p2p.remote_file

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from uTorrent
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve (model):
  try:
    app_model = pymobius.app.utorrent.model (model.item)
    retrieve_utorrent_application_data (model, app_model)
    retrieve_utorrent_profiles_data (model, app_model)

  except Exception as e:
    mobius.core.logf (f'WRN {str(e)}\n{traceback.format_exc()}')

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve application data from uTorrent
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve_utorrent_application_data (model, app_model):
  try:
    application = pymobius.p2p.application.application ()
    application.id = 'utorrent'
    application.name = 'µTorrent'
    application.versions = app_model.get_installed_versions ()
    model.applications.append (application)

  except Exception as e:
    mobius.core.logf (f'WRN {str(e)}\n{traceback.format_exc()}')

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from uTorrent profiles
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve_utorrent_profiles_data (model, app_model):
  try:
    for profile in app_model.get_profiles ():
      retrieve_utorrent_profile_data (model, profile)

  except Exception as e:
    mobius.core.logf (f'WRN {str(e)}\n{traceback.format_exc()}')

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from uTorrent profile
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve_utorrent_profile_data (model, profile):
  try:
    retrieve_utorrent_accounts (model, profile)
    retrieve_utorrent_local_files (model, profile)
    retrieve_utorrent_remote_files (model, profile)

  except Exception as e:
    mobius.core.logf (f'WRN {str(e)}\n{traceback.format_exc()}')
	      
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve accounts from uTorrent profile
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve_utorrent_accounts (model, profile):
  try:
    for a in profile.get_accounts ():
      account = pymobius.p2p.account.account ()
      account.username = a.username
      account.app_id = a.app_id
      account.app = a.app_name
      account.network = 'BitTorrent'
      account.guid = a.guid
      model.accounts.append (account)
  except Exception as e:
    mobius.core.logf (f'WRN {str(e)}\n{traceback.format_exc()}')

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve local files from uTorrent profile
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve_utorrent_local_files (model, profile):
  try:
    for f in profile.get_local_files ():
      lf = pymobius.p2p.local_file.local_file ()
      lf.app_id = f.app_id
      lf.app = f.app_name
      lf.path = f.path
      lf.size = f.size
      lf.name = f.name or f.caption
      lf.username = f.username
      
      lf.flag_downloaded = f.flag_downloaded
      lf.flag_uploaded = f.flag_uploaded
      lf.flag_shared = f.flag_shared
      lf.flag_completed = f.is_completed
      lf.flag_corrupted = pymobius.p2p.STATE_UNKNOWN
      
      lf.add_metadata ('Added date/time', f.added_time)
      lf.add_metadata ('Completed date/time', f.completed_time)
      lf.add_metadata ('Last metadata saving date/time', f.metadata_time)
      lf.add_metadata ('Last seen complete date/time', f.last_seen_complete_time)
      lf.add_metadata ('Download URL', f.download_url)
      lf.add_metadata ('Caption', f.caption)
      lf.add_metadata ('Resume.dat path', f.resume_dat_path)
      lf.add_metadata ('Resume.dat modification date/time', f.resume_dat_modification_time)
      lf.add_metadata ('.Torrent path', f.torrent_path)

      model.local_files.append (lf)

  except Exception as e:
    mobius.core.logf (f'WRN {str(e)}\n{traceback.format_exc()}')

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve remote files from uTorrent profile
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve_utorrent_remote_files (model, profile):
  try:
    for f in profile.get_remote_files ():
      if f.ip != '127.0.0.1':
        rf = pymobius.p2p.remote_file.remote_file ()
        rf.app_id = f.app_id
        rf.app = f.app_name
        rf.path = f.path
        rf.size = f.size
        rf.name = f.name or f.caption
        rf.username = f.username
        rf.timestamp = f.timestamp
        rf.peer.ip = f.ip
        rf.peer.port = f.port
      
        rf.flag_downloaded = f.flag_downloaded
        rf.flag_uploaded = f.flag_uploaded
        rf.flag_shared = f.flag_shared
        rf.flag_completed = f.is_completed
        rf.flag_corrupted = pymobius.p2p.STATE_UNKNOWN
      
        rf.add_metadata ('Local path', f.path)
        rf.add_metadata ('Added date/time', f.added_time)
        rf.add_metadata ('Completed date/time', f.completed_time)
        rf.add_metadata ('Last metadata saving date/time', f.metadata_time)
        rf.add_metadata ('Last seen complete date/time', f.last_seen_complete_time)
        rf.add_metadata ('Download URL', f.download_url)
        rf.add_metadata ('Caption', f.caption)
        rf.add_metadata ('Resume.dat path', f.resume_dat_path)
        rf.add_metadata ('Resume.dat modification date/time', f.resume_dat_modification_time)
        rf.add_metadata ('.Torrent path', f.torrent_path)

        model.remote_files.append (rf)
  except Exception as e:
    mobius.core.logf (f'WRN {str(e)}\n{traceback.format_exc()}')
