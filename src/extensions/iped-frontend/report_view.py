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
import hashlib
import os
import shutil
import tempfile
import threading
import traceback
import zipfile

import mobius
import mobius.core.io
import pymobius
import pymobius.item_browser
from gi.repository import Gtk

from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: Report
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ReportView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__mediator = pymobius.mediator.copy()
        self.__control = control
        self.__iped_path = None
        self.__itemlist = None
        self.__processed_items = []
        self.__laudo = None
        self.__uids = []
        self.name = 'Generate Report'

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'report.png')
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
        label.set_markup('<b>Report ID</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 0, 1, 1)

        self.__report_id_entry = Gtk.Entry()
        self.__report_id_entry.set_sensitive(False)
        self.__report_id_entry.set_hexpand(True)
        self.__report_id_entry.show()
        self.__report_id_entry.connect('changed', self.__on_report_id_changed)
        grid.attach(self.__report_id_entry, 1, 0, 2, 1)

        self.__hashes_txt_checkbutton = Gtk.CheckButton.new_with_mnemonic("Generate _hashes.txt")
        self.__hashes_txt_checkbutton.show()
        self.__hashes_txt_checkbutton.set_sensitive(False)
        self.__hashes_txt_checkbutton.connect('toggled', self.__on_hashes_txt_checkbutton_toggled)
        grid.attach(self.__hashes_txt_checkbutton, 1, 1, 2, 1)

        self.__generate_iso_checkbutton = Gtk.CheckButton.new_with_mnemonic("Generate _.iso file")
        self.__generate_iso_checkbutton.show()
        self.__generate_iso_checkbutton.set_sensitive(False)
        self.__hashes_txt_checkbutton.connect('toggled', self.__on_generate_iso_checkbutton_toggled)
        grid.attach(self.__generate_iso_checkbutton, 1, 2, 2, 1)

        label = mobius.core.ui.label()
        label.set_markup('<b>Hashes.txt SHA2-256</b>')
        label.set_halign(mobius.core.ui.label.align_right)
        label.set_visible(True)
        grid.attach(label.get_ui_widget(), 0, 3, 1, 1)

        self.__hashes_txt_hash_label = mobius.core.ui.label()
        self.__hashes_txt_hash_label.set_halign(mobius.core.ui.label.align_left)
        self.__hashes_txt_hash_label.set_selectable(True)
        self.__hashes_txt_hash_label.show()
        grid.attach(self.__hashes_txt_hash_label.get_ui_widget(), 1, 3, 2, 1)

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_visible(True)
        hbox.add_filler ()
        vbox.add_child(hbox, mobius.core.ui.box.fill_none)

        self.__generate_report_button = mobius.core.ui.button()
        self.__generate_report_button.set_icon_by_name('system-run')
        self.__generate_report_button.set_text('_Execute')
        self.__generate_report_button.set_sensitive(False)
        self.__generate_report_button.set_visible(True)
        self.__generate_report_button.set_callback('clicked', self.__on_generate_report)
        hbox.add_child(self.__generate_report_button, mobius.core.ui.box.fill_none)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save current state
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
        self.__itemlist = itemlist
        self.__processed_items = []
        self.__laudo = None

        if self.__itemlist:
            if self.__is_laudo_container(itemlist):
                self.__set_laudo_container(itemlist[0])

            else:
                self.__set_selected_items(self.__itemlist)

        else:
            self.__widget.set_message('Select item(s) to generate report')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set IPED path
    # @param path Full path
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_iped_path(self, path):
        self.__iped_path = path

        if path:
            self.__widget.show_content()
        else:
            self.__widget.set_message('You must set IPED path before generating reports')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set laudo container (Brazilian Federal Police)
    # @param item Container item
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __set_laudo_container(self, item):
        case = item.case
        self.__laudo = None
        self.__processed_items = []

        for c in item.get_children():
            if c.category == 'laudo':
                self.__laudo = c

            elif c.category == 'report-data' and c.rid == 'item.attributes':
                data = c.data
                uid = data.uid
                self.__add_processed_item(case.get_item_by_uid(uid))

        if self.__laudo and self.__processed_items:
            numero, ano = [int(x) for x in self.__laudo.numero.split('/')]
            report_id = f'{ano:04d}-{numero:04d}'

            self.__report_id_entry.set_text(report_id)
            self.__report_id_entry.set_sensitive(False)
            self.__hashes_txt_checkbutton.set_active(True)
            self.__hashes_txt_checkbutton.set_sensitive(False)
            self.__generate_iso_checkbutton.set_active(True)
            self.__generate_iso_checkbutton.set_sensitive(False)

            hashes_txt_path = case.get_path(os.path.join('report', report_id, 'hashes.txt'))
            hashes_txt_value = self.__laudo.hashes_txt

            if hashes_txt_value:
                self.__hashes_txt_hash_label.set_text(hashes_txt_value)

            elif os.path.exists(hashes_txt_path):
                self.__calculate_hashes_txt_hash(hashes_txt_path, self.__laudo)

            else:
                self.__hashes_txt_hash_label.set_text('')

            self.__control.set_status('')
            self.__widget.show_content()

        else:
            self.__widget.set_message('No processed item(s) selected')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set selected items
    # @param itemlist Case item list
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __set_selected_items(self, itemlist):

        # Search for processed items
        self.__processed_items = []

        for item in itemlist:
            self.__add_processed_item(item)

        # Enable Report ID entry
        if self.__processed_items:
            self.__report_id_entry.set_text('')
            self.__report_id_entry.set_sensitive(True)
            self.__hashes_txt_checkbutton.set_sensitive(True)
            self.__hashes_txt_checkbutton.set_active(mobius.framework.get_config('iped.generate_hashes_txt') == 'True')
            self.__generate_iso_checkbutton.set_sensitive(True)
            self.__generate_iso_checkbutton.set_active(mobius.framework.get_config('iped.generate_iso') == 'True')
            self.__widget.show_content()
        else:
            self.__widget.set_message('No processed item(s) selected')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Add processed item UID to list
    # @param item Case item
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __add_processed_item(self, item):
        case = item.case
        search_path = case.get_path(f"work/{item.uid:04d}/iped/lib/iped-search-app.jar")

        if os.path.exists(search_path):
            self.__processed_items.append(item)

        for child in item.get_children():
            self.__add_processed_item(child)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Check if selected item is a laudo (Brazilian Federal Police) container
    # @param itemlist Case item list
    # @return True/False
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __is_laudo_container(self, itemlist):
        return len(itemlist) == 1 and any(c for c in itemlist[0].get_children() if c.category == 'laudo')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_report_id_changed event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_report_id_changed(self, entry, *args):
        can_generate_report = entry.get_text() != ''
        self.__generate_report_button.set_sensitive(can_generate_report)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_hashes_txt_checkbutton_toggled
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_hashes_txt_checkbutton_toggled(self, checkbutton, *args):
        transaction = mobius.framework.new_config_transaction()
        mobius.framework.set_config('iped.generate_hashes_txt', str(checkbutton.get_active()))
        transaction.commit()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_generate_iso_checkbutton_toggled
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_generate_iso_checkbutton_toggled(self, checkbutton, *args):
        transaction = mobius.framework.new_config_transaction()
        mobius.framework.set_config('iped.generate_iso', str(checkbutton.get_active()))
        transaction.commit()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief on_generate_report event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_generate_report(self):
        case = self.__itemlist[0].case
        report_id = self.__report_id_entry.get_text()
        report_path = case.get_path(os.path.join('report', report_id))

        # create main report folder, if necessary
        main_report_path = case.get_path('report')

        if not os.path.exists(main_report_path):
            os.makedirs(main_report_path)

        # check if there is an older report
        if os.path.exists(report_path):
            dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
            dialog.text = f"Do you want to overwrite '{report_id}' report?"
            dialog.add_button(mobius.core.ui.message_dialog.button_yes)
            dialog.add_button(mobius.core.ui.message_dialog.button_no)
            dialog.set_default_response(mobius.core.ui.message_dialog.button_no)
            rc = dialog.run()

            if rc != mobius.core.ui.message_dialog.button_yes:
                return

        option = pymobius.Data()
        option.flag_generate_hashes_txt = self.__hashes_txt_checkbutton.get_active()
        option.flag_generate_iso = self.__generate_iso_checkbutton.get_active()
        option.report_id = report_id
        option.report_path = report_path
        option.report_log_path = report_path + '.log'
        option.report_iso_path = report_path + '.iso'
        option.case = case
        option.laudo = self.__laudo
        option.itemlist = self.__itemlist[:]
        option.processed_items = self.__processed_items[:]
        option.control = self.__control
        option.iped_path = self.__iped_path
        option.xmx = mobius.framework.get_config('iped.xmx') or 8

        # create thread
        t = threading.Thread(target=self.__generate_report_thread, args=(option,), daemon=True)
        t.start()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Report generation thread
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_report_thread(self, option):
        guard = mobius.core.thread_guard()
        connection = option.case.new_connection()

        # remove previous report folder and .log
        if os.path.exists(option.report_log_path):
            os.remove(option.report_log_path)

        if os.path.exists(option.report_iso_path):
            os.remove(option.report_iso_path)

        if os.path.exists(option.report_path):
            option.control.set_status('Removing old report...')
            shutil.rmtree(option.report_path)

        os.makedirs(option.report_path)

        # build command line
        # create one xxxx.iped file for each 'bookmarks.iped' file, because each file name must have a unique name
        cmd = f'java -jar {option.iped_path}/iped.jar \
            -log "{option.report_log_path}"  \
            -Xmx{option.xmx}g \
            -Xms{option.xmx}g \
            -o "{option.report_path}"'

        for item in option.processed_items:
            case = item.case
            indexer_path = case.get_path(os.path.join('work', f'{item.uid:04d}', 'iped'))
            bookmarks_default_path = os.path.join(indexer_path, 'bookmarks.iped')

            if os.path.exists(bookmarks_default_path):
                bookmarks_path = os.path.join(indexer_path, f'{item.uid:04d}.iped')
                shutil.copyfile(bookmarks_default_path, bookmarks_path)
                cmd += f' -d "{bookmarks_path}"'

        # generate .asap file, if necessary
        if option.laudo:
            asap_path = self.__generate_asap(option)
            cmd += f' -asap "{asap_path}"'

        # run report
        mobius.core.logf('INF ' + cmd)
        option.control.set_status('Running IPED report...')
        os.system(cmd)

        # run pf.midia.principal report template (Brazilian Federal Police only, for now)
        if option.laudo:
            self.__generate_pf_reports(option)

        # generate hashes.txt, if necessary
        if option.flag_generate_hashes_txt:
            self.__generate_hashes_txt(option)

        # generate .iso, if necessary
        if option.flag_generate_iso:
            self.__generate_iso(option)

        # final message
        option.control.set_status(f'Report {option.report_id} generated.')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate .asap file (Brazilian Federal Police)
    # @param option Option object
    # @return .asap generated file path
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_asap(self, option):
        asap_path = option.report_path + '.asap'
        laudo = option.laudo

        f = mobius.core.io.new_file_by_path(asap_path)
        fp = mobius.core.io.text_writer(f.new_writer(), 'iso-8859-1')
        fp.write('[LAUDO]\n')
        fp.write('TITULO=%s\n' % laudo.titulo)
        fp.write('SUBTITULO=%s\n' % laudo.subtitulo)
        fp.write('UNIDADE=%s\n' % laudo.unidade)
        fp.write('NUMERO=%s\n' % laudo.numero)
        fp.write('DATA=%s\n' % laudo.data)
        fp.write('PCF1=%s|%s|E|M\n' % (laudo.perito1_nome, laudo.perito1_matricula))
        fp.write('MATERIAL=%s\n' % '|'.join(item.name for item in option.processed_items))
        fp.write('MATERIAL_DESCR=%s\n' % '|'.join(item.name for item in option.processed_items))
        fp.write('CODIGO_BARRA=%s\n' % laudo.codigo_barra)
        fp.write('\n')

        fp.write('[SOLICITACAO]\n')
        fp.write('NUMERO_IPL=%s\n' % laudo.ipl)
        fp.write('AUTORIDADE=%s\n' % laudo.requisitante)
        fp.write('DOCUMENTO=%s\n' % laudo.documento)
        fp.write('DATA_DOCUMENTO=%s\n' % laudo.data_documento)
        fp.write('NUMERO_SIAPRO=%s\n' % laudo.numero_sei)
        fp.write('NUMERO_CRIMINALISTICA=%s\n' % laudo.protocolo)
        fp.write('DATA_CRIMINALISTICA=%s\n' % laudo.data_protocolo)
        fp.flush()

        return asap_path

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate Brazilian Federal Police reports
    # @param option Option object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_pf_reports(self, option):
        option.control.set_status('Generating PF reports...')

        # get resource html.zip path
        html_zip_path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'html.zip')

        if not os.path.exists(html_zip_path):
            mobius.core.logf('ERR Could not find resource file <html.zip>')
            return

        # extract files from html.zip to report folder
        zfile = zipfile.ZipFile(html_zip_path)
        zfile.extractall(option.report_path)
        zfile.close()

        # change current dir
        try:
            current_dir = os.getcwd()  # test
        except OSError as e:
            current_dir = None

        os.chdir(option.report_path)

        # run report
        item = option.itemlist[0]
        wrapped_item = pymobius.item_browser.ItemBrowser(item)

        try:
            self.__mediator.call('report.run', 'pf.midia.principal', wrapped_item)
        except Exception as e:
            mobius.core.logf(f'ERR {str(e)}\n{traceback.format_exc()}')

        # get generated reports
        reports = []
        report_filename = 'reports.txt'

        f = mobius.core.io.new_file_by_path(report_filename)
        fp = mobius.core.io.line_reader(f.new_reader())

        for line in fp:
            ref, name = line.rstrip().split('|')
            reports.append((ref, name))

        option.laudo.reports = reports
        os.remove(report_filename)

        # restore old current dir
        if current_dir:
            os.chdir(current_dir)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate hashes.txt
    # @param option Option object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_hashes_txt(self, option):
        option.control.set_status('Generating hashes.txt file...')

        # remove old hashes.txt, if any
        path = os.path.join(option.report_path, 'hashes.txt')
        if os.path.exists(path):
            os.remove(path)

        # create temporary file
        fd, tmpfile = tempfile.mkstemp('.txt')
        os.close(fd)

        # generate hashes.txt
        f = mobius.core.io.new_file_by_path(tmpfile)
        fp = mobius.core.io.text_writer(f.new_writer())
        pos = len(option.report_path) + 1

        for root, dirs, files in os.walk(option.report_path, topdown=False):
            for name in files:
                path = os.path.join(root, name)
                hash_value = self.__get_hash(path)
                filename = path[pos:]
                fp.write(f'{hash_value} ?SHA256*{filename}\n')

        fp.flush()

        # move file to report_path
        hashes_txt_path = os.path.join(option.report_path, 'hashes.txt')

        if os.path.exists(hashes_txt_path):
            os.remove(hashes_txt_path)

        shutil.move(tmpfile, hashes_txt_path)

        # calculate hashes.txt hash
        self.__calculate_hashes_txt_hash(hashes_txt_path, option.laudo)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Calculate hashes.txt SHA2-256 hash
    # @param path Hashes.txt path
    # @param laudo Laudo object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __calculate_hashes_txt_hash(self, path, laudo):
        self.__control.set_status('Calculating hashes.txt file SHA2-256 hash value...')
        hash_value = self.__get_hash(path)
        self.__hashes_txt_hash_label.set_text(hash_value)
        laudo.hashes_txt = hash_value

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Calculate file hash
    # @param path File path
    # @return Hash as string
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_hash(self, path):
        h = hashlib.sha256()
        fp = open(path, 'rb')

        data = fp.read(65536)
        while data:
            h.update(data)
            data = fp.read(65536)

        fp.close()
        return h.hexdigest()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate .iso file
    # @param option Option object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_iso(self, option):
        option.control.set_status(f'Generating {os.path.basename(option.report_path)}.iso file...')

        iso_path = option.report_path + '.iso'
        cmd = f'mkisofs -udf -iso-level 3 -quiet -V {option.report_id} -o "{iso_path}" "{option.report_path}"'
        mobius.core.logf('INF ' + cmd)
        os.system(cmd)
