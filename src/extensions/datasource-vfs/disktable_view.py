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
import mobius.core.io
import mobius.core.system
import mobius.core.vfs
import pymobius
from gi.repository import Gdk
from gi.repository import GdkPixbuf
from gi.repository import Gtk

from add_imagefile_dialog import AddImageFileDialog
from copy_dialog import CopyDialog
from imagefile_chooser_dialog import ImagefileChooserDialog
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: DiskTableView
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class DiskTableView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__mediator = pymobius.mediator.copy()
        self.__control = control
        self.__itemlist = []
        self.name = 'Disks'

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'disks.png')
        self.icon_data = open(path, 'rb').read()

        # vpaned
        self.__vpaned = Gtk.VPaned()
        self.__vpaned.show()
        self.__widget = self.__vpaned

        pos = mobius.framework.get_config('vfs-viewer.disks-vpaned-position')

        if pos:
            self.__vpaned.set_position(pos)

        # hbox
        self.__hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        self.__hbox.set_spacing(5)
        self.__hbox.set_visible(True)
        self.__vpaned.pack1(self.__hbox.get_ui_widget(), True, False)

        # tableview
        self.__tableview = self.__mediator.call('ui.new-widget', 'tableview')
        self.__tableview.set_report_id('vfs.disks')
        self.__tableview.set_report_name('VFS Disks')
        self.__tableview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__tableview.set_control(self, 'main_tableview')

        column = self.__tableview.add_column('idx', '#', column_type='int')
        column.is_sortable = True

        column = self.__tableview.add_column('type', 'Type')
        column.is_sortable = True

        column = self.__tableview.add_column('size', 'Size (bytes)', column_type='int')
        column.is_sortable = True

        column = self.__tableview.add_column('name', 'Name')
        column.is_sortable = True

        column = self.__tableview.add_column('obj', 'object', column_type='object')
        column.is_visible = False

        self.__tableview.show()
        self.__hbox.add_child(self.__tableview.get_ui_widget(), mobius.core.ui.box.fill_with_widget)

        # side buttons
        vbuttonbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbuttonbox.set_visible(True)
        self.__hbox.add_child(vbuttonbox, mobius.core.ui.box.fill_none)

        button = mobius.core.ui.button()
        button.set_icon_by_name('list-add')
        button.set_text("Add image file")
        button.set_visible(True)
        button.set_callback('clicked', self.__on_add_imagefile)
        vbuttonbox.add_child(button, mobius.core.ui.box.fill_none)

        button = mobius.core.ui.button()
        button.set_icon_by_name('list-add')
        button.set_text("Add device")
        button.set_visible(True)
        button.set_callback('clicked', self.__on_add_device)
        vbuttonbox.add_child(button, mobius.core.ui.box.fill_none)

        self.__remove_button = mobius.core.ui.button()
        self.__remove_button.set_icon_by_name('list-remove')
        self.__remove_button.set_text("_Remove")
        self.__remove_button.set_sensitive(False)
        self.__remove_button.set_visible(True)
        self.__remove_button.set_callback('clicked', self.__on_remove_disk)
        vbuttonbox.add_child(self.__remove_button, mobius.core.ui.box.fill_none)

        # details container
        self.__details_container = mobius.core.ui.container()
        self.__details_container.show()
        self.__vpaned.pack2(self.__details_container.get_ui_widget(), True, True)

        # view selector
        self.__view_selector = self.__mediator.call('ui.new-widget', 'view-selector')
        self.__view_selector.set_control(self)
        self.__view_selector.show()
        self.__details_container.set_content(self.__view_selector.get_ui_widget())

        # views
        self.__attribute_view = DiskAttrView(self)
        self.__view_selector.add('attributes', self.__attribute_view)

        self.__data_view = DiskDataView(self)
        self.__view_selector.add('data', self.__data_view)

        # toolbuttons
        toolbar = self.__view_selector.get_toolbar()

        toolitem = Gtk.SeparatorToolItem()
        toolitem.show()
        toolbar.insert(toolitem, -1)

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'export.png')
        icon = mobius.core.ui.new_icon_by_path(path, mobius.core.ui.icon.size_dnd)
        icon.set_visible(True)

        self.__export_toolitem = Gtk.ToolButton.new()
        self.__export_toolitem.set_icon_widget(icon.get_ui_widget())
        self.__export_toolitem.set_tooltip_text("Export disk as...")
        self.__export_toolitem.show()
        self.__export_toolitem.connect('clicked', self.__on_export_disk)
        toolbar.insert(self.__export_toolitem, -1)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_destroy event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        position = self.__vpaned.get_property('position')
        mobius.framework.set_config('vfs-viewer.disks-vpaned-position', position)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, vfs):
        self.__vfs = vfs
        self.__tableview.clear()

        for idx, disk in enumerate(vfs.get_disks()):
            self.__tableview.add_row((idx + 1, disk.type, disk.size, disk.name, disk))

        self.__tableview.select_row(0)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle main tableview selection-changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def main_tableview_on_selection_changed(self, selection_list):

        if selection_list:
            row_id, row_data = selection_list[0]
            self.__disk = row_data[-1]
            self.__view_selector.set_data(self.__disk)
            self.__details_container.show_content()
            self.__remove_button.set_sensitive(True)
            self.__export_toolitem.set_sensitive(True)

        else:
            self.__disk = None
            self.__details_container.set_message("Select disk")
            self.__remove_button.set_sensitive(False)
            self.__export_toolitem.set_sensitive(False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle add_imagefile event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_add_imagefile(self):
        dialog = AddImageFileDialog()
        url = dialog.run()

        if url:
            disk = mobius.core.vfs.new_disk_by_url(url)
            idx = self.__vfs.add_disk(disk)

            self.__tableview.add_row((idx + 1, disk.type, disk.size, disk.name, disk))
            self.__control.on_vfs_modified()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle add_device event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_add_device(self):
        dialog = AddDeviceDialog()
        device_uid = dialog.run()

        if device_uid:
            disk = mobius.core.vfs.new_disk_by_device_uid(device_uid)
            idx = self.__vfs.add_disk(disk)

            self.__tableview.add_row((idx + 1, disk.type, disk.size, disk.name, disk))
            self.__control.on_vfs_modified()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle remove_disk event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_remove_disk(self):

        # show confirmation dialog
        dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
        dialog.text = f"You are about to remove disk '{self.__disk.name}'. Are you sure?"
        dialog.add_button(mobius.core.ui.message_dialog.button_yes)
        dialog.add_button(mobius.core.ui.message_dialog.button_no)
        dialog.set_default_response(mobius.core.ui.message_dialog.button_no)
        rc = dialog.run()

        if rc != mobius.core.ui.message_dialog.button_yes:
            return

        # remove disk
        selection_list = self.__tableview.get_selected_rows()
        row_number, row_data = selection_list[0]

        self.__vfs.remove_disk(row_number)
        self.set_data(self.__vfs)
        self.__control.on_vfs_modified()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle export_disk event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_disk(self, widget, *args):

        # choose imagefile file name and type
        dialog = ImagefileChooserDialog('disk')
        url, imagetype = dialog.run()

        if not imagetype:
            return

        # create reader
        reader = self.__disk.new_reader()

        # create imagefile and writer
        imagefile = mobius.core.vfs.new_imagefile_by_url(url, imagetype)

        if imagetype == 'ewf':
            imagefile.segment_size = reader.size
            imagefile.compression_level = 1

        writer = imagefile.new_writer()

        # run CopyDialog
        dialog = CopyDialog(reader, writer, 'Copying disk...')
        rc = dialog.run()

        # if copy was cancelled, remove file
        if rc == Gtk.ResponseType.CANCEL:
            f = mobius.core.io.file(url)
            f.remove()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: DiskAttrView
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class DiskAttrView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__mediator = pymobius.mediator.copy()
        self.__control = control
        self.__itemlist = []
        self.name = 'Attributes'

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'attributes.png')
        self.icon_data = open(path, 'rb').read()

        # metadata view
        self.__metadata_view = self.__mediator.call('ui.new-widget', 'tableview')
        self.__metadata_view.set_report_id('vfs.disk')
        self.__metadata_view.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
        self.__metadata_view.show()
        self.__widget = self.__metadata_view

        column = self.__metadata_view.add_column('name', 'Attribute')
        column.is_sortable = True

        column = self.__metadata_view.add_column('value', 'Value')
        column.is_sortable = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle on_destroy event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_destroy(self):
        pass

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, disk):
        # show attributes
        self.__metadata_view.clear()
        self.__metadata_view.add_row(('Type', disk.type))
        self.__metadata_view.add_row(('Name', disk.name))

        for name, value in disk.get_attributes().get_values():
            self.__metadata_view.add_row((pymobius.id_to_name(name), pymobius.to_string(value)))

        self.__metadata_view.set_report_name('Attributes of disk <%s>' % disk.name)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: data
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class DiskDataView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__mediator = pymobius.mediator.copy()
        self.__control = control
        self.name = 'Data'

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'data.png')
        self.icon_data = open(path, 'rb').read()

        # build widget
        self.__widget = mobius.core.ui.container()
        self.__widget.show()

        self.__data_widget = self.__mediator.call('ui.new-widget', 'hexview')
        self.__widget.set_content(self.__data_widget.get_ui_widget())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, disk):
        if disk.is_available():
            reader = disk.new_reader()
            self.__data_widget.set_reader(reader)
            self.__widget.show_content()
        else:
            self.__widget.set_content("Disk is not available")


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Dialog: AddDeviceDialog
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class AddDeviceDialog(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Init widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):

        # dialog
        self.__dialog = Gtk.Dialog(title='Select device to add', modal=True)
        self.__dialog.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        self.__dialog.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        self.__dialog.set_position(Gtk.WindowPosition.CENTER)
        self.__dialog.set_type_hint(Gdk.WindowTypeHint.DIALOG)
        self.__dialog.set_border_width(5)

        # scrolled window
        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        self.__dialog.vbox.pack_start(sw, True, True, 0)

        # listview
        model = Gtk.ListStore.new([str, str, str, object])

        self.__listview = Gtk.TreeView.new_with_model(model)
        self.__listview.set_enable_search(False)
        self.__listview.show()
        sw.add(self.__listview)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('device')
        tvcolumn.set_resizable(True)
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', 0)
        self.__listview.append_column(tvcolumn)

        renderer = Gtk.CellRendererText()
        tvcolumn = Gtk.TreeViewColumn('name')
        tvcolumn.set_resizable(True)
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', 1)
        self.__listview.append_column(tvcolumn)

        tvcolumn = Gtk.TreeViewColumn('serial number')
        tvcolumn.set_resizable(True)
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', 2)
        self.__listview.append_column(tvcolumn)

        # populate widget
        for path, name, serial, dev in self.__get_disks():
            model.append((path, name, serial, dev))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run dialog
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):

        # run dialog
        rc = self.__dialog.run()

        # get selected device, if any
        selection = self.__listview.get_selection()
        model, treeiter = selection.get_selected()

        if treeiter:
            device = model.get_value(treeiter, 3)
            device_uid = device.get_property('ID_SERIAL')

        else:
            device_uid = None

        self.__dialog.destroy()

        # if user pressed OK, return selected device
        if rc == Gtk.ResponseType.OK and device_uid:
            return device_uid

        return None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get disks
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_disks(self):
        device_list = []

        for dev in mobius.core.system.get_devices("block", "disk"):
            id_type = dev.get_property('ID_TYPE')

            if id_type == 'disk':
                path = dev.node
                id_vendor = dev.get_property('ID_VENDOR')
                id_model = dev.get_property('ID_MODEL')
                serial = dev.get_property('ID_SERIAL_SHORT')

                # name
                name_parts = []
                if id_vendor:
                    name_parts.append(id_vendor)

                if id_model:
                    name_parts.append(id_model)

                name = ' '.join(name_parts)

                # add to list
                device_list.append((path, name, serial, dev))

        return list(sorted(device_list))
