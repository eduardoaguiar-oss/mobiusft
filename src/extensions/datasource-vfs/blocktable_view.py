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
import mobius
import mobius.core.vfs
import pymobius
from gi.repository import GdkPixbuf
from gi.repository import Gtk

from copy_dialog import CopyDialog
from imagefile_chooser_dialog import ImagefileChooserDialog
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: BlockTableView
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class BlockTableView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, control):
        self.__mediator = pymobius.mediator.copy()
        self.__control = control
        self.__itemlist = []
        self.name = 'Blocks'

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'blocks.png')
        self.icon_data = open(path, 'rb').read()

        # vpaned
        self.__vpaned = Gtk.VPaned()
        self.__vpaned.show()
        self.__widget = self.__vpaned

        pos = mobius.framework.get_config('vfs-viewer.blocks-vpaned-position')

        if pos:
            self.__vpaned.set_position(pos)

        # tableview
        self.__tableview = self.__mediator.call('ui.new-widget', 'tableview')
        self.__tableview.set_report_id('vfs.blocks')
        self.__tableview.set_report_name('VFS Blocks')
        self.__tableview.set_report_app(f'{EXTENSION_NAME} v{EXTENSION_VERSION}')
        self.__tableview.set_control(self, 'main_tableview')
        self.__tableview.add_export_handler('dot', 'Graphviz .dot file', 'dot', self.__on_export_dot)

        column = self.__tableview.add_column('idx', '#', column_type='int')
        column.is_sortable = True

        column = self.__tableview.add_column('parents', 'Parents')

        column = self.__tableview.add_column('type', 'Type')
        column.is_sortable = True

        column = self.__tableview.add_column('status', 'Status')
        column.is_sortable = True

        column = self.__tableview.add_column('size', 'Size (bytes)', column_type='int')
        column.is_sortable = True

        column = self.__tableview.add_column('description', 'Description')
        column.is_sortable = True

        column = self.__tableview.add_column('obj', 'object', column_type='object')
        column.is_visible = False

        self.__tableview.show()
        self.__vpaned.pack1(self.__tableview.get_ui_widget(), False, True)

        # details container
        self.__details_container = mobius.ui.container()
        self.__details_container.show()
        self.__vpaned.pack2(self.__details_container.get_ui_widget(), True, True)

        # view selector
        self.__view_selector = self.__mediator.call('ui.new-widget', 'view-selector')
        self.__view_selector.set_control(self)
        self.__view_selector.show()
        self.__details_container.set_content(self.__view_selector.get_ui_widget())

        # views
        self.__attribute_view = BlockAttrView(self)
        self.__view_selector.add('attributes', self.__attribute_view)

        self.__data_view = BlockDataView(self)
        self.__view_selector.add('data', self.__data_view)

        for r in mobius.core.get_resources('vfs.block-view'):
            self.__view_selector.add(r.id, r.value(self))

        # toolbuttons
        toolbar = self.__view_selector.get_toolbar()

        toolitem = Gtk.SeparatorToolItem()
        toolitem.show()
        toolbar.insert(toolitem, -1)

        path = self.__mediator.call('extension.get-resource-path', EXTENSION_ID, 'export.png')
        icon = mobius.ui.new_icon_by_path(path, mobius.ui.icon.size_dnd)
        icon.set_visible(True)

        self.__export_toolitem = Gtk.ToolButton.new()
        self.__export_toolitem.set_icon_widget(icon.get_ui_widget())
        self.__export_toolitem.set_tooltip_text("Export block as...")
        self.__export_toolitem.show()
        self.__export_toolitem.connect('clicked', self.__on_export_block)
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
        mobius.framework.set_config('vfs-viewer.blocks-vpaned-position', position)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, vfs):
        self.__vfs = vfs
        self.__tableview.clear()

        for block in vfs.get_blocks():
            parents = ','.join(str(p.uid) for p in block.get_parents())
            description = block.get_attribute("description") or ''

            if block.is_available():
                if block.is_handled():
                    status = 'handled'
                else:
                    status = 'unknown'
            else:
                status = 'not available'

            self.__tableview.add_row((block.uid, parents, block.type, status, block.size, description, block))

        self.__tableview.select_row(0)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle main tableview selection-changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def main_tableview_on_selection_changed(self, selection_list):

        if selection_list:
            row_id, row_data = selection_list[0]
            self.__block = row_data[-1]
            self.__view_selector.set_data(self.__block)
            self.__details_container.show_content()
            self.__export_toolitem.set_sensitive(True)

        else:
            self.__block = None
            self.__details_container.set_message("Select block")
            self.__export_toolitem.set_sensitive(False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle export_block event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_block(self, widget, *args):

        # choose imagefile file name and type
        dialog = ImagefileChooserDialog('block')
        url, imagetype = dialog.run()

        if not imagetype:
            return

        # create reader
        reader = self.__block.new_reader()

        # create imagefile and writer
        imagefile = mobius.core.vfs.new_imagefile_by_url(url, imagetype)

        if imagetype == 'ewf':
            imagefile.segment_size = reader.size
            imagefile.compression_level = 1

        writer = imagefile.new_writer()

        # run CopyDialog
        dialog = CopyDialog(reader, writer, 'Copying block #%d...' % self.__block.uid)
        rc = dialog.run()

        # if copy was cancelled, remove file
        if rc == Gtk.ResponseType.CANCEL:
            f = mobius.io.file(url)
            f.remove()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Export .dot file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_export_dot(self, url, data):

        # create writer
        f = mobius.io.new_file_by_url(url)
        fp = mobius.io.text_writer(f.new_writer())

        # .dot header
        fp.write("digraph VFS\n")
        fp.write("{\n")
        fp.write("  rankdir=LR\n")
        fp.write("  node [shape=box]\n")

        # blocks
        for block in self.__vfs.get_blocks():
            description = block.get_attribute("description") or block.type
            fp.write('  B%d [label="%s"]\n' % (block.uid, description))

        # connections
        for block in self.__vfs.get_blocks():
            for child in block.get_children():
                fp.write('  B%d -> B%d\n' % (block.uid, child.uid))

        # .dot footer
        fp.write("}\n")


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: BlockAttrView
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class BlockAttrView(object):

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
        self.__metadata_view.set_report_id('vfs.block')
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
    def set_data(self, block):

        # show attributes
        self.__metadata_view.clear()
        self.__metadata_view.add_row(('#', block.uid))
        self.__metadata_view.add_row(('Type', block.type))
        self.__metadata_view.add_row(('Is available', block.is_available()))
        self.__metadata_view.add_row(('Is complete', block.is_complete()))
        self.__metadata_view.add_row(('Is handled', block.is_handled()))
        self.__metadata_view.add_row(('Parents', ','.join(str(p.uid) for p in block.get_parents())))
        self.__metadata_view.add_row(('Children', ','.join(str(c.uid) for c in block.get_children())))

        for name, value in block.get_attributes().get_values():
            if not isinstance(value, mobius.pod.map) and not isinstance(value, list):
                self.__metadata_view.add_row((pymobius.id_to_name(name), pymobius.to_string(value)))

        self.__metadata_view.set_report_name('Attributes of block <%s>' % '')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief View: data
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class BlockDataView(object):

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
        self.__widget = mobius.ui.container()
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
    def set_data(self, block):
        if block.is_available():
            reader = block.new_reader()
            self.__data_widget.set_reader(reader)
            self.__widget.show_content()
        else:
            self.__widget.set_message("Block is not available")
