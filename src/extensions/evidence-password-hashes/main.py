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
import re

import mobius
import mobius.core.crypt
import mobius.core.os
import mobius.core.turing
import pymobius
from gi.repository import Gtk

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Extension metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
EXTENSION_ID = 'evidence-password-hashes'
EXTENSION_NAME = 'Evidence: Password Hashes'
EXTENSION_AUTHOR = 'Eduardo Aguiar'
EXTENSION_VERSION = '1.10'
EXTENSION_DESCRIPTION = 'List password hashes'

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
EVIDENCE_TYPE = 'password-hash'
REGEX_WORDS = re.compile("(\\w[\\w']*\\w|\\w)")
PASSWORD_NOT_FOUND, PASSWORD_PARTIAL, PASSWORD_FOUND = range(3)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Hash testing class
#
# This class contains all the hash objects being tested, grouped by
# hash type and hash unique parameters. NT and LM hashes form two groups.
# MSDCC1 hashes are grouped by type and username, and MSDCC2 hashes are
# grouped by type, username and iterations.
#
# The function "test" test each keyword.
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class HashTesting(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__hashes = {}
        self.__case = None
        self.__hash_passwords = []

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Check if list is empty
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __len__(self):
        return len(self.__hashes)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Add hash to testing list
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add(self, h):
        if not self.__case:
            self.__case = h.item.case

        if h.password_hash_type in ('lm', 'nt', 'sha1.utf16'):
            g = (h.password_hash_type,)

        elif h.password_hash_type == 'msdcc1':
            metadata = h.metadata
            username = metadata.get('username')
            g = (h.password_hash_type, username)

        elif h.password_hash_type == 'msdcc2':
            metadata = h.metadata
            username = metadata.get('username')
            iterations = metadata.get('iterations', 10240)
            g = (h.password_hash_type, username, iterations)

        else:
            g = None

        # add hash to the group
        if g:
            d = self.__hashes.setdefault(g, {})
            d.setdefault(h.value, []).append(h)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Test keyword
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def test(self, keyword):

        # generate hashes according to the groups defined
        for g, hashdict in self.__hashes.items():
            htype = g[0]
            hvalue = None

            if htype == 'nt':
                hvalue = mobius.encoder.hexstring(mobius.core.os.win.hash_nt(keyword))

            elif htype == 'lm':
                hvalue = mobius.encoder.hexstring(mobius.core.os.win.hash_lm(keyword))

            elif htype == 'sha1.utf16':
                hvalue = mobius.encoder.hexstring(mobius.core.crypt.hash_digest('sha1', keyword.encode('utf-16le')))

            elif htype == 'msdcc1':
                hvalue = mobius.encoder.hexstring(mobius.core.os.win.hash_msdcc1(keyword, g[1]))

            elif htype == 'msdcc2':
                hvalue = mobius.encoder.hexstring(mobius.core.os.win.hash_msdcc2(keyword, g[1], g[2]))

            # test hash value
            for h in hashdict.pop(hvalue, []):
                self.__hash_passwords.append((h, keyword))

        # remove emptied groups
        self.__hashes = dict((g, hashdict) for (g, hashdict) in self.__hashes.items() if hashdict)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Update databases
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def update(self):

        # update password_hashes passwords
        transaction = self.__case.new_transaction()

        for h, keyword in self.__hash_passwords:
            h.password = keyword

        transaction.commit()

        # update Turing database
        turing = mobius.core.turing.turing()
        transaction = turing.new_transaction()

        for h, keyword in self.__hash_passwords:
            turing.set_hash(h.password_hash_type, h.value, keyword)

        transaction.commit()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Evidence: Password Hashes
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class HashesView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__mediator = pymobius.mediator.copy()
        self.__control = control
        self.__data = None

        self.name = 'Password Hashes'
        icon_path = self.__mediator.call('extension.get-icon-path', EXTENSION_ID)
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.ui.container()
        self.__widget.set_visible(True)

        # vbox
        vbox = mobius.ui.box(mobius.ui.box.orientation_vertical)
        vbox.set_border_width(5)
        vbox.set_spacing(10)
        vbox.set_visible(True)
        self.__widget.set_content(vbox)

        # vpaned
        self.__vpaned = Gtk.VPaned()
        self.__vpaned.show()
        vbox.add_child(self.__vpaned, mobius.ui.box.fill_with_widget)

        pos = mobius.framework.get_config('evidence-password-hashes.vpaned-pos')
        if pos:
            self.__vpaned.set_position(pos)

        # tableview
        self.__tableview = self.__mediator.call('ui.new-widget', 'tableview')
        self.__tableview.set_report_id('crypt.pwd-hashes')
        self.__tableview.set_report_name('Turing: password hashes')
        self.__tableview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')

        column = self.__tableview.add_column('password_hash_type', 'Hash Type')
        column.is_sortable = True

        column = self.__tableview.add_column('value', 'Value')
        column.is_sortable = True

        self.__tableview.add_column('password', 'Password')

        column = self.__tableview.add_column('password_status', 'Password Status')
        column.is_sortable = True

        self.__tableview.add_column('description', 'Description')
        self.__tableview.set_control(self, 'main_tableview')

        column = self.__tableview.add_column('obj', column_type='object')
        column.is_visible = False
        column.is_exportable = False

        self.__tableview.add_export_handler('hashcat', 'Hashcat hash file', 'hashcat', self.__on_export_hashcat)
        self.__tableview.add_export_handler('jtr', 'John the Ripper .john file', 'john', self.__on_export_john)
        self.__tableview.add_export_handler('pot', 'John the Ripper .pot file', 'pot', self.__on_export_pot)
        self.__tableview.add_export_handler('wordlist', 'Word list', 'txt', self.__on_export_wordlist)
        self.__tableview.show()
        self.__vpaned.pack1(self.__tableview.get_ui_widget(), False, True)

        toolbar = self.__tableview.get_toolbar()

        toolitem = Gtk.SeparatorToolItem()
        toolitem.show()
        toolbar.insert(toolitem, -1)

        path = pymobius.mediator.call('extension.get-resource-path', EXTENSION_ID, 'test.png')
        image = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_dnd)
        image.show()

        self.__test_keywords_toolitem = Gtk.ToolButton.new()
        self.__test_keywords_toolitem.set_icon_widget(image.get_ui_widget())
        self.__test_keywords_toolitem.show()
        self.__test_keywords_toolitem.set_tooltip_text("Test keywords from file")
        self.__test_keywords_toolitem.connect("clicked", self.__on_test_keywords)
        toolbar.insert(self.__test_keywords_toolitem, -1)

        toolitem = Gtk.ToolItem()
        toolitem.set_border_width(10)
        toolitem.set_expand(True)
        toolitem.show()
        toolbar.insert(toolitem, -1)

        self.__test_keywords_progress_bar = Gtk.ProgressBar()
        self.__test_keywords_progress_bar.hide()
        toolitem.add(self.__test_keywords_progress_bar)

        # metadata view
        self.__metadata_view = self.__mediator.call('ui.new-widget', 'tableview')
        self.__metadata_view.set_report_id('password-hash.details')
        self.__metadata_view.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
        self.__metadata_view.show()

        column = self.__metadata_view.add_column('name', 'Attribute')
        column.is_sortable = True

        column = self.__metadata_view.add_column('value', 'Value')
        column.is_sortable = True

        self.__vpaned.pack2(self.__metadata_view.get_ui_widget(), True, True)

        # status bar
        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.IN)
        frame.show()
        vbox.add_child(frame, mobius.ui.box.fill_none)

        self.__status_label = mobius.ui.label()
        self.__status_label.set_halign(mobius.ui.label.align_left)
        self.__status_label.set_elide_mode(mobius.ui.label.elide_end)
        self.__status_label.set_visible(True)
        frame.add(self.__status_label.get_ui_widget())

        # set initial message
        self.__widget.set_message('Select item(s) to view password hashes')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):
        model = self.__tableview.new_model()

        STATUS = ['not found', 'found', 'first half found', 'second half found']
        flag_has_unknown_password = False
        count = 0
        password_count = 0

        turing = mobius.core.turing.turing()

        for item in itemlist:
            for h in item.get_evidences(EVIDENCE_TYPE):
                count += 1

                if h.has_attribute("password"):
                    status = 1  # found
                    password_count += 1
                    password = h.password

                else:
                    status, password = turing.get_hash_password(h.password_hash_type, h.value)

                    if status == 1:
                        h.password = password
                    else:
                        flag_has_unknown_password = True

                model.append((h.password_hash_type, h.value, password, STATUS[status], h.description, h))

        # enable/disable options
        model.set_sort_column_id(0, Gtk.SortType.ASCENDING)
        self.__tableview.set_model(model)
        self.__test_keywords_toolitem.set_sensitive(flag_has_unknown_password)
        self.__status_label.set_text(f'{password_count:d} of {count:d} passwords found.')
        self.__widget.show_content()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save current state
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        mobius.framework.set_config('evidence-password-hashes.vpaned-pos', self.__vpaned.get_position())
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle main tableview selection-changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def main_tableview_on_selection_changed(self, selection_list):
        self.__metadata_view.clear()

        if selection_list:
            row_id, row_data = selection_list[0]
            h = row_data[-1]

            self.__metadata_view.add_row(('Hash Type', h.password_hash_type))
            self.__metadata_view.add_row(('Value', h.value))
            self.__metadata_view.add_row(('Description', h.description))

            if h.has_attribute("password"):
                self.__metadata_view.add_row(('Password', h.password))

            for name, value in h.metadata.get_values():
                self.__metadata_view.add_row((pymobius.id_to_name(name), pymobius.to_string(value)))

            self.__metadata_view.set_report_name('%s metadata' % h.description)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle test keywords from file option
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_test_keywords(self, widget, *args):

        # choose file
        fs = Gtk.FileChooserDialog(title='Choose wordlist file...', action=Gtk.FileChooserAction.OPEN)
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)

        filefilter = Gtk.FileFilter()
        filefilter.set_name('John The Ripper .pot file (*.pot)')
        filefilter.add_pattern('*.pot')
        filefilter.filetype = 'pot'
        fs.add_filter(filefilter)

        filefilter = Gtk.FileFilter()
        filefilter.set_name('Wordlist (*.txt)')
        filefilter.add_pattern('*.txt')
        filefilter.filetype = 'wordlist'
        fs.add_filter(filefilter)

        rc = fs.run()
        uri = fs.get_uri()
        filefilter = fs.get_filter()
        filetype = filefilter.filetype
        fs.destroy()

        if rc != Gtk.ResponseType.OK:
            return

        # check if file is empty
        f = mobius.io.new_file_by_url(uri)
        size = f.size

        if size == 0:
            return

        # get hashes with unknown password
        hash_testing = HashTesting()

        for row in self.__tableview:
            h = row[-1]

            if not h.has_attribute("password"):
                hash_testing.add(h)

        # test keywords
        self.__test_keywords_progress_bar.show()
        reader = f.new_reader()
        fp = mobius.io.line_reader(reader, 'utf-8', '\n')
        pct = -1

        for line in fp:
            if filetype == 'pot' and ':' in line:
                keyword = line.split(':', 1)[1]
            else:
                keyword = line

            fraction = float(reader.tell()) / float(size)
            tmp_pct = int(fraction * 100)

            if tmp_pct != pct:
                self.__test_keywords_progress_bar.set_fraction(fraction)
                self.__test_keywords_progress_bar.set_text('%d %%' % tmp_pct)
                mobius.ui.flush()
                pct = tmp_pct

            hash_testing.test(keyword)

            if not hash_testing:  # no more hashes to test
                break

        # update tableview
        hash_testing.update()
        count = 0

        for row in self.__tableview:
            h = row[-1]
            status = row[3]

            if status != 'found' and h.has_attribute("password"):
                row[2] = h.password
                row[3] = 'found'
                count += 1

        # update view
        self.__test_keywords_progress_bar.hide()
        self.__status_label.set_text('Passwords found: %d' % count)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Export .hashcat file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_hashcat(self, uri, data):

        # create writer
        f = mobius.io.new_file_by_url(uri)
        fp = mobius.io.text_writer(f.new_writer())

        # export data
        for row in self.__tableview:
            h = row[-1]
            metadata = h.metadata
            username = metadata.get('username')

            if h.has_password():
                pass  # do not export hashes already found

            elif h.type == 'lm':
                fp.write('%s\n' % h.value)

            elif h.type == 'nt':
                fp.write('%s\n' % h.value)

            elif h.type == 'msdcc1':
                fp.write('%s:%s\n' % (h.value, username))

            elif h.type == 'msdcc2':
                iterations = metadata.get('iterations')
                fp.write('$DCC2$%s#%s#%s\n' % (iterations, username, h.value))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Export .john hash file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_john(self, uri, data):

        # create writer
        f = mobius.io.new_file_by_url(uri)
        fp = mobius.io.text_writer(f.new_writer())

        # export data
        for row in self.__tableview:
            h = row[-1]
            metadata = h.metadata
            username = metadata.get('username')
            rid = metadata.get('user_rid')
            gid = metadata.get('user_gid')
            gecos = metadata.get('fullname') or metadata.get('user_comment')

            if h.has_password():
                pass  # do not export hashes already found

            elif h.type == 'nt':
                fp.write('%s:$NT$%s:%s:%s:%s::\n' % (username, h.value, rid, gid, gecos))

            elif h.type == 'lm':
                fp.write('%s:$LM$%s:%s:%s:%s::\n' % (username, h.value, rid, gid, gecos))

            elif h.type == 'msdcc1':
                fp.write('%s:M$%s#%s:%s:%s:%s::\n' % (username, username, h.value, rid, gid, gecos))

            elif h.type == 'msdcc2':
                iterations = metadata.get('iterations')
                fp.write('%s:$DCC2$%s#%s#%s:%s:%s:%s::\n' % (username, iterations, username, h.value, rid, gid, gecos))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Export .pot file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_pot(self, uri, data):

        # create writer
        f = mobius.io.new_file_by_url(uri)
        fp = mobius.io.text_writer(f.new_writer())

        # export data
        exported = set()

        for row in self.__tableview:
            h = row[-1]
            metadata = h.metadata
            username = metadata.get('username')

            if h.has_password() and (h.type, h.value) not in exported:
                if h.type == 'nt':
                    fp.write('$NT$%s:%s\n' % (h.value, h.password))

                elif h.type == 'lm':
                    if ('lm', h.value[:16]) not in exported:
                        fp.write('$LM$%s:%s\n' % (h.value[:16], h.password[:7].upper()))
                        exported.add(('lm', h.value[:16]))
                    if ('lm', h.value[16:]) not in exported:
                        fp.write('$LM$%s:%s\n' % (h.value[16:], h.password[7:].upper()))
                        exported.add(('lm', h.value[16:]))

                elif h.type == 'msdcc1':
                    fp.write('M$%s#%s:%s\n' % (h.value, username, h.password))

                elif h.type == 'msdcc2':
                    iterations = metadata.get('iterations')
                    fp.write('$DCC2$%d#%s#%s:%s\n' % (iterations, username, h.value, h.password))

                exported.add((h.type, h.value))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Export wordlist file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_wordlist(self, uri, data):
        keywords = set()

        for row in self.__tableview:
            h = row[-1]

            if h.has_password():
                keywords.add(h.password)

            metadata = h.metadata
            keywords.update(REGEX_WORDS.findall(metadata.get('username')))
            keywords.update(REGEX_WORDS.findall(metadata.get('fullname')))
            keywords.update(REGEX_WORDS.findall(metadata.get('admin_comment')))
            keywords.update(REGEX_WORDS.findall(metadata.get('user_comment')))

        # create writer
        f = mobius.io.new_file_by_url(uri)
        fp = mobius.io.text_writer(f.new_writer())

        # export data
        for pwd in sorted(keywords):
            fp.write('%s\n' % pwd)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    mobius.core.add_resource('evidence.widget.password-hashes', 'Evidence: Password Hashes', HashesView)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('evidence.widget.password-hashes')
