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
import shutil
import traceback

import mobius
import mobius.core.crypt
import mobius.core.os
import pymobius

ANT_ID = 'registry'
ANT_NAME = 'Registry Ant'
ANT_VERSION = '1.1'
ATTR_NAME = 'data.registry'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get folder's child by name
# @param folder folder
# @param name child's name
# @return child or None
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_child_by_name(folder, name):
    child = None

    try:
        child = folder.get_child_by_name(name, False)
    except Exception as e:
        mobius.core.logf('DBG ' + str(e))

    return child


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get folder's child by path
# @param folder folder
# @param path child's path
# @return child or None
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_child_by_path(folder, path):
    child = None

    try:
        child = folder.get_child_by_path(path, False)
    except Exception as e:
        mobius.core.logf('DBG ' + str(e))

    return child


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get folder's children
# @param folder folder
# @return children
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_children(folder):
    children = []

    try:
        children = folder.get_children()
    except Exception as e:
        mobius.core.logf('DBG ' + str(e))

    return children


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief check if entry is folder
# @param entry entry
# @return true/false
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def is_folder(entry):
    rc = False

    try:
        rc = entry.is_folder()
    except Exception as e:
        mobius.core.logf('DBG ' + str(e))

    return rc


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief forensics: win registry
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.__item = item
        self.__data = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get registry data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_data(self):
        data = []
        reg_list = []

        # if ant has run, get data
        if self.__item.has_ant(ANT_ID):
            reg_list = self.__item.get_attribute(ATTR_NAME)

        # no data yet, retrieves data from disk
        elif self.__item.has_datasource():
            reg_list = self.retrieve_data()

        # no datasource, removes old cached data
        else:
            self.remove_data()
            return data

        # builds registry list
        for r in reg_list:
            registry = mobius.core.os.win.registry.registry()

            for hivefile in r.hivefiles:
                try:
                    localpath = self.__item.get_data_path(os.path.join('registry', '%04d' % r.idx, hivefile.filename))
                    registry.add_file_by_path(hivefile.role, hivefile.path, localpath)
                except Exception as e:
                    mobius.core.logf('WRN %s %s' % (str(e), traceback.format_exc()))

            data.append(registry)

        return data

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve artifacts
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def retrieve_data(self):
        mobius.core.logf('DBG pymobius.registry.retrieve_data (1)')

        # get item VFS
        datasource = self.__item.get_datasource()
        if not datasource:
            raise Exception("No datasource set")

        if not datasource.is_available():
            raise Exception("Datasource is not available")

        if datasource.get_type() != 'vfs':
            raise Exception("Datasource type is not VFS")

        vfs = datasource.get_vfs()

        # retrieve artifacts
        mobius.core.logf('DBG pymobius.registry.retrieve_data (2)')
        next_idx = 1
        registry = None
        hashes = set()
        reg_list = []
        user_hivefiles = []

        # scan VFS root entries
        mobius.core.logf('DBG pymobius.registry.retrieve_data (3)')

        for root_entry in vfs.get_root_entries():
            mobius.core.logf('DBG pymobius.registry.retrieve_data (3.1)')

            if root_entry.is_folder():

                # system entries
                system_hivefiles = self.__get_system_hivefiles(root_entry)
                mobius.core.logf('DBG pymobius.registry.retrieve_data (3.2) %d' % len(system_hivefiles))

                if system_hivefiles:
                    registry = pymobius.Data()
                    registry.idx = next_idx
                    registry.hivefiles = system_hivefiles
                    reg_list.append(registry)
                    next_idx += 1

                    if user_hivefiles:  # previously found NTUSER hivefiles
                        registry.hivefiles.extend(user_hivefiles)
                        user_hivefiles = []

                # user hivefiles
                user_hivefiles = self.__get_user_hivefiles(root_entry)

                if user_hivefiles and registry:
                    registry.hivefiles.extend(user_hivefiles)
                    user_hivefiles = []

        # retrieve hivefiles
        mobius.core.logf('DBG pymobius.registry.retrieve_data (4)')

        for registry in reg_list:
            for hivefile in registry.hivefiles:
                hash_sha2_512 = self.__retrieve_file(hivefile.entry, registry.idx, hivefile.filename)

                if hash_sha2_512:
                    hashes.add(hash_sha2_512)
                del hivefile.entry

        # save data model
        mobius.core.logf('DBG pymobius.registry.retrieve_data (5)')
        self.__item.set_attribute(ATTR_NAME, reg_list)
        self.__item.set_ant(ANT_ID, ANT_NAME, ANT_VERSION)

        # save KFF
        mobius.core.logf('DBG pymobius.registry.retrieve_data (6)')
        if hashes:
            case = self.__item.case
            path = case.create_path(os.path.join('hashset', '%04d-registry.ignore' % self.__item.uid))

            fp = open(path, 'w')
            for h in hashes:
                fp.write('%s\n' % h)
            fp.close()

        mobius.core.logf('DBG pymobius.registry.retrieve_data (7)')
        return reg_list

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Remove registry data for an item
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def remove_data(self):
        self.__item.remove_attribute(ATTR_NAME)
        self.__item.reset_ant(ANT_ID)

        # data/registry
        path = self.__item.get_data_path('registry')
        if os.path.exists(path):
            shutil.rmtree(path)

        # .ignore
        case = self.__item.case
        path = case.get_path(os.path.join('hashset', '%04d-registry.ignore' % self.__item.uid))

        if os.path.exists(path):
            os.remove(path)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get system hivefiles
    # @param filesystem's root entry
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_system_hivefiles(self, root):
        mobius.core.logf('DBG pymobius.registry.__get_system_hivefiles (1) %s' % root.path)
        entries = []

        # %windir%/system32/config
        config_dir = get_child_by_path(root, 'windows/system32/config')
        mobius.core.logf('DBG pymobius.registry.__get_system_hivefiles (2) %s' % config_dir)

        if config_dir:
            components = get_child_by_name(config_dir, 'components')
            mobius.core.logf('DBG pymobius.registry.__get_system_hivefiles (3) %s' % components)
            if components:
                entries.append(components)

            sam = get_child_by_name(config_dir, 'sam')
            mobius.core.logf('DBG pymobius.registry.__get_system_hivefiles (4) %s' % sam)
            if sam:
                entries.append(sam)

            security = get_child_by_name(config_dir, 'security')
            mobius.core.logf('DBG pymobius.registry.__get_system_hivefiles (5) %s' % security)
            if security:
                entries.append(security)

            software = get_child_by_name(config_dir, 'software')
            mobius.core.logf('DBG pymobius.registry.__get_system_hivefiles (6) %s' % software)
            if software:
                entries.append(software)

            system = get_child_by_name(config_dir, 'system')
            mobius.core.logf('DBG pymobius.registry.__get_system_hivefiles (7) %s' % system)
            if system:
                entries.append(system)

            default = get_child_by_name(config_dir, 'default')
            mobius.core.logf('DBG pymobius.registry.__get_system_hivefiles (8) %s' % default)
            if default:
                entries.append(default)

            mobius.core.logf('DBG pymobius.registry.__get_system_hivefiles (9)')

        # create hivefiles
        hivefiles = []

        for entry in entries:
            parent = entry.get_parent()

            hivefile = pymobius.Data()
            hivefile.path = entry.path
            hivefile.role = entry.name.upper()
            hivefile.filename = entry.name.upper()
            hivefile.entry = entry
            hivefiles.append(hivefile)

            mobius.core.logf('DBG pymobius.registry.__get_system_hivefiles (10) %s %s %s' % (
            hivefile.filename, hivefile.role, hivefile.path))

        mobius.core.logf('DBG pymobius.registry.__get_system_hivefiles (11)')
        return hivefiles

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get user hivefiles
    # @param filesystem's root entry
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_user_hivefiles(self, root):
        mobius.core.logf('DBG pymobius.registry.__get_user_hivefiles (1) %s' % root.path)
        entries = []

        # %profilesdir%/*/ntuser.dat
        users_dir = get_child_by_name(root, 'users')

        if not users_dir:
            users_dir = get_child_by_name(root, 'documents and settings')

        mobius.core.logf('DBG pymobius.registry.__get_user_hivefiles (2) %s' % users_dir)

        if users_dir:
            mobius.core.logf('DBG pymobius.registry.__get_user_hivefiles (2.1) %s' % users_dir.name)

            for child in get_children(users_dir):
                mobius.core.logf('DBG pymobius.registry.__get_user_hivefiles (2.1.1) %s' % child.name)
                if is_folder(child):
                    mobius.core.logf('DBG pymobius.registry.__get_user_hivefiles (2.2) %s' % child.name)
                    ntuser = get_child_by_name(child, 'ntuser.dat')
                    if ntuser:
                        entries.append((child.name, ntuser))

        # %windir%/serviceprofiles/{localservice,networkservice}/ntuser.dat
        config_dir = get_child_by_path(root, 'windows/serviceprofiles')
        mobius.core.logf('DBG pymobius.registry.__get_user_hivefiles (3) %s' % config_dir)

        if config_dir:
            localservice = get_child_by_path(config_dir, 'localservice/ntuser.dat')
            mobius.core.logf('DBG pymobius.registry.__get_user_hivefiles (4) %s' % localservice)
            if localservice:
                entries.append((config_dir.name, localservice))

            networkservice = get_child_by_path(config_dir, 'networkservice/ntuser.dat')
            mobius.core.logf('DBG pymobius.registry.__get_user_hivefiles (5) %s' % networkservice)
            if networkservice:
                entries.append((config_dir.name, networkservice))

        # system profile
        config_dir = get_child_by_path(root, 'windows/system32/config/systemprofile')
        mobius.core.logf('DBG pymobius.registry.__get_user_hivefiles (6) %s' % config_dir)
        if config_dir:
            f = get_child_by_name(config_dir, "ntuser.dat")
            if f:
                entries.append((config_dir.name, f))

        # create hivefiles
        mobius.core.logf('DBG pymobius.registry.__get_user_hivefiles (7)')
        hivefiles = []

        for username, entry in entries:
            hivefile = pymobius.Data()
            hivefile.path = entry.path
            hivefile.role = 'NTUSER'
            hivefile.filename = 'NTUSER-%s.dat' % username.upper()
            hivefile.entry = entry
            hivefiles.append(hivefile)

        mobius.core.logf('DBG pymobius.registry.__get_user_hivefiles (8)')
        return hivefiles

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief retrieve file
    # @param f file
    # @param path fullpath
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_file(self, f, idx, filename):

        # create reader
        reader = f.new_reader()

        if not reader:
            return None

        # create output file
        rpath = os.path.join('registry', '%04d' % idx, filename)
        path = self.__item.create_data_path(rpath)

        fp = open(path, 'wb')
        h = mobius.core.crypt.hash('sha2-512')
        BLOCK_SIZE = 65536

        # copy data
        data = reader.read(BLOCK_SIZE)
        while data:
            fp.write(data)
            h.update(data)
            data = reader.read(BLOCK_SIZE)

        fp.close()

        # return hash
        return h.get_hex_digest()
