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
import pymobius
from gi.repository import Gdk
from gi.repository import Gtk
from gi.repository import GObject
from gi.repository import Pango
import mobius

BYTES_PER_LINE = (8, 16, 24, 32, 48)
ENCODINGS = ('iso-8859-1', 'cp850', 'cp860', 'cp1252')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Customized textbuffer
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class TextBuffer(Gtk.TextBuffer):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        GObject.GObject.__init__(self)
        self.__stream = None
        self.__pos = -1
        self.__encoding = 'iso-8859-1'
        self.__width = 16
        self.__height = 16

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set encoding
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_encoding(self, encoding):
        if encoding != self.__encoding:
            self.__encoding = encoding
            self.__dump()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set stream
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_stream(self, stream):
        self.__stream = stream
        self.__pos = 0
        self.__dump()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief jump to previous line
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def previous_line(self):
        if self.__stream:
            self.seek(self.__pos - self.__width)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief jump to next line
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def next_line(self):
        if self.__stream:
            self.seek(self.__pos + self.__width)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief jump to previous page
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def previous_page(self):
        if self.__stream:
            self.seek(self.__pos - self.__width * self.__height)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief jump to next page
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def next_page(self):
        if self.__stream:
            self.seek(self.__pos + self.__width * self.__height)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get current position
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def tell(self):
        return self.__pos

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief jump to a given position
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def seek(self, pos):
        if pos < 0:
            pos = 0

        elif pos > self.max_position:
            pos = self.max_position

        if pos != self.__pos:
            self.__pos = pos
            self.__dump()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief update textbuffer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __dump(self):
        if not self.__stream:  # or not self.__stream.is_available ():
            self.set_text('')
            return

        self.__stream.seek(self.__pos, 0)
        data = self.__stream.read(self.__width * self.__height)
        data_length = len(data)

        text = ''
        for i in range(0, data_length, self.__width):
            text += '%08x' % (self.__pos + i)
            chartext = ''

            for j in range(self.__width):
                pos = i + j
                if pos < data_length:
                    c = data[pos]
                    text += ' %02x' % c
                    if c >= 32 and c != 127:
                        chartext += str(bytes([c]), self.__encoding, 'replace')
                    else:
                        chartext += '.'
                else:
                    text += '   '

            text += ' %s\n' % chartext

        self.set_text(text)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief width getter
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_width(self):
        return self.__width

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief width setter
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __set_width(self, width):
        self.__width = width
        self.__dump()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief height getter
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_height(self):
        return self.__height

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief height setter
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __set_height(self, height):
        if height != self.__height - 2:
            self.__height = height - 2
            self.__dump()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get max position
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_max_position(self):
        if self.__stream:
            position = self.__stream.size - (self.__height - 1) * self.__width
        else:
            position = 0
        return position

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief properties
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    width = property(__get_width, __set_width)
    height = property(__get_height, __set_height)
    max_position = property(__get_max_position, None)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Widget: hexview
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class HexViewWidget(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__control = None
        self.__mediator = pymobius.mediator.copy()
        self.__textview = None

        # widget
        self.__widget = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        self.__widget.set_border_width(5)
        self.__widget.set_spacing(10)
        self.__widget.show()

        # menubar
        self.__accel_group = Gtk.AccelGroup()
        self.__menubar = Gtk.MenuBar()
        self.__menubar.show()
        self.__widget.add_child(self.__menubar, mobius.core.ui.box.fill_none)

        item = Gtk.MenuItem.new_with_mnemonic('_View')
        item.show()
        self.__menubar.append(item)

        menu = Gtk.Menu()
        menu.set_accel_group(self.__accel_group)
        menu.show()
        item.set_submenu(menu)

        # "view->encoding" menu
        item = Gtk.MenuItem.new_with_mnemonic('_Encoding')
        item.show()
        menu.append(item)

        encoding_menu = Gtk.Menu()
        encoding_menu.show()
        item.set_submenu(encoding_menu)

        config_encoding = mobius.framework.get_config('gtk-ui-hexview.encoding') or 'cp850'
        group_menuitem = None

        for encoding in ENCODINGS:
            encoding_menuitem = Gtk.RadioMenuItem(group=group_menuitem, label=encoding)
            group_menuitem = group_menuitem or encoding_menuitem

            if encoding == config_encoding:
                encoding_menuitem.set_active(True)

            encoding_menuitem.connect('activate', self.__on_view_encoding_activate, encoding)
            encoding_menuitem.show()
            encoding_menu.append(encoding_menuitem)

        # "view->bytes per line" menu
        item = Gtk.MenuItem.new_with_mnemonic('_Bytes per line')
        item.show()
        menu.append(item)

        bytes_per_line_menu = Gtk.Menu()
        bytes_per_line_menu.show()
        item.set_submenu(bytes_per_line_menu)

        config_bytes_per_line = mobius.framework.get_config('gtk-ui-hexview.bytes_per_line') or 16
        group_menuitem = None

        for bytes_per_line in BYTES_PER_LINE:
            bytes_per_line_menuitem = Gtk.RadioMenuItem(group=group_menuitem, label=str(bytes_per_line))
            group_menuitem = group_menuitem or bytes_per_line_menuitem

            if config_bytes_per_line == bytes_per_line:
                bytes_per_line_menuitem.set_active(True)

            bytes_per_line_menuitem.connect('activate', self.__on_view_bytes_per_line_activate, bytes_per_line)
            bytes_per_line_menuitem.show()
            bytes_per_line_menu.append(bytes_per_line_menuitem)

        # "search" menubar entry
        item = Gtk.MenuItem.new_with_mnemonic('_Search')
        item.show()
        self.__menubar.append(item)

        menu = Gtk.Menu()
        menu.set_accel_group(self.__accel_group)
        menu.show()
        item.set_submenu(menu)

        self.__goto_menuitem = Gtk.MenuItem.new_with_mnemonic("Go to _Address")
        self.__goto_menuitem.set_sensitive(False)
        self.__goto_menuitem.connect("activate", self.__on_search_goto)
        self.__goto_menuitem.show()

        accel_key, accel_mods = Gtk.accelerator_parse('<control>j')
        self.__goto_menuitem.add_accelerator('activate', self.__accel_group, accel_key, accel_mods,
                                             Gtk.AccelFlags.VISIBLE)

        menu.append(self.__goto_menuitem)

        # textview
        self.__frame = Gtk.Frame()
        self.__frame.show()
        self.__widget.add_child(self.__frame, mobius.core.ui.box.fill_with_widget)

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_visible(True)
        self.__frame.add(hbox.get_ui_widget())

        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.NEVER)
        sw.show()
        hbox.add_child(sw, mobius.core.ui.box.fill_with_widget)

        self.__textview = Gtk.TextView.new_with_buffer(TextBuffer())
        self.__textview.set_editable(False)
        self.__textview.set_cursor_visible(False)
        self.__textview.set_wrap_mode(Gtk.WrapMode.NONE)
        self.__textview.set_left_margin(3)
        self.__textview.set_right_margin(3)
        self.__textview.connect('size-allocate', self.__on_textview_size_allocate)
        self.__textview.connect('key-press-event', self.__on_textview_key_press_event)

        font = Pango.FontDescription("Monospace 8")
        self.__textview.override_font(font)
        self.__textview.show()
        sw.add(self.__textview)

        context = self.__textview.get_pango_context()
        layout = Pango.Layout(context)
        layout.set_font_description(font)
        layout.set_text('X')
        self.__x_char_width, self.__x_char_height = layout.get_pixel_size()

        self.__vscrollbar = Gtk.VScrollbar()
        self.__vscrollbar.connect('change-value', self.__on_vscrollbar_change_value)
        hbox.add_child(self.__vscrollbar, mobius.core.ui.box.fill_none)

        # view data
        self.__textview_resized = False
        self.__reader = None

        textbuffer = self.__textview.get_buffer()
        textbuffer.set_encoding(config_encoding)
        textbuffer.width = config_bytes_per_line

        # set handlers
        # self.__on_selection_changed_handler = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set control object
    # @param control control object
    # @param wid widget id to be appended to the handler functions' names
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_control(self, control, wid=None):
        self.__control = control

        # event: on_selection_changed
        # self.__on_selection_changed_handler = get_handler ('selection_changed', control, wid)

        # if self.__on_selection_changed_handler:
        #  selection = self.__treeview.get_selection ()
        #  selection.connect ('changed', self.__on_selection_changed)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget

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
    # @brief Set reader
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_reader(self, reader):
        self.__reader = reader

        textbuffer = self.__textview.get_buffer()
        textbuffer.set_stream(self.__reader)

        self.__adjust_vscrollbar()
        self.__vscrollbar.set_value(0)
        self.__goto_menuitem.set_sensitive(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle textview size-allocate event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_textview_size_allocate(self, textview, rectangle):
        if self.__textview_resized:
            self.__textview_resized = False

        else:
            textbuffer = self.__textview.get_buffer()
            textbuffer.height = (rectangle.height // self.__x_char_height) + 1

            self.__adjust_vscrollbar()
            self.__textview_resized = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle textview key-press-event event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_textview_key_press_event(self, widget, event):

        if not self.__reader:
            return False

        textbuffer = self.__textview.get_buffer()

        if event.keyval == Gdk.KEY_Page_Down:
            textbuffer.next_page()

        elif event.keyval == Gdk.KEY_Page_Up:
            textbuffer.previous_page()

        elif event.keyval == Gdk.KEY_Down:
            textbuffer.next_line()

        elif event.keyval == Gdk.KEY_Up:
            textbuffer.previous_line()

        elif event.keyval == Gdk.KEY_Home:
            textbuffer.seek(0)

        elif event.keyval == Gdk.KEY_End:
            textbuffer.seek(textbuffer.max_position)

        else:
            return True

        self.__vscrollbar.set_value(textbuffer.tell())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle vscrollbar change-value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_vscrollbar_change_value(self, vscrollbar, scroll, value):
        self.__textbuffer_seek(int(value))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle search->jump to
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_search_goto(self, widget, *args):

        # build dialog
        dialog = Gtk.Dialog(title="Enter address expression", modal=True)
        dialog.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        dialog.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        dialog.set_default_response(Gtk.ResponseType.OK)
        dialog.set_border_width(10)
        dialog.vbox.set_spacing(10)

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(3)
        hbox.set_visible(True)
        dialog.vbox.pack_start(hbox.get_ui_widget(), True, True, 0)

        label = mobius.core.ui.label('Expression:')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_with_widget)

        dialog.entry = Gtk.Entry()
        dialog.entry.set_activates_default(True)
        dialog.entry.show()
        hbox.add_child(dialog.entry, mobius.core.ui.box.fill_with_widget)

        # run dialog
        rc = dialog.run()
        try:
            address = int(eval(dialog.entry.get_text()))
        except:
            rc = None
        dialog.destroy()

        # if response = OK, jump position
        if rc == Gtk.ResponseType.OK:
            self.__textbuffer_seek(address)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle view->encoding
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_view_encoding_activate(self, menuitem, encoding):
        if self.__textview:
            textbuffer = self.__textview.get_buffer()
            textbuffer.set_encoding(encoding)

            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('gtk-ui-hexview.encoding', encoding)
            transaction.commit()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle view->bytes_per_line
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_view_bytes_per_line_activate(self, menuitem, bytes_per_line):
        if self.__textview:
            textbuffer = self.__textview.get_buffer()
            textbuffer.width = bytes_per_line

            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('gtk-ui-hexview.bytes_per_line', bytes_per_line)
            transaction.commit()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Go to address
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __textbuffer_seek(self, pos):
        textbuffer = self.__textview.get_buffer()
        textbuffer.seek(pos)
        self.__vscrollbar.set_value(pos)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief adjust vscrollbar range
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __adjust_vscrollbar(self):
        textbuffer = self.__textview.get_buffer()
        pagesize = textbuffer.width * textbuffer.height

        if pagesize < textbuffer.max_position:
            self.__vscrollbar.set_range(0, textbuffer.max_position)
            self.__vscrollbar.set_increments(textbuffer.width, pagesize)
            self.__vscrollbar.show()
        else:
            self.__vscrollbar.hide()
