# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023 Eduardo Aguiar
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

import mobius
import pymobius
from gi.repository import Gdk
from gi.repository import Gtk


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Report dialog
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ReportDialog(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Build widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__widget = Gtk.Dialog(title='Choose report', modal=True)

        self.__widget.set_position(Gtk.WindowPosition.CENTER)
        self.__widget.set_default_size(430, 200)
        self.__widget.set_type_hint(Gdk.WindowTypeHint.DIALOG)
        self.__widget.set_border_width(10)
        self.__widget.vbox.set_border_width(5)

        self.__execute_button = self.__widget.add_button(Gtk.STOCK_EXECUTE, Gtk.ResponseType.OK)
        self.__execute_button.set_sensitive(False)

        button = self.__widget.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)

        self.__mediator = pymobius.mediator.copy()

        label = mobius.ui.label()
        label.set_markup('<b>report</b>')
        label.set_halign(mobius.ui.label.align_left)
        label.set_visible(True)
        self.__widget.vbox.pack_start(label.get_ui_widget(), False, True, 0)

        # report ID entry
        self.__report_id_entry = Gtk.ComboBoxText.new_with_entry()
        self.__report_id_entry.connect('changed', self.on_report_id_selected)
        self.__report_id_entry.get_child().connect('changed', self.on_report_id_edited)

        last_report_id = mobius.framework.get_config('report-model.last-report-id')

        report_id_list = mobius.framework.get_config('report-model.report-id-list') or []
        # @begin-deprecated since=2.0
        if isinstance(report_id_list, str):
            report_id_list = report_id_list.split('|')
        # @end-deprecated
        report_ids = set(report_id_list)
        last_row = -1

        for row, report_id in enumerate(sorted(report_ids)):
            self.__report_id_entry.append_text(report_id)

            if report_id == last_report_id:
                last_row = row

        self.__report_id_entry.set_active(last_row)
        self.__report_id_entry.show()
        self.__widget.vbox.pack_start(self.__report_id_entry, False, True, 0)

        # placeholder
        label = mobius.ui.label(' ')
        label.set_visible(True)
        self.__widget.vbox.pack_start(label.get_ui_widget(), False, True, 0)

        # output folder
        label = mobius.ui.label()
        label.set_markup('<b>output folder</b>')
        label.set_halign(mobius.ui.label.align_left)
        label.set_visible(True)
        self.__widget.vbox.pack_start(label.get_ui_widget(), False, True, 0)

        self.__report_folder_button = Gtk.FileChooserButton(title='Select output folder')
        self.__report_folder_button.set_action(Gtk.FileChooserAction.SELECT_FOLDER)

        report_folder = mobius.framework.get_config('report-model.folder') or os.getcwd()

        if report_folder:
            self.__report_folder_button.set_current_folder(report_folder)

        self.__report_folder_button.show()
        self.__widget.vbox.pack_start(self.__report_folder_button, False, True, 0)

        # placeholder
        label = mobius.ui.label(' ')
        label.set_visible(True)
        self.__widget.vbox.pack_start(label.get_ui_widget(), False, True, 0)

        # error textview
        frame = Gtk.Frame()
        frame.show()
        self.__widget.vbox.pack_start(frame, True, True, 0)

        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        frame.add(sw)

        self.__error_textview = Gtk.TextView()
        self.__error_textview.set_wrap_mode(Gtk.WrapMode.WORD)
        self.__error_textview.hide()
        sw.add(self.__error_textview)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run dialog
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self, *args, **kwds):
        rc = self.__widget.run()
        report_generated = False

        # execute report
        while rc == Gtk.ResponseType.OK and not report_generated:
            report_generated = self.run_report(*args, **kwds)

            if not report_generated:
                rc = Gtk.Dialog.run(self)

        return rc

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Destroy dialog
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def destroy(self):
        self.__widget.destroy()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief run report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run_report(self, *args, **kwds):
        report_id = self.__report_id_entry.get_active_text()

        # save current state
        transaction = mobius.framework.new_config_transaction()
        mobius.framework.set_config('report-model.last-report-id', report_id)

        model = self.__report_id_entry.get_model()
        report_ids = set((row[0] for row in model))
        report_ids.add(report_id)
        mobius.framework.set_config('report-model.report-id-list', report_ids)

        report_folder = self.__report_folder_button.get_current_folder()
        mobius.framework.set_config('report-model.folder', report_folder)
        transaction.commit()

        # call report.run
        try:
            current_dir = os.getcwd()
        except OSError as e:
            current_dir = None

        os.chdir(report_folder)

        try:
            self.__mediator.call('report.run', report_id, *args, **kwds)
            self.__error_textview.hide()
            rc = True
        except Exception as e:
            textbuffer = self.__error_textview.get_buffer()
            textbuffer.set_text('ERROR: ' + str(e))
            self.__error_textview.show()
            rc = False

        if current_dir:
            os.chdir(current_dir)

        return rc

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle report ID selection
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_report_id_selected(self, combobox, *args):
        selected = combobox.get_active() != -1
        self.__execute_button.set_sensitive(selected)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle report ID editing
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_report_id_edited(self, entry, *args):
        valid = entry.get_text().strip() != ''
        self.__execute_button.set_sensitive(valid)
