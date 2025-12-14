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
import mobius.core.crypt
import pymobius.registry
import pymobius.registry.installed_programs
import pymobius.registry.main


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Scan installed operating systems
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def scan(item):
    # get item datasource
    datasource = item.get_datasource()

    if not datasource:
        raise Exception("Datasource is not set")

    if not datasource.is_available():
        raise Exception("Datasource is not available")

    # scan datasource
    return [os_win(datasource, item)]


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Safe file wrapper (no exceptions)
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_wrapper(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, f, item):
        self.__f = f
        self.__item = item

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get attributes from file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __getattr__(self, name):
        return self.__dict__.get(name, getattr(self.__f, name))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set file handled
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_handled(self):
        if not self.__f:
            return

        reader = self.__f.new_reader()
        if not reader:
            return

        # calculate hash sha2-512
        h = mobius.core.crypt.hash('sha2-512')
        data = reader.read(65536)

        while data:
            h.update(data)
            data = reader.read(65536)

        # add to kff file
        case = self.__item.case
        path = case.create_path(os.path.join('hashset', f'{self.__item.uid:04d}-handled.ignore'))

        fp = open(path, 'a')
        fp.write('%s\n' % h.get_hex_digest())
        fp.close()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Safe folder wrapper (no exceptions)
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class folder_wrapper(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, folder, item, case_sensitive=True):
        self.__folder = folder
        self.__item = item
        self.__case_sensitive = case_sensitive

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get attributes from folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __getattr__(self, name):
        return self.__dict__.get(name, getattr(self.__folder, name))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get children
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_children(self):
        children = []

        try:
            for child in self.__folder.get_children():
                if child.is_folder():
                    f = folder_wrapper(child, self.__item, self.__case_sensitive)
                else:
                    f = file_wrapper(child, self.__item)
                children.append(f)

        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

        return children

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get child by name
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_child_by_name(self, name, case_sensitive=False):
        child = None

        try:
            child = self.__folder.get_child_by_name(name, self.__case_sensitive)

            if not child:
                pass
            elif child.is_folder():
                child = folder_wrapper(child, self.__item, self.__case_sensitive)
            else:
                child = file_wrapper(child, self.__item)

        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

        return child

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get child by path
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_child_by_path(self, path, case_sensitive=False):
        child = None

        try:
            child = self.__folder.get_child_by_path(path, self.__case_sensitive)

            if not child:
                pass

            elif child.is_folder():
                child = folder_wrapper(child, self.__item, self.__case_sensitive)

            else:
                child = file_wrapper(child, self.__item)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

        return child


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief OS Windows object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class os_win(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, datasource, item):
        self.__datasource = datasource
        self.__vfs = datasource.get_vfs()
        self.__installed_programs = []
        self.__installed_programs_loaded = False
        self.__item = item

        # retrieve registry object
        ant = pymobius.registry.main.Ant(item)
        data = ant.get_data()

        if not data:
            raise Exception('OS.Win has no registry')

        elif len(data) == 1:
            self.__registry = data[0]

        else:
            mobius.core.logf(f'INF Item with {len(data):d} registries')
            self.__registry = data[0]

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get item
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_item(self):
        return self.__item

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get registry object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_registry(self):
        return self.__registry

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get system folder path
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_folder(self, name):
        return pymobius.registry.get_system_folder(self.__registry, name)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get System root folders
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_root_folders(self):
        return [folder_wrapper(entry, self.__item, False) for entry in self.__vfs.get_root_entries() if
                entry.is_folder()]

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get installed programs
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_installed_programs(self):
        if not self.__installed_programs_loaded:
            self.__installed_programs = pymobius.registry.installed_programs.get(self.__registry)
            self.__installed_programs_loaded = True

        return self.__installed_programs

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get system profiles
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_profiles(self):

        # get profiles path
        profiles_path = self.get_folder('PROFILES') or 'C:\\Users'
        profiles_path = profiles_path[3:].replace('\\', '/')

        # retrieve profiles
        for entry in self.__vfs.get_root_entries():
            if entry.is_folder():
                root_wrapper = folder_wrapper(entry, self.__item, False)
                profile_folder = root_wrapper.get_child_by_path(profiles_path)

                if profile_folder:
                    for child in profile_folder.get_children():
                        if not child.is_reallocated() and child.is_folder():
                            yield os_win_profile(self, child)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief OS Windows profile object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class os_win_profile(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, operating_system, folder):
        self.__folder = folder
        self.__operating_system = operating_system
        self.__ntuser_dat = None
        self.folder = folder
        self.path = folder.path.replace('/', '\\')
        self.username = folder.path.rsplit('/', 1)[-1]
        self.is_deleted = folder.is_deleted()

        self.__control_folders = {
            'appdata': 'AppData/Roaming',
            'localappdata': 'AppData/Local',
            'cookies': 'AppData/Roaming/Microsoft/Windows/Cookies',
            'desktop': 'Desktop',
            'downloads': 'Downloads',
            'favorites': 'Favorites',
        }

        # get NTUSER.DAT root key
        nt_path = folder.path + '/NTUSER.DAT'
        nt_path = nt_path.replace('/', '\\').lower()

        registry = operating_system.get_registry()

        for f in registry.files:
            if f.role == 'NTUSER' and f.path.lower() == nt_path:
                self.__ntuser_dat = f.root_key

        # get user shell folders
        if self.__ntuser_dat:
            for value in self.__ntuser_dat.get_value_by_mask(
                    'Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders\\*'):
                path = value.data.get_data_as_string('utf-16')
                if path.startswith('%USERPROFILE%\\'):
                    folder_id = value.name.lower().replace(' ', '')
                    self.__control_folders[folder_id] = path[14:].replace('\\', '/')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get filesystem entry by path
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_entry_by_path(self, path):
        rpath = path

        if path.startswith('%'):
            pos = path.find('%', 1)

            if pos != -1:
                folder_id = path[1:pos]
                folder_path = self.__control_folders.get(folder_id)

                if folder_path:
                    rpath = folder_path + path[pos + 1:]

        try:
            f = self.__folder.get_child_by_path(rpath)
        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")
            f = None

        return f

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get NTUSER.dat root key
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_hkey(self):
        return self.__ntuser_dat

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get AppData folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_appdata_folder(self, rpath):
        appdata = self.__control_folders.get('appdata', 'AppData/Roaming')

        if rpath:
            appdata = appdata + '/' + rpath

        folder = None

        try:
            folder = self.__folder.get_child_by_path(appdata)
        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")
            folder = None

        return folder

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get AppData entry
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_appdata_entry(self, rpath):
        appdata = self.__control_folders.get('appdata', 'AppData/Roaming')

        if rpath:
            appdata = appdata + '/' + rpath

        entry = None

        try:
            entry = self.__folder.get_child_by_path(appdata)
        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

        return entry

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get AppDataLocal folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_appdata_local_folder(self, rpath):
        appdata = self.__control_folders.get('local appdata', 'AppData/Local')

        if rpath:
            appdata = appdata + '/' + rpath

        folder = None

        try:
            folder = self.__folder.get_child_by_path(appdata)
        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")
            folder = None

        return folder

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get control folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_control_folder(self, folder_id, folder_default):
        rpath = self.__control_folders.get(folder_id, folder_default)
        folder = None

        try:
            folder = self.__folder.get_child_by_path(rpath)
        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")
            folder = None

        return folder
