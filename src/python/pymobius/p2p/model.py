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
import datetime
import json
import os
import os.path
import traceback
import mobius
import pymobius.p2p.account
import pymobius.p2p.application
import pymobius.p2p.search
import pymobius.p2p.local_file
import pymobius.p2p.remote_file

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief P2P activity model class
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class model (object):

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief initialize object
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def __init__ (self, item):
    self.item = item
    self.applications = []
    self.accounts = []
    self.searches = []
    self.local_files = []
    self.remote_files = []
    
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief load model from .json file
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def load (self):
    rc = False
    path = self.item.get_data_path ('p2p.json')

    if os.path.exists (path):
      fp = open (path)
      data = json.load (fp, object_hook=json_decoder)
      fp.close ()
    
      self.applications = data.get ('applications')
      self.accounts = data.get ('accounts')
      self.searches = data.get ('searches')
      self.local_files = data.get ('local_files')
      self.remote_files = data.get ('remote_files')

      rc = True

    return rc

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief save model to .json file
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def save (self, path):
    path = self.item.get_data_path ('p2p.json')

    data = {'applications' : self.applications,
            'accounts' : self.accounts,
            'searches' : self.searches,
            'local_files' : self.local_files,
            'remote_files' : self.remote_files,
            }

    f = mobius.io.new_file_by_path (path)
    fp = mobius.io.text_writer (f.new_writer ())

    try:
      json.dump (data, fp, default=json_encoder)

    except Exception as e:
      mobius.core.logf ('WRN %s %s' % (str (e), traceback.format_exc ()))
      os.remove (path)
    
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief JSON encoder helper function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def json_encoder (obj):

  if isinstance (obj, datetime.datetime):
    data = {
            'class' : 'datetime',
            'data' : [obj.year, obj.month, obj.day, obj.hour, obj.minute, obj.second]
           }

  elif isinstance (obj, bytes):
    data = {
             'class' : 'bytes',
             'data': mobius.encoder.base64 (obj)
           }

  elif isinstance (obj, pymobius.p2p.account.account):
    data = {
            'class' : 'p2p.account',
            'app_id' : obj.app_id,
            'app' : obj.app,
            'username' : obj.username,
            'network' : obj.network,
            'guid' : obj.guid
           }

  elif isinstance (obj, pymobius.p2p.application.application):
    data = {
            'class' : 'p2p.application',
            'id' : obj.id,
            'name' : obj.name,
            'versions' : list (obj.versions),
            'total_searches' : obj.total_searches,
            'total_local_files' : obj.total_local_files,
            'total_remote_files' : obj.total_remote_files
           }

  elif isinstance (obj, pymobius.p2p.search.search):
    data = {
            'class' : 'p2p.search',
            'app_id' : obj.app_id,
            'app' : obj.app,
            'timestamp' : obj.timestamp,
            'username' : obj.username,
            'count' : obj.count,
            'text' : obj.text,
            'metadata' : list (obj.iter_metadata ()),
            'files' : list (obj.iter_files ())
           }

  elif isinstance (obj, pymobius.p2p.local_file.local_file):
    data = {
            'class' : 'p2p.local_file',
            'app_id' : obj.app_id,
            'app' : obj.app,
            'username' : obj.username,
            'name' : obj.name,
            'size' : obj.size,
            'path' : obj.path,
            'flag_shared' : obj.flag_shared,
            'flag_downloaded' : obj.flag_downloaded,
            'flag_uploaded' : obj.flag_uploaded,
            'flag_completed' : obj.flag_completed,
            'flag_corrupted' : obj.flag_corrupted,
            'metadata' : list (obj.get_metadata ()),
            'hashes' : obj.get_hashes ()
           }

  elif isinstance (obj, pymobius.p2p.remote_file.remote_file):
    data = {
            'class' : 'p2p.remote_file',
            'app_id' : obj.app_id,
            'app' : obj.app,
            'username' : obj.username,
            'name' : obj.name,
            'size' : obj.size,
            'timestamp' : obj.timestamp,
            'url' : obj.url,
            'preview_data' : obj.preview_data,
            'peer' : obj.peer,
            'metadata' : list (obj.get_metadata ()),
            'hashes' : obj.get_hashes ()
           }

  elif isinstance (obj, pymobius.p2p.remote_file.peer):
    data = {
            'class' : 'p2p.peer',
            'ip' : obj.ip,
            'port' : obj.port,
            'nickname' : obj.nickname,
            'country_code' : obj.country_code,
            'country' : obj.country,
            'application' : obj.application
           }

  else:
    raise TypeError ("%s type is not JSON serializable" % type (obj))

  return data

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief JSON decoder helper function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def json_decoder (dct):
  classname = dct.get ('class')

  if classname == 'datetime':
    args = dct.get ('data')
    data = datetime.datetime (*args)

  elif classname == 'bytes':
    data = mobius.decoder.base64 (dct.get ('data'))

  elif classname == 'p2p.account':
    data = pymobius.p2p.account.account ()
    data.app_id = dct.get ('app_id')
    data.app = dct.get ('app')
    data.username = dct.get ('username')
    data.network = dct.get ('network')
    data.guid = dct.get ('guid')

  elif classname == 'p2p.application':
    data = pymobius.p2p.application.application ()
    data.id = dct.get ('id')
    data.name = dct.get ('name')
    data.versions = set (dct.get ('versions'))
    data.total_searches = dct.get ('total_searches')
    data.total_local_files = dct.get ('total_local_files')
    data.total_remote_files = dct.get ('total_remote_files')

  elif classname == 'p2p.local_file':
    data = pymobius.p2p.local_file.local_file ()
    data.app_id = dct.get ('app_id')
    data.app = dct.get ('app')
    data.username = dct.get ('username')
    data.name = dct.get ('name')
    data.size = dct.get ('size')
    data.path = dct.get ('path')
    data.flag_shared = dct.get ('flag_shared')
    data.flag_downloaded = dct.get ('flag_downloaded')
    data.flag_uploaded = dct.get ('flag_uploaded')
    data.flag_completed = dct.get ('flag_completed')
    data.flag_corrupted = dct.get ('flag_corrupted')
    
    for name, value in dct.get ('hashes'):
      data.set_hash (name, value)
      
    for name, value in dct.get ('metadata'):
      data.add_metadata (name, value)

  elif classname == 'p2p.peer':
    data = pymobius.p2p.remote_file.peer ()
    data.ip = dct.get ('ip')
    data.port = dct.get ('port')
    data.nickname = dct.get ('nickname')
    data.country_code = dct.get ('country_code')
    data.country = dct.get ('country')
    data.application = dct.get ('application')
    
  elif classname == 'p2p.remote_file':
    data = pymobius.p2p.remote_file.remote_file ()
    data.app_id = dct.get ('app_id')
    data.app = dct.get ('app')
    data.username = dct.get ('username')
    data.name = dct.get ('name')
    data.size = dct.get ('size')
    data.timestamp = dct.get ('timestamp')
    data.url = dct.get ('url')
    data.preview_data = dct.get ('preview_data')
    data.peer = dct.get ('peer')
    
    for name, value in dct.get ('hashes'):
      data.set_hash (name, value)
      
    for name, value in dct.get ('metadata'):
      data.add_metadata (name, value)

  elif classname == 'p2p.search':
    data = pymobius.p2p.search.search ()
    data.app_id = dct.get ('app_id')
    data.app = dct.get ('app')
    data.timestamp = dct.get ('timestamp')
    data.username = dct.get ('username')
    data.text = dct.get ('text')
    data.count = dct.get ('count')
    
    for name, value in dct.get ('metadata', []):
      data.add_metadata (name, value)
      
    for f in dct.get ('files', []):
      data.add_file (f)

  else:
    data = dct
    
  return data
