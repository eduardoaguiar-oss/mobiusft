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
import mobius
import pymobius
from gi.repository import Gtk

from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SKYPE_DATATYPE = {0: 'number', 3: 'string', 4: 'raw'}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Voicemail object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Voicemail(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.timestamp = None
        self.skypename = None
        self.displayname = None
        self.duration = None
        self.path = None
        self.size = None
        self.subject = None


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief User object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class User(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.skypename = None
        self.gender = None
        self.birthday = None
        self.phone = None
        self.phone_home = None
        self.phone_office = None
        self.phone_mobile = None
        self.country = None
        self.province = None
        self.city = None
        self.homepage = None
        self.languages = None
        self.emails = None
        self.about = None
        self.displayname = None
        self.registration_timestamp = None
        self.profile_timestamp = None
        self.last_online_timestamp = None
        self.last_used_timestamp = None
        self.mood_timestamp = None
        self.mood_text = None
        self.avatar_timestamp = None
        self.avatar_image = None
        self.balance = None
        self.balance_currency = None


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Skype agent
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SkypeAgent(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__records = []
        self.__timeline = set()
        self.__contacts = []
        self.__voicemails = []

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add logfile to model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_logfile(self, stream):
        for resource in mobius.core.get_resources('skype.agent'):
            agent = resource.value()

            if agent.check_signature(stream):
                agent.parse_logfile(self, stream)
                return

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get contacts
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_contacts(self):
        return self.__contacts

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get voicemails
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_voicemails(self):
        return self.__voicemails

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief iter timeline
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def iter_timeline(self):
        return sorted(self.__timeline)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief iter records
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def iter_records(self):
        return iter(self.__records)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add record
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_record(self, record):
        self.__records.append(record)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add timeline event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_timeline(self, timestamp, event):
        self.__timeline.add((timestamp, event))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief create contact
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def new_contact(self):
        contact = User()
        self.__contacts.append(contact)

        return contact

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief create voicemail
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def new_voicemail(self):
        voicemail = Voicemail()
        self.__voicemails.append(voicemail)

        return voicemail


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief User list widget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class UserListWidget(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__details = {}
        self.__details_report_name = None

        # vpaned
        self.__hpaned = Gtk.HPaned()
        self.__hpaned.set_border_width(5)
        self.__widget = self.__hpaned

        position = mobius.framework.get_config('skype-agent.userlist-hpaned-position')
        if position:
            self.__hpaned.set_position(position)

        # master - vpaned
        self.__vpaned = Gtk.VPaned()
        self.__vpaned.set_border_width(5)
        self.__vpaned.show()
        self.__hpaned.pack1(self.__vpaned, True, True)

        position = mobius.framework.get_config('skype-agent.userlist-vpaned-position')
        if position:
            self.__vpaned.set_position(position)

        # master - listview
        self.__user_listview = self.__mediator.call('ui.new-widget', 'tableview')
        self.__user_listview.set_control(self, 'user_listview')

        column = self.__user_listview.add_column('skypename')
        column.is_sortable = True

        column = self.__user_listview.add_column('name')
        column.is_sortable = True

        column = self.__user_listview.add_column('phone number')
        column.is_sortable = True

        column = self.__user_listview.add_column('user', column_type='object')
        column.is_visible = False

        self.__user_listview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__user_listview.show()
        self.__vpaned.pack1(self.__user_listview.get_ui_widget(), True, True)

        # user image
        self.__image = mobius.ui.new_icon_by_name('image-missing', mobius.ui.icon.size_large)
        self.__image.set_visible(True)
        self.__vpaned.pack2(self.__image.get_ui_widget(), False, True)

        # user details
        self.__details_listview = self.__mediator.call('ui.new-widget', 'attribute-list')
        self.__details_listview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__details_listview.show()
        self.__hpaned.pack2(self.__details_listview.get_ui_widget(), True, True)

        # add common details
        self.add_detail('skypename')
        self.add_detail('fullname')
        self.add_detail('displayname', 'display name')
        self.add_detail('gender')
        self.add_detail('birthday')
        self.add_detail('city')
        self.add_detail('province')
        self.add_detail('country')
        self.add_detail('languages')
        self.add_detail('phone', 'phone number')
        self.add_detail('phone_home', 'home phone number')
        self.add_detail('phone_office', 'office phone number')
        self.add_detail('phone_mobile', 'mobile phone number')
        self.add_detail('emails', 'e-mails')
        self.add_detail('homepage')
        self.add_detail('about')
        self.add_detail('mood_text', 'mood text')
        self.add_detail('balance')
        self.add_detail('balance_currency', 'balance currency')
        self.add_detail('timezone', 'last connection timezone')
        self.add_detail('ipcountry', 'last connection country')
        self.add_detail('registration_timestamp', 'registration date/time')
        self.add_detail('profile_timestamp', 'profile last saved')
        self.add_detail('last_online_timestamp', 'last time seen online')
        self.add_detail('last_used_timestamp', 'last time used')
        self.add_detail('avatar_timestamp', 'avatar last modification')
        self.add_detail('mood_timestamp', 'mood text last modification')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief show widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def show(self):
        return self.__widget.show()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief clear widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def clear(self):
        self.__user_listview.clear()
        self.__details_listview.clear_values()
        self.__image.set_icon_by_name('image-missing', mobius.ui.icon.size_large)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set widget sensitive status
    # @param sensitive True/False
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_sensitive(self, sensitive):
        return self.__widget.set_sensitive(sensitive)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief destroy widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def destroy(self):
        transaction = mobius.framework.new_config_transaction()
        mobius.framework.set_config('skype-agent.userlist-vpaned-position', self.__vpaned.get_position())
        mobius.framework.set_config('skype-agent.userlist-hpaned-position', self.__hpaned.get_position())
        transaction.commit()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set list report id
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_list_report_id(self, report_id):
        self.__user_listview.set_report_id(report_id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set list report name
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_list_report_name(self, report_name):
        self.__user_listview.set_report_name(report_name)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set details report id
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_details_report_id(self, report_id):
        self.__details_listview.set_report_id(report_id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set details report name
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_details_report_name(self, report_name):
        self.__details_report_name = report_name

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add detail to user list
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_detail(self, value_id, value_name=None):
        name = value_name or value_id
        self.__details[value_id] = name
        self.__details_listview.add_value(value_id, name)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add user to list
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_user(self, account):
        name = account.fullname or account.displayname or account.skypename
        phone = account.phone or account.phone_mobile or account.phone_home or account.phone_office
        self.__user_listview.add_row((account.skypename, name, phone, account))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle user_listview selection-changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def user_listview_on_selection_changed(self, selection_list):
        self.__details_listview.clear_values()

        if not selection_list:
            self.__details_listview.set_sensitive(False)

        else:
            row_number, row_data = selection_list[0]
            account = row_data[-1]

            # attributes
            for data_id in self.__details:
                data_value = getattr(account, data_id, None)

                if data_id == 'gender':
                    if data_value == 1:
                        data_value = 'male'
                    elif data_value == 2:
                        data_value = 'female'
                    else:
                        data_value = ''

                self.__details_listview.set_value(data_id, data_value)

            # user avatar image
            if account.avatar_image:
                try:
                    self.__image.set_icon_from_data(account.avatar_image, mobius.ui.icon.size_large)
                except Exception as e:
                    self.__image.set_icon_by_name('image-missing', mobius.ui.icon.size_large)
            else:
                self.__image.set_icon_by_name('image-missing', mobius.ui.icon.size_large)

            # report name
            if self.__details_report_name:
                report_name = self.__details_report_name % (account.skypename or account.displayname)
                self.__details_listview.set_report_name(report_name)

            # set details sensitive
            self.__details_listview.set_sensitive(True)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief main widget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
VIEW_CONTACTS, VIEW_VOICEMAILS, VIEW_TIMELINE, VIEW_RECORDS = range(4)


class Widget(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__widget = mobius.ui.box(mobius.ui.box.orientation_vertical)
        self.__widget.set_border_width(10)
        self.__widget.set_spacing(5)
        self.__widget.set_visible(True)
        self.__mediator = pymobius.mediator.copy()

        # menubar
        menubar = Gtk.MenuBar()
        menubar.show()
        self.__widget.add_child(menubar, mobius.ui.box.fill_none)

        item = Gtk.MenuItem.new_with_mnemonic('_File')
        item.show()
        menubar.append(item)

        menu = Gtk.Menu()
        menu.show()
        item.set_submenu(menu)

        item = Gtk.MenuItem.new_with_mnemonic('_Open')
        item.connect("activate", self.on_file_open)
        item.show()
        menu.append(item)

        item = Gtk.SeparatorMenuItem.new()
        item.show()
        menu.append(item)

        item = Gtk.MenuItem.new_with_mnemonic('_Quit')
        item.connect("activate", self.__on_extension_quit)
        item.show()
        menu.append(item)

        self.__view_menu_entry = Gtk.MenuItem.new_with_mnemonic('_View')
        self.__view_menu_entry.set_sensitive(False)
        self.__view_menu_entry.show()
        menubar.append(self.__view_menu_entry)

        menu = Gtk.Menu()
        menu.show()
        self.__view_menu_entry.set_submenu(menu)

        self.__view_menuitems = {}

        menuitem = Gtk.RadioMenuItem(group=None, label='Contacts')
        menuitem.connect('activate', self.on_view_toggled, VIEW_CONTACTS)
        menuitem.show()
        menu.append(menuitem)
        self.__view_menuitems[VIEW_CONTACTS] = menuitem
        menugroup = menuitem

        menuitem = Gtk.RadioMenuItem(group=menugroup, label='Voicemails')
        menuitem.connect('activate', self.on_view_toggled, VIEW_VOICEMAILS)
        menuitem.show()
        menu.append(menuitem)
        self.__view_menuitems[VIEW_VOICEMAILS] = menuitem

        item = Gtk.SeparatorMenuItem.new()
        item.show()
        menu.append(item)

        menuitem = Gtk.RadioMenuItem(group=menugroup, label='Timeline')
        menuitem.connect('activate', self.on_view_toggled, VIEW_TIMELINE)
        menuitem.show()
        menu.append(menuitem)
        self.__view_menuitems[VIEW_TIMELINE] = menuitem

        menuitem = Gtk.RadioMenuItem(group=menugroup, label='Records')
        menuitem.connect('activate', self.on_view_toggled, VIEW_RECORDS)
        menuitem.show()
        menu.append(menuitem)
        self.__view_menuitems[VIEW_RECORDS] = menuitem

        # toolbar
        toolbar = Gtk.Toolbar()
        toolbar.set_style(Gtk.ToolbarStyle.ICONS)
        toolbar.show()
        self.__widget.add_child(toolbar, mobius.ui.box.fill_none)

        toolitem = Gtk.ToolButton.new()
        toolitem.set_icon_name('document-open')
        toolitem.connect("clicked", self.on_file_open)
        toolitem.show()
        toolitem.set_tooltip_text("Open database files")
        toolbar.insert(toolitem, -1)

        self.__report_toolitem = Gtk.ToolButton.new()

        image = mobius.ui.new_icon_by_name('report-run', mobius.ui.icon.size_toolbar)
        image.show()

        self.__report_toolitem.set_icon_widget(image.get_ui_widget())
        self.__report_toolitem.set_sensitive(False)
        self.__report_toolitem.connect("clicked", self.on_generate_report)
        self.__report_toolitem.show()
        self.__report_toolitem.set_tooltip_text("Generate report")
        toolbar.insert(self.__report_toolitem, -1)

        toolitem = Gtk.SeparatorToolItem()
        toolitem.show()
        toolbar.insert(toolitem, -1)

        self.__view_toolitems = {}

        toolitem = Gtk.RadioToolButton()
        toolitem.set_sensitive(False)
        toolitem.show()
        toolitem.set_tooltip_text('contacts')
        toolitem.connect('toggled', self.on_view_toggled, VIEW_CONTACTS)
        toolbar.insert(toolitem, -1)
        toolgroup = toolitem
        self.__view_toolitems[VIEW_CONTACTS] = toolitem

        path = pymobius.mediator.call('extension.get-resource-path', EXTENSION_ID, 'view-contacts.png')
        image = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_dnd)
        image.show()
        toolitem.set_icon_widget(image.get_ui_widget())

        toolitem = Gtk.RadioToolButton(group=toolgroup)
        toolitem.set_sensitive(False)
        toolitem.show()
        toolitem.set_tooltip_text('voicemails')
        toolitem.connect('toggled', self.on_view_toggled, VIEW_VOICEMAILS)
        toolbar.insert(toolitem, -1)
        self.__view_toolitems[VIEW_VOICEMAILS] = toolitem

        path = pymobius.mediator.call('extension.get-resource-path', EXTENSION_ID, 'view-voicemails.png')
        image = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_dnd)
        image.show()
        toolitem.set_icon_widget(image.get_ui_widget())

        toolitem = Gtk.SeparatorToolItem()
        toolitem.set_sensitive(False)
        toolitem.show()
        toolbar.insert(toolitem, -1)

        toolitem = Gtk.RadioToolButton(group=toolgroup)
        toolitem.set_sensitive(False)
        toolitem.show()
        toolitem.set_tooltip_text('events timeline')
        toolitem.connect('toggled', self.on_view_toggled, VIEW_TIMELINE)
        toolbar.insert(toolitem, -1)
        self.__view_toolitems[VIEW_TIMELINE] = toolitem

        path = pymobius.mediator.call('extension.get-resource-path', EXTENSION_ID, 'view-timeline.png')
        image = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_dnd)
        image.show()
        toolitem.set_icon_widget(image.get_ui_widget())

        toolitem = Gtk.RadioToolButton(group=toolgroup)
        toolitem.set_sensitive(False)
        toolitem.show()
        toolitem.set_tooltip_text('records')
        toolitem.connect('toggled', self.on_view_toggled, VIEW_RECORDS)
        toolbar.insert(toolitem, -1)
        self.__view_toolitems[VIEW_RECORDS] = toolitem

        path = pymobius.mediator.call('extension.get-resource-path', EXTENSION_ID, 'view-records.png')
        image = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_dnd)
        image.show()
        toolitem.set_icon_widget(image.get_ui_widget())

        # notebook
        self.__notebook = Gtk.Notebook()
        self.__notebook.set_show_tabs(False)
        self.__notebook.set_sensitive(False)
        self.__notebook.show()
        self.__widget.add_child(self.__notebook, mobius.ui.box.fill_with_widget)

        # tab: contacts view
        self.__contact_view = UserListWidget()
        self.__contact_view.set_list_report_id('skype.contacts')
        self.__contact_view.set_list_report_name('Skype contacts')
        self.__contact_view.set_details_report_id('skype.contact')
        self.__contact_view.set_details_report_name('skype contact')
        self.__contact_view.show()

        self.__notebook.append_page(self.__contact_view.get_ui_widget(), mobius.ui.label('contacts').get_ui_widget())

        # tab: voicemail view
        self.__voicemail_listview = pymobius.mediator.call('ui.new-widget', 'tableview')

        column = self.__voicemail_listview.add_column('date')
        column.is_sortable = True

        column = self.__voicemail_listview.add_column('skypename')
        column.is_sortable = True

        column = self.__voicemail_listview.add_column('displayname')
        column.is_sortable = True

        column = self.__voicemail_listview.add_column('duration')
        column.is_sortable = True

        column = self.__voicemail_listview.add_column('path')
        column.is_sortable = True

        self.__voicemail_listview.set_report_id('skype.voicemail')
        self.__voicemail_listview.set_report_name('Skype voicemail')
        self.__voicemail_listview.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
        self.__voicemail_listview.show()

        self.__notebook.append_page(self.__voicemail_listview.get_ui_widget(), mobius.ui.label('voicemails').get_ui_widget())

        # tab: timeline view
        self.__timeline_view = pymobius.mediator.call('ui.new-widget', 'tableview')

        column = self.__timeline_view.add_column('date')
        column = self.__timeline_view.add_column('description')

        self.__timeline_view.set_report_id('skype.timeline')
        self.__timeline_view.set_report_name('Skype timeline')
        self.__timeline_view.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
        self.__timeline_view.show()

        self.__notebook.append_page(self.__timeline_view.get_ui_widget(), mobius.ui.label('timeline').get_ui_widget())

        # tab: record view
        self.__record_view = pymobius.mediator.call('ui.new-widget', 'tableview')

        column = self.__record_view.add_column('sequence', column_type='int')
        column.is_sortable = True

        column = self.__record_view.add_column('tag')
        column.is_sortable = True

        column = self.__record_view.add_column('datatype')

        column = self.__record_view.add_column('value')

        self.__record_view.set_report_id('skype.records')
        self.__record_view.set_report_name('Skype log records')
        self.__record_view.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__record_view.show()

        self.__notebook.append_page(self.__record_view.get_ui_widget(), mobius.ui.label('records').get_ui_widget())

        # status bar
        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.IN)
        frame.show()
        self.__widget.add_child(frame, mobius.ui.box.fill_none)

        self.__status_label = mobius.ui.label()
        self.__status_label.set_selectable(True)
        self.__status_label.set_halign(mobius.ui.label.align_left)
        self.__status_label.set_elide_mode(mobius.ui.label.elide_middle)
        self.__status_label.set_visible(True)
        frame.add(self.__status_label.get_ui_widget())

        # data model
        self.__agent = SkypeAgent()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_widget(self):
        return self.__widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle stop event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_widget_stopped(self):

        # show confirmation dialog
        dialog = mobius.ui.message_dialog(mobius.ui.message_dialog.type_question)
        dialog.text = f'Do you want to quit from {EXTENSION_NAME}?'
        dialog.add_button(mobius.ui.message_dialog.button_yes)
        dialog.add_button(mobius.ui.message_dialog.button_no)
        dialog.set_default_response(mobius.ui.message_dialog.button_no)
        rc = dialog.run()

        if rc != mobius.ui.message_dialog.button_yes:
            return True

        # close extension
        self.__contact_view.destroy()
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle close button
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_extension_quit(self, widget, *args):
        self.__mediator.call('ui.working-area.close', self.working_area.id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle view toggle
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_view_toggled(self, widget, view_id, *args):
        if widget.get_active():
            self.__notebook.set_current_page(view_id)
            self.__view_menuitems[view_id].set_active(True)
            self.__view_toolitems[view_id].set_active(True)
            self.__status_label.set_text('')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief model modified event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_model_modified(self):
        self.__record_view.clear()
        self.__contact_view.clear()
        self.__voicemail_listview.clear()
        self.__timeline_view.clear()

        # contacts
        for user in self.__agent.get_contacts():
            self.__contact_view.add_user(user)

        # voicemails
        for voicemail in self.__agent.get_voicemails():
            self.__voicemail_listview.add_row(
                (voicemail.timestamp, voicemail.skypename, voicemail.displayname, voicemail.duration, voicemail.path))

        # timeline
        for event_date, event_description in self.__agent.iter_timeline():
            self.__timeline_view.add_row((event_date, event_description))

        # records
        for record in self.__agent.iter_records():
            tags = {}

            for tag in record.taglist:
                tags[tag.id] = tag.value
                datatype = SKYPE_DATATYPE.get(tag.datatype, 'unknown: %02x' % tag.datatype)

                if tag.datatype in (0x00, 0x03):
                    value = tag.value
                else:
                    value = '<binary>'
                self.__record_view.add_row((record.sequence, '0x%04x' % tag.id, datatype, value))

        # enable window options
        self.__view_menu_entry.set_sensitive(True)
        self.__notebook.set_sensitive(True)
        self.__report_toolitem.set_sensitive(True)

        for toolitem in self.__view_toolitems.values():
            toolitem.set_sensitive(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle file->open
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_file_open(self, widget, *args):

        # build filechooser
        fs = Gtk.FileChooserDialog(title='Select Skype log files')
        fs.set_select_multiple(True)
        fs.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        fs.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)

        # add filters
        filefilter = Gtk.FileFilter()
        filefilter.set_name('Skype log files (*.dbb, *.db)')
        filefilter.add_pattern('*.dbb')
        filefilter.add_pattern('*.db')
        fs.add_filter(filefilter)

        filefilter = Gtk.FileFilter()
        filefilter.set_name('All files (*)')
        filefilter.add_pattern('*')
        fs.add_filter(filefilter)

        # set current folder
        last_folder = mobius.framework.get_config('skype-agent.folder') or mobius.framework.get_config('general.data-folder')
        if last_folder:
            fs.set_current_folder(last_folder)

        # run dialog
        rc = fs.run()
        uri_list = fs.get_uris()
        folder = fs.get_current_folder()
        fs.destroy()

        # if user hit OK add files
        if rc == Gtk.ResponseType.OK:
            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('skype-agent.folder', folder)
            mobius.framework.set_config('general.data-folder', folder)
            transaction.commit()

            # interpret log files
            self.__agent = SkypeAgent()

            for uri in uri_list:
                f = mobius.io.new_file_by_url(uri)
                self.__status_label.set_text(f'Reading file {f.name}...')
                mobius.ui.flush()

                stream = f.new_reader()
                self.__agent.add_logfile(stream)

            # update views
            self.on_model_modified()
            self.__status_label.set_text('')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_generate_report(self, widget, *args):
        dialog = self.__mediator.call('report.run-dialog')
        dialog.run(self.__agent)
        dialog.destroy()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    icon = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)
    mobius.core.add_resource('menu.tools.' + EXTENSION_ID, 'Menu Tool: Skype Agent',
                             (icon, EXTENSION_NAME, on_activate))


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('menu.tools.' + EXTENSION_ID)
    pymobius.mediator.call('ui.working-area.del', EXTENSION_ID)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief on_activate
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def on_activate(item_id):
    widget = Widget()
    icon_path = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)

    working_area = pymobius.mediator.call('ui.working-area.new', EXTENSION_ID)
    working_area.set_default_size(700, 500)
    working_area.set_title(EXTENSION_NAME)
    working_area.set_icon(icon_path)
    working_area.set_widget(widget)
    working_area.show()
