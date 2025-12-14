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
import mobius
import mobius.core.decoder
import pymobius

from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: FileView
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class FileView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__control = control
        self.__mediator = pymobius.mediator.copy()

        # build widget
        self.__widget = self.__mediator.call('ui.new-widget', 'view-selector')

        view = FilePropertiesView(self)
        self.__widget.add('properties', view)

        view = FileHexView(self)
        self.__widget.add('hexview', view)

        view = ContentPropertiesView(self)
        self.__widget.add('content-properties', view)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Show widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def show(self):
        return self.__widget.show()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Hide widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def hide(self):
        return self.__widget.hide()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set widget sensitive status
    # @param sensitive True/False
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_sensitive(self, sensitive):
        return self.__widget.set_sensitive(sensitive)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save current state
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, entries):
        self.__widget.set_data(entries)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: FileProperties
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class FilePropertiesView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__control = control
        self.__mediator = pymobius.mediator.copy()
        self.name = 'File Properties'

        icon_path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'fileview_properties.png')
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.core.ui.container()
        self.__widget.show()

        self.__details_view = self.__mediator.call('ui.new-widget', 'attribute-list')
        self.__details_view.add_value('name', 'Name')
        self.__details_view.add_value('short_name', 'Short name')
        self.__details_view.add_value('path', 'Path')
        self.__details_view.add_value('size', 'Size')
        self.__details_view.add_value('inode', 'Inode')
        self.__details_view.add_value('type', 'Type')
        self.__details_view.add_value('permissions', 'Permissions')
        self.__details_view.add_value('uid', 'User ID')
        self.__details_view.add_value('gid', 'Group ID')
        self.__details_view.add_value('is_deleted', 'Is deleted')
        self.__details_view.add_value('is_reallocated', 'Is reallocated')
        self.__details_view.add_value('crtime', 'Creation date/time')
        self.__details_view.add_value('dtime', 'Deletion date/time')
        self.__details_view.add_value('atime', 'Last access date/time')
        self.__details_view.add_value('ctime', 'Last metadata date/time')
        self.__details_view.add_value('mtime', 'Last modification date/time')

        self.__details_view.set_report_id('entry.metadata')
        self.__details_view.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__details_view.show()
        self.__details_view.set_sensitive(False)
        self.__widget.set_content(self.__details_view.get_ui_widget())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get ui widget
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
    def set_data(self, entries):

        if len(entries) != 1:
            self.__details_view.clear_values()
            self.__details_view.set_sensitive(False)

        else:
            entry = entries[0]

            # File types
            TYPES = {
                0: 'Undefined',
                1: 'Block device',
                2: 'Char device',
                3: 'FIFO',
                4: 'Symbolic link',
                5: 'Regular file',
                6: 'Socket'
            }

            # permissions
            perm = ''
            perm += 'r' if entry.permissions & 0o0400 else '-'
            perm += 'w' if entry.permissions & 0o0200 else '-'
            perm += 'x' if entry.permissions & 0o0100 else '-'
            perm += 'r' if entry.permissions & 0o0040 else '-'
            perm += 'w' if entry.permissions & 0o0020 else '-'
            perm += 'x' if entry.permissions & 0o0010 else '-'
            perm += 'r' if entry.permissions & 0o0004 else '-'
            perm += 'w' if entry.permissions & 0o0002 else '-'
            perm += 'x' if entry.permissions & 0o0001 else '-'

            self.__details_view.set_value('name', entry.name)
            self.__details_view.set_value('short_name', entry.short_name)
            self.__details_view.set_value('path', entry.path)

            if entry.size == 1:
                self.__details_view.set_value('size', '1 byte')
            else:
                self.__details_view.set_value('size', '%d bytes' % entry.size)

            if entry.is_file():
                self.__details_view.set_value('type', TYPES.get(entry.type, 'unknown: %d' % entry.type))
            else:
                self.__details_view.set_value('type', 'Folder')

            self.__details_view.set_value('inode', entry.inode)
            self.__details_view.set_value('permissions', '%s (%04o)' % (perm, entry.permissions))
            self.__details_view.set_value('uid', entry.user_id)
            self.__details_view.set_value('gid', entry.group_id)
            self.__details_view.set_value('is_deleted', entry.is_deleted())
            self.__details_view.set_value('is_reallocated', entry.is_reallocated())
            self.__details_view.set_value('crtime', entry.creation_time)
            self.__details_view.set_value('dtime', entry.deletion_time)
            self.__details_view.set_value('atime', entry.access_time)
            self.__details_view.set_value('mtime', entry.modification_time)
            self.__details_view.set_value('ctime', entry.metadata_time)
            self.__details_view.set_report_name('%s file properties' % entry.name)
            self.__details_view.set_sensitive(True)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: FileHexView
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class FileHexView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__control = control
        self.__mediator = pymobius.mediator.copy()
        self.name = 'Hex Viewer'

        icon_path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'fileview_hex.png')
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.core.ui.container()
        self.__widget.show()

        self.__hexview = self.__mediator.call('ui.new-widget', 'hexview')
        self.__hexview.show()
        self.__widget.set_content(self.__hexview.get_ui_widget())

        self.__widget.set_message('Select file to show data')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get ui widget
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
    def set_data(self, entries):

        # can show data from one entry only
        if len(entries) == 0:
            self.__widget.set_message('No entry selected')
            return

        elif len(entries) > 1:
            self.__widget.set_message('More than one entry selected')
            return

        # entry must be a file, not folder
        entry = entries[0]
        if not entry.is_file():
            self.__widget.set_message('Select file to show data')
            return

        # set hexview
        reader = entry.new_reader()
        self.__hexview.set_reader(reader)
        self.__widget.show_content()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: ContentProperties
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ContentPropertiesView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__control = control
        self.__mediator = pymobius.mediator.copy()
        self.name = 'Content Properties'

        icon_path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'content_properties.png')
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = mobius.core.ui.container()
        self.__widget.show()

        # tableview
        self.__tableview = self.__mediator.call('ui.new-widget', 'tableview')
        self.__tableview.set_report_id('content-properties')
        self.__tableview.set_report_name('File Content Properties')
        self.__tableview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__tableview.set_control(self, 'main_tableview')

        column = self.__tableview.add_column('property', 'Property')
        column.is_sortable = True

        column = self.__tableview.add_column('value', 'Value')
        column.is_sortable = True

        self.__tableview.show()
        self.__widget.set_content(self.__tableview.get_ui_widget())

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
    def set_data(self, entries):

        # can show data from one entry only
        if len(entries) == 0:
            self.__widget.set_message('No entry selected')
            return

        elif len(entries) > 1:
            self.__widget.set_message('More than one entry selected')
            return

        # entry must be a file, not folder
        entry = entries[0]
        if not entry.is_file():
            self.__widget.set_message('Select file to show content properties')
            return

        # get filetype
        filetype = mobius.core.decoder.get_filetype(entry.new_reader())

        if filetype == 'empty':
            self.__widget.set_message('File is empty')
            return

        # get content properties
        HANDLERS = {'data.ms.shllink': self.__get_data_ms_shllink}
        handler = HANDLERS.get(filetype)

        if not handler:
            self.__widget.set_message('Unhandled filetype')
            return

        try:
            properties = handler(entry)
        except Exception as e:
            self.__widget.set_message(str(e))
            return

        # show content properties
        self.__tableview.clear()

        for name, value in properties:
            self.__tableview.add_row((name, value))

        self.__tableview.set_report_name(f'{entry.name} content properties')
        self.__widget.show_content()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get data.ms.shllink properties
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_data_ms_shllink(self, entry):
        properties = []

        try:
            lnk = mobius.core.decoder.lnk(entry.new_reader())

        except Exception as e:
            self.__widget.set_message(str(e))
            return

        DRIVE_TYPE = {0: 'Unknown', 1: 'No root dir', 2: 'Removable', 3: 'Fixed', 4: 'Remote', 5: 'CD-ROM',
                      6: 'Ramdisk'}

        properties.append(('Header size', lnk.header_size))
        properties.append(('CLSID', lnk.clsid))
        properties.append(('Flags', '0x%08x' % lnk.flags))
        properties.append(('File attributes', '0x%08x' % lnk.file_attributes))
        properties.append(('Name', lnk.name))
        properties.append(('Local base path', lnk.local_base_path))
        properties.append(('Common path suffix', lnk.common_path_suffix))
        properties.append(('Relative path', lnk.relative_path))
        properties.append(('Drive type', DRIVE_TYPE.get(lnk.drive_type, lnk.drive_type)))
        properties.append(('Drive serial number', '0x%08x' % lnk.drive_serial_number))
        properties.append(('Volume label', lnk.volume_label))
        properties.append(('Net name', lnk.net_name))
        properties.append(('Device name', lnk.device_name))
        properties.append(('NetBIOS name', lnk.netbios_name))
        properties.append(('Target file size', lnk.file_size))
        properties.append(('Target creation time', lnk.creation_time))
        properties.append(('Target access time', lnk.access_time))
        properties.append(('Target write time', lnk.write_time))
        properties.append(('Is target readonly', lnk.is_target_readonly()))
        properties.append(('Is target hidden', lnk.is_target_hidden()))
        properties.append(('Is target system', lnk.is_target_system()))
        properties.append(('Is target directory', lnk.is_target_directory()))
        properties.append(('Is target archive', lnk.is_target_archive()))
        properties.append(('Is target temporary', lnk.is_target_temporary()))
        properties.append(('Is target sparse', lnk.is_target_sparse()))
        properties.append(('Is target reparse point', lnk.is_target_reparse_point()))
        properties.append(('Is target compressed', lnk.is_target_compressed()))
        properties.append(('Is target offline', lnk.is_target_offline()))
        properties.append(('Is target content indexed', lnk.is_target_content_indexed()))
        properties.append(('Is target encrypted', lnk.is_target_encrypted()))

        return properties
