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
import os
import os.path
import shutil
import tempfile
import threading

import mobius
import mobius.core.io
import pymobius
from gi.repository import Gtk

from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ATTR_DNAME = 'iped.dname'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get items with datasource
# @param itemlist Item list
# @return List of items with datasource
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_datasource_items(itemlist):
    return [i for i in itemlist if i.has_datasource()]


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get item paths, according to datasource
# @param item Item object
# @return Path list
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_paths(item):
    paths = []
    datasource = item.get_datasource()

    if not datasource:
        pass

    elif datasource.get_type() == 'vfs':
        vfs = datasource.get_vfs()

        for disk in vfs.get_disks():
            paths.append(disk.get_path())

    elif datasource.get_type() == 'ufdr':
        uri = mobius.core.io.uri(datasource.get_url())
        paths.append(uri.get_path('utf-8'))

    else:
        raise Exception(f"Invalid datasource type: {datasource.get_type()}")

    return paths


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get iped-search-app.jar path for item
# @param item Case Item
# @return Path
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_iped_search_app_path(item):
    indexer_path = get_iped_indexer_path(item)

    if indexer_path:
        path = os.path.join(indexer_path, 'lib', 'iped-search-app.jar')

        if os.path.exists(path):
            return path

    return None


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get workdir path for item
# @param item Case Item
# @return Path
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_workdir_path(item):
    case = item.case
    path = case.get_path(os.path.join('work', '%04d' % item.uid))
    return path


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get indexer folder
# @param item Case Item
# @return Path
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_iped_indexer_path(item):
    case = item.case
    path = case.get_path(os.path.join('work', '%04d' % item.uid, 'iped'))

    if os.path.exists(path):
        return path

    return None


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Check if item is already processed
# @param item Case Item
# @return True/False
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def is_item_processed(item):
    return get_iped_search_app_path(item) is not None


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: Processing
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ProcessingView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__mediator = pymobius.mediator.copy()
        self.__control = control
        self.__iped_path = None
        self.__itemlist = []
        self.name = 'Process item(s)'

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'processing.png')
        self.icon_data = open(path, 'rb').read()

        # widget
        self.__widget = mobius.core.ui.container()
        self.__widget.show()

        vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbox.set_border_width(5)
        vbox.set_spacing(10)
        vbox.set_visible(True)
        self.__widget.set_content(vbox)

        grid = Gtk.Grid.new()
        grid.set_row_spacing(10)
        grid.set_column_spacing(5)
        grid.show()
        vbox.add_child(grid, mobius.core.ui.box.fill_with_widget)

        label = mobius.core.ui.label()
        label.set_markup('<b>Dname:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 0, 1, 1)

        self.__dname_entry = Gtk.Entry()
        self.__dname_entry.set_sensitive(False)
        self.__dname_entry.set_hexpand(True)
        self.__dname_entry.show()
        self.__dname_entry.connect('changed', self.__on_dname_changed)
        grid.attach(self.__dname_entry, 1, 0, 2, 1)

        label = mobius.core.ui.label()
        label.set_markup('<b>Profile:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 1, 1, 1)

        self.__profile_model = Gtk.ListStore.new([str])
        renderer = Gtk.CellRendererText()

        self.__profile_combobox = Gtk.ComboBox.new_with_model(self.__profile_model)
        self.__profile_combobox.set_hexpand(True)
        self.__profile_combobox.pack_start(renderer, True)
        self.__profile_combobox.add_attribute(renderer, 'text', 0)
        self.__profile_combobox.connect('changed', self.__on_profile_changed)
        self.__profile_combobox.show()
        grid.attach(self.__profile_combobox, 1, 1, 2, 1)

        label = mobius.core.ui.label()
        label.set_markup('<b>Sector size:</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 2, 1, 2)

        sector_size = mobius.framework.get_config('iped.sector_size') or 512

        self.__sector_size_spinbutton = Gtk.SpinButton.new_with_range(512, 4096, 512)
        self.__sector_size_spinbutton.set_value(sector_size)
        self.__sector_size_spinbutton.show()

        grid.attach(self.__sector_size_spinbutton, 1, 2, 2, 2)

        # XMM (max memory size)
        label = mobius.core.ui.label()
        label.set_markup('<b>Max. Mem. (XMX, in GB):</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 4, 1, 2)

        xmx = mobius.framework.get_config('iped.xmx') or 8

        self.__xmx_spinbutton = Gtk.SpinButton.new_with_range(1, 4096, 1)
        self.__xmx_spinbutton.set_value(xmx)
        self.__xmx_spinbutton.connect('value-changed', self.__on_xmx_value_changed)

        self.__xmx_spinbutton.show()
        grid.attach(self.__xmx_spinbutton, 1, 4, 2, 2)
        
        self.__gc_checkbutton = Gtk.CheckButton.new_with_label('Force full garbage collection during processing')
        self.__gc_checkbutton.set_active(False)
        self.__gc_checkbutton.show()
        grid.attach(self.__gc_checkbutton, 1, 6, 2, 1)
        
        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(5)
        hbox.set_visible(True)
        hbox.add_filler ()
        vbox.add_child(hbox, mobius.core.ui.box.fill_none)

        self.__execute_button = mobius.core.ui.button()
        self.__execute_button.set_icon_by_name('system-run')
        self.__execute_button.set_text('_Execute')
        self.__execute_button.set_visible(True)
        self.__execute_button.set_callback('clicked', self.__on_execute_button_clicked)
        hbox.add_child(self.__execute_button, mobius.core.ui.box.fill_none)

        self.__resume_button = mobius.core.ui.button()
        self.__resume_button.set_icon_by_name('media-playback-start')
        self.__resume_button.set_text('_Resume')
        self.__resume_button.set_visible(True)
        self.__resume_button.set_callback('clicked', self.__on_resume_button_clicked)
        hbox.add_child(self.__resume_button, mobius.core.ui.box.fill_none)

        self.__open_button = mobius.core.ui.button()
        self.__open_button.set_icon_by_name('document-open')
        self.__open_button.set_text('_Open')
        self.__open_button.set_visible(True)
        self.__open_button.set_callback('clicked', self.__on_open_button_clicked)
        hbox.add_child(self.__open_button, mobius.core.ui.box.fill_none)

        self.__populate_profile_combobox()
        self.__update_view()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
        self.__itemlist = itemlist
        self.__update_view()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update panel
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __update_view(self):
        itemlist = get_datasource_items(self.__itemlist)

        if not itemlist:
            self.__widget.set_message('Select item(s) with datasource set to process')
            return

        # enable/disable open button
        flag_is_processed = any(is_item_processed(item) for item in self.__itemlist)
        self.__open_button.set_sensitive(flag_is_processed)
        self.__resume_button.set_sensitive(flag_is_processed)

        # set dname entry
        if len(self.__itemlist) == 1:
            item = self.__itemlist[0]

            if not item.has_attribute(ATTR_DNAME):
                transaction = item.case.new_transaction()
                item.set_attribute(ATTR_DNAME, item.name)
                transaction.commit()

            self.__dname_entry.set_text(item.get_attribute(ATTR_DNAME))
            self.__dname_entry.set_sensitive(True)

        else:
            self.__dname_entry.set_text('Multiple items selected')
            self.__dname_entry.set_sensitive(False)

        # show widget
        self.__widget.show_content()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set IPED path
    # @param path Full path
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_iped_path(self, path):
        self.__iped_path = path

        if path:
            self.__populate_profile_combobox()
            self.__update_view()
        else:
            self.__widget.set_message('You must set IPED path before processing items')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save current state
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        transaction = mobius.framework.new_config_transaction()
        mobius.framework.set_config('iped.sector_size', self.__sector_size_spinbutton.get_value_as_int())
        transaction.commit()

        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_profile_changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_profile_changed(self, combobox, *args):
        model = combobox.get_model()
        treeiter = combobox.get_active_iter()

        if treeiter:
            self.__profile = model.get_value(treeiter, 0)

            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('iped.last_profile', self.__profile)
            transaction.commit()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_execute_button_clicked event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_execute_button_clicked(self):

        # if there are items processed, show dialog
        is_processed = any(is_item_processed(item) for item in self.__itemlist)

        if is_processed:
            dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
            dialog.text = "Are you sure you want to reprocess item(s)?"
            dialog.add_button(mobius.core.ui.message_dialog.button_yes)
            dialog.add_button(mobius.core.ui.message_dialog.button_no)
            dialog.set_default_response(mobius.core.ui.message_dialog.button_no)
            rc = dialog.run()

            if rc != mobius.core.ui.message_dialog.button_yes:
                return

        # create thread
        option = pymobius.Data()
        option.itemlist = get_datasource_items(self.__itemlist)
        option.sector_size = self.__sector_size_spinbutton.get_value_as_int()
        option.xmx = self.__xmx_spinbutton.get_value_as_int()
        option.force_gc = self.__gc_checkbutton.get_active()
        option.continue_processing = False

        t = threading.Thread(target=self.__processing_thread, args=(option,), daemon=True)
        t.start()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_resume_button_clicked event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_resume_button_clicked(self):

        # create thread
        option = pymobius.Data()
        option.itemlist = get_datasource_items(self.__itemlist)
        option.sector_size = self.__sector_size_spinbutton.get_value_as_int()
        option.xmx = self.__xmx_spinbutton.get_value_as_int()
        option.force_gc = self.__gc_checkbutton.get_active()
        option.continue_processing = True

        t = threading.Thread(target=self.__processing_thread, args=(option,), daemon=True)
        t.start()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle 'value-changed' spinbutton event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_xmx_value_changed(self, spinbutton, *args):
        transaction = mobius.framework.new_config_transaction()
        mobius.framework.set_config('iped.xmx', spinbutton.get_value_as_int())
        transaction.commit()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Processing thread
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __processing_thread(self, option):
        guard = mobius.core.thread_guard()
        case = option.itemlist[0].case
        connection = case.new_connection()

        # process items
        for item in option.itemlist:
            datasource = item.get_datasource()
            name = (item.get_attribute(ATTR_DNAME) or f'item-{item.uid:04d}').strip()
            logfile = case.get_path(os.path.join('history', f'{item.uid:04d}.log'))
            workdir = get_workdir_path(item)

            # remove old processing files, if necessary
            if not option.continue_processing:
                if os.path.exists(logfile):
                    os.remove(logfile)

                if os.path.exists(workdir):
                    self.__control.set_status(f'Removing {item.uid:04d} work directory...')
                    shutil.rmtree(workdir)

            # build command
            cmd = 'java'
            
            if option.force_gc:
                new_size = (option.xmx // 4) or 1
                cmd += f' -XX:MinHeapFreeRatio=10 -XX:MaxHeapFreeRatio=20 -XX:+UseG1GC -XX:NewSize={new_size}g -XX:MaxNewSize={new_size}g'
                
            cmd += f' -jar {self.__iped_path}/iped.jar \
                -b {option.sector_size} \
                -log "{logfile}" \
                -Xmx{option.xmx}g \
                -Xms{option.xmx}g \
                -o "{workdir}"'

            for idx, path in enumerate(get_paths(item)):
                if idx == 0:
                    dname = name
                else:
                    dname = f'{name}[{idx:d}]'
                cmd += f' -d "{path}" -dname "{dname}"'

            if self.__profile and self.__profile != 'default':
                cmd += f' -profile "{self.__profile}"'

            if option.continue_processing:
                cmd += ' --continue'
                
            mobius.core.logf('INF ' + cmd)

            # run
            self.__control.set_status(f'Processing item {item.uid:04d} - {item.name}...')
            os.system(cmd)
            self.__control.set_status(f'Item {item.uid:04d} processed.')

        # update panel
        self.__update_view()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_open_button_clicked event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_open_button_clicked(self):
        processed_items = [item for item in self.__itemlist if is_item_processed(item)]

        if processed_items:
            t = threading.Thread(target=self.__opening_thread, args=(processed_items,), daemon=True)
            t.start()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Opening thread
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __opening_thread(self, itemlist):
        item = itemlist[0]
        case = item.case
        connection = case.new_connection()

        # get iped-search-app.jar path
        search_app_path = get_iped_search_app_path(item)

        # build command for one item
        if len(itemlist) == 1:
            cmd = 'java -jar "%s"' % search_app_path
            tmpfile = None

        # build command for multiple items
        else:
            # create temporary file with workdirs
            fd, tmpfile = tempfile.mkstemp('.txt')
            os.close(fd)

            f = mobius.core.io.new_file_by_path(tmpfile)
            fp = mobius.core.io.text_writer(f.new_writer())

            for item in itemlist:
                fp.write(get_workdir_path(item) + '\n')

            fp.flush()

            cmd = f'java -jar "{search_app_path}" -multicases {tmpfile}'

        # run command
        mobius.core.logf('INF ' + cmd)
        os.system(cmd)

        # remove tmpfile, if necessary
        if tmpfile:
            os.remove(tmpfile)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Populate profile combobox
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __populate_profile_combobox(self):
        if not self.__iped_path:
            return

        # List profiles
        profiles_path = os.path.join(self.__iped_path, 'profiles')
        profiles = set(os.listdir(profiles_path) + ['default'])

        # Fill combobox
        self.__profile_model.clear()

        last_profile = mobius.framework.get_config('iped.last_profile')
        select_idx = -1

        for idx, profile in enumerate(sorted(profiles)):
            self.__profile_model.append((profile,))

            if profile == last_profile:
                select_idx = idx

        self.__profile_combobox.set_sensitive(True)
        if select_idx != -1:
            self.__profile_combobox.set_active(select_idx)

        self.__profile = last_profile

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_dname_changed event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_dname_changed(self, entry, *args):
        if len(self.__itemlist) == 1:
            item = self.__itemlist[0]
            transaction = item.case.new_transaction()
            item.set_attribute(ATTR_DNAME, entry.get_text().rstrip())
            transaction.commit()
