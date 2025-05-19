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
import mobius.core.os
import pymobius

ANT_ID = 'trash-can-entries'
ANT_NAME = 'Trash Can entries'
ANT_VERSION = '1.1'
EVIDENCE_TYPE = 'trash-can-entry'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Ant: trash-can
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

        # check if datasource is available
        datasource = self.__item.get_datasource()

        if not datasource:
            return

        if not datasource.is_available():
            raise Exception('Datasource is not available')

        # retrieve data
        self.__entries = []
        self.__retrieve_win(datasource)
        self.__save_data()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from Vista/Win7/Win8/Win10 OS
    # @see https://www.forensicfocus.com/articles/forensic-analysis-of-the-microsoft-windows-vista-recycle-bin/
    # @see https://www.csee.umbc.edu/courses/undergraduate/FYS102D/Recycle.Bin.Forensics.for.Windows7.and.Windows.Vista.pdf
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_win(self, datasource):
        try:
            vfs = datasource.get_vfs()

            for entry in vfs.get_root_entries():
                if entry.is_folder():
                    self.__retrieve_win_root_folder(entry)

        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from win root folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_win_root_folder(self, root_folder):
        try:
            recycle_bin_folder = root_folder.get_child_by_name('$RECYCLE.BIN', False)

            if recycle_bin_folder:
                self.__retrieve_vista_recycle_bin_folder(recycle_bin_folder)

            recycler_folder = root_folder.get_child_by_name('RECYCLER', False)

            if recycler_folder:
                self.__retrieve_win2k_recycler_folder(recycler_folder)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from $RECYCLE_BIN folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_vista_recycle_bin_folder(self, folder):
        try:
            for child in folder.get_children():
                if child.is_folder() and child.name.startswith('S-'):
                    self.__retrieve_vista_sid_folder(child)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from <SID> subfolder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_vista_sid_folder(self, folder):
        mobius.core.logf(f'DBG retrieving SID folder: {folder.name}')

        try:
            i_files = {}
            r_files = {}

            # get $I and $R files from folder
            for child in folder.get_children():
                if child.name.startswith('$I') and child.is_file() and child.size > 0:
                    i_files[child.name] = child

                elif child.name.startswith('$R'):
                    r_files[child.name] = child

            # process $I files
            for f in i_files.values():
                rname = '$R' + f.name[2:]
                r = r_files.get(rname)
                self.__retrieve_vista_process_info_file(f, r, folder)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from $I info file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_vista_process_info_file(self, f, r, sid_folder):
        try:
            # decode record
            ifile = mobius.core.os.win.trashbin.ifile(f.new_reader())

            if ifile.path:
                version = ifile.version
                rname = '$R' + f.name[2:]

                e = pymobius.Data()
                e.name = f.name
                e.size = ifile.size
                e.deletion_time = ifile.deletion_time
                e.path = ifile.path

                # fill metadata
                e.metadata = mobius.pod.map()
                e.metadata.set('version', version)
                e.metadata.set('sid', sid_folder.name)
                e.metadata.set('info-file-path', f.path)
                e.metadata.set('restore-file-name', rname)

                # retrieve info from $R file
                if r and r.exists():
                    e.metadata.set('restore-file-path', r.path)
                    e.metadata.set('restore-file-size', r.size)
                    e.metadata.set('restore-is-available', True)

                else:
                    e.metadata.set('restore-is-available', False)

                self.__entries.append(e)
                pymobius.set_handled(self.__item, f)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from RECYCLER folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_win2k_recycler_folder(self, folder):
        try:
            for child in folder.get_children():
                if child.is_folder() and child.name.startswith('S-'):
                    self.__retrieve_win2k_sid_folder(child)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from <SID> subfolder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_win2k_sid_folder(self, folder):
        try:
            f = folder.get_child_by_name('INFO2', False)
            if f:
                info2 = mobius.core.os.win.trashbin.info2_file(f.new_reader())

                for entry in info2.entries:
                    self.__retrieve_win2k_info2_entry(entry, folder, f, info2)

                pymobius.set_handled(self.__item, f)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve data from INFO2 entry
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_win2k_info2_entry(self, entry, sid_folder, f_info2, info2):
        try:
            # Get Dc*.* file name
            p = mobius.io.new_path_from_win(entry.path)
            ext = p.get_extension()

            dc_name = 'Dc' + str(entry.file_idx)
            if ext:
                dc_name = dc_name + '.' + ext

            # retrieve data from record
            e = pymobius.Data()
            e.name = p.get_filename()
            e.size = entry.size
            e.deletion_time = entry.deletion_time
            e.path = entry.path

            # fill metadata
            e.metadata = mobius.pod.map()
            e.metadata.set('logical-size', entry.size)
            e.metadata.set('file-idx', entry.file_idx)
            e.metadata.set('sid', sid_folder.name)
            e.metadata.set('info2-file-path', f_info2.path)
            e.metadata.set('info2-version', info2.version)
            e.metadata.set('info2-record-size', info2.record_size)
            e.metadata.set('restore-file-name', dc_name)

            # retrieve info from Dc*.* file
            r = sid_folder.get_child_by_name(dc_name)

            if r and r.exists():
                e.size = r.size
                e.metadata.set('restore-file-path', r.path)
                e.metadata.set('restore-file-size', r.size)
                e.metadata.set('restore-is-available', True)

            else:
                e.metadata.set('restore-is-available', False)

            self.__entries.append(e)
        except Exception as e:
            mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save data into model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __save_data(self):
        transaction = self.__item.new_transaction()

        # save evidences
        for e in self.__entries:
            entry = self.__item.new_evidence(EVIDENCE_TYPE)
            entry.path = e.path
            entry.size = e.size
            entry.name = e.name
            entry.deletion_time = e.deletion_time
            entry.metadata = e.metadata

        # set ant run
        transaction.commit()
