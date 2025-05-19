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
import cairo
import mobius
import pymobius
from gi.repository import GObject
from gi.repository import Gdk
from gi.repository import Gtk

from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
REPORT_ICON_NAME, REPORT_NAME, REPORT_ID, REPORT_STATE, REPORT_OBJ = range(5)
STATE_FOLDER, STATE_NOT_MODIFIED, STATE_MODIFIED = range(3)
DND_TARGET_OPTION, DND_TARGET_STMT = range(2)
DND_TARGETS = [Gtk.TargetEntry.new('application/x-mobius-report-opt', 0, DND_TARGET_OPTION),
               Gtk.TargetEntry.new('application/x-mobius-report-stmt', 0, DND_TARGET_STMT)]

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Statement colors constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
TEXT_COLOR = Gdk.Color(65535, 65535, 30000)
FOR_COLOR = Gdk.Color(20000, 65535, 65535)
IF_COLOR = Gdk.Color(60000, 55000, 30000)
WHILE_COLOR = Gdk.Color(20000, 65535, 65535)
CMD_COLOR = Gdk.Color(10000, 50000, 50000)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief StatementBlockWidget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class StatementBlockWidget(Gtk.VBox):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        GObject.GObject.__init__(self)
        self.set_spacing(10)
        self.set_border_width(10)
        self.connect('drag-motion', self.on_drag_motion)
        self.connect('drag-leave', self.on_drag_leave)
        self.connect('drag-data-received', self.on_drag_data_received)
        self.connect_after('draw', self.on_drag_highlight_draw)
        self.drag_dest_set(Gtk.DestDefaults.ALL, DND_TARGETS, Gdk.DragAction.COPY)
        self.add_placeholder()

        # internal vars
        self.empty = True
        self.highlight_y = None
        self.highlight_placeholder = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add placeholder 'do nothing'
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_placeholder(self):
        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.OUT)
        frame.show()
        self.pack_start(frame, False, True, 0)

        label = mobius.core.ui.label()
        label.set_markup('<i>do nothing</i>')
        label.set_visible(True)
        frame.add(label.get_ui_widget())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief insert child widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def insert_child(self, widget, pos=-1):

        # remove placeholder, if necessary
        if self.empty:
            for child in self.get_children():
                self.remove(child)

        # add widget
        self.pack_start(widget, False, True, 0)
        if not self.empty and pos != -1:
            self.reorder_child(widget, pos)

        # change emptiness
        self.empty = False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief remove child widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def remove_child(self, widget):
        self.remove(widget)

        # if it is empty, insert placeholder
        if not self.get_children():
            self.add_placeholder()
            self.empty = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: drag-motion
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_drag_motion(self, widget, context, x, y, timestamp, *args):
        widget.drag_unhighlight()

        # if widget is empty, highlight placeholder
        if widget.empty:
            child = widget.get_children()[0]
            rect = child.get_allocation()
            self.highlight_placeholder = rect
            self.highlight_y = None

        # else, find nearest intermediate position
        else:
            rect = widget.get_allocation()
            y = y + rect.y

            highlight_y = -1
            highlight_distance = 50000

            for child in widget.get_children():
                child_rect = child.get_allocation()

                d = abs(child_rect.y - y)
                if d < highlight_distance:
                    highlight_distance = d
                    highlight_y = child_rect.y - 5

                d = abs((child_rect.y + child_rect.height) - y)
                if d < highlight_distance:
                    highlight_distance = d
                    highlight_y = child_rect.y + child_rect.height + 5

            self.highlight_placeholder = None
            self.highlight_y = min(highlight_y, rect.y + rect.height)

        # redraw
        widget.queue_draw()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: drag-leave
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_drag_leave(self, widget, context, timestamp, *args):
        self.highlight_last_y = self.highlight_y
        self.highlight_y = None
        self.highlight_placeholder = None
        self.queue_draw()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: drag-data-received
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_drag_data_received(self, widget, context, x, y, selection, info, utime, *args):
        y = self.highlight_last_y
        self.highlight_y = None
        self.highlight_last_y = None
        self.highlight_placeholder = None
        self.drag_unhighlight()
        self.queue_draw()

        # evaluate drop position
        if self.empty:
            pos = -1

        else:
            pos = 0

            for child in self.get_children():
                rect = child.get_allocation()
                if rect.y < y:
                    pos += 1

        # if it is an option from pallette, emit 'drag-option-received'
        if info == DND_TARGET_OPTION:
            option = selection.get_data().decode('utf-8')
            self.emit('drag-option-received', pos, option)

        # if it is a widget, emit 'drag-stmt-received'
        elif info == DND_TARGET_STMT:
            rc = self.emit('drag-stmt-received', pos)
            if rc:
                context.finish(False, False, utime)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: after draw (drag_highlight)
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_drag_highlight_draw(self, widget, cr, *args):
        y = self.highlight_y
        placeholder = self.highlight_placeholder

        if y:
            rect = self.get_allocation()
            cr.set_source_rgba(0.0, 0.0, 1.0, 1.0)
            cr.set_line_width(1.0)
            cr.move_to(10, y - rect.y)
            cr.line_to(rect.width - 10, y - rect.y)
            cr.stroke()

        elif placeholder:
            cr.set_source_rgba(0.0, 0.0, 1.0, 1.0)
            cr.set_line_width(1.0)
            cr.rectangle(0, 0, placeholder.width, placeholder.height)
            cr.stroke()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief StatementBlockWidget signals
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
GObject.type_register(StatementBlockWidget)
GObject.signal_new('drag-option-received', StatementBlockWidget,
                   GObject.SignalFlags.RUN_LAST, GObject.TYPE_BOOLEAN, (int, str))
GObject.signal_new('drag-stmt-received', StatementBlockWidget,
                   GObject.SignalFlags.RUN_LAST, GObject.TYPE_BOOLEAN, (int,))


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Report Editor widget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ReportWidget(Gtk.VBox):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, mediator):
        GObject.GObject.__init__(self)
        self.set_spacing(5)
        self.__mediator = mediator.copy()

        self.report = None
        self.statement_icons = {}

        self.sw = Gtk.ScrolledWindow()
        self.sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        self.sw.show()
        self.pack_start(self.sw, True, True, 0)

        toolbar = Gtk.Toolbar()
        toolbar.set_style(Gtk.ToolbarStyle.ICONS)
        toolbar.show()
        self.pack_start(toolbar, False, False, 0)

        # trash bin
        toolitem = Gtk.ToolButton.new()
        toolitem.set_use_drag_window(True)
        toolitem.drag_dest_set(Gtk.DestDefaults.ALL,
                               [Gtk.TargetEntry.new('application/x-mobius-report-stmt', 0, DND_TARGET_STMT)],
                               Gdk.DragAction.COPY)
        toolitem.connect('drag-data-received', self.on_drop_trash_can)

        image = mobius.core.ui.new_icon_by_name('dnd-delete', mobius.core.ui.icon.size_dialog)
        image.show()

        toolitem.set_label_widget(image.get_ui_widget())
        toolitem.show()
        toolbar.insert(toolitem, -1)

        toolitem = Gtk.SeparatorToolItem()
        toolitem.show()
        toolbar.insert(toolitem, -1)

        # statement icons
        toolitem = self.build_toolitem('for')
        toolitem.show()
        toolbar.insert(toolitem, -1)

        toolitem = self.build_toolitem('while')
        toolitem.show()
        toolbar.insert(toolitem, -1)

        toolitem = self.build_toolitem('if')
        toolitem.show()
        toolbar.insert(toolitem, -1)

        toolitem = self.build_toolitem('assign')
        toolitem.show()
        toolbar.insert(toolitem, -1)

        toolitem = self.build_toolitem('call')
        toolitem.show()
        toolbar.insert(toolitem, -1)

        toolitem = self.build_toolitem('exec')
        toolitem.show()
        toolbar.insert(toolitem, -1)

        toolitem = self.build_toolitem('output')
        toolitem.show()
        toolbar.insert(toolitem, -1)

        toolitem = self.build_toolitem('verbatim')
        toolitem.show()
        toolbar.insert(toolitem, -1)

        toolitem = self.build_toolitem('text')
        toolitem.show()
        toolbar.insert(toolitem, -1)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get color for statement
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_statement_color(self, stmt):
        if stmt == 'for':
            return FOR_COLOR

        elif stmt == 'while':
            return WHILE_COLOR

        elif stmt == 'if':
            return IF_COLOR

        elif stmt in ('call', 'assign', 'exec'):
            return CMD_COLOR

        return TEXT_COLOR

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get statement pallette icon
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_statement_icon(self, stmt):

        # return icon if it is cached
        surface = self.statement_icons.get(stmt)
        if surface:
            return surface

        # draw icon
        width = 72
        height = 32

        color = self.get_statement_color(stmt)
        r = float(color.red) / 65535.0
        g = float(color.green) / 65535.0
        b = float(color.blue) / 65535.0

        surface = cairo.ImageSurface(cairo.Format.ARGB32, width, height)
        cr = cairo.Context(surface)

        # rectangle
        linear = cairo.LinearGradient(width // 2, 0, width // 2, height)
        linear.add_color_stop_rgb(0, r / 1.5, g / 1.5, b / 1.5)
        linear.add_color_stop_rgb(1, r, g, b)

        cr.set_source(linear)
        cr.set_line_width(1.0)
        cr.rectangle(0, 0, width, height)
        cr.fill()

        cr.set_source_rgb(0.0, 0.0, 0.0)
        cr.rectangle(0, 0, width, height)

        # statement text
        cr.select_font_face('sans', cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
        cr.set_font_size(12.0)
        extents = cr.text_extents(stmt)
        cr.move_to((width - extents[2]) // 2, (height - extents[3]) // 2 + 10)
        cr.show_text(stmt)

        cr.stroke()

        # return surface and terminate
        self.statement_icons[stmt] = surface
        return surface

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build draggable toolitem
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_toolitem(self, stmt):
        toolitem = Gtk.ToolButton.new()
        toolitem.set_use_drag_window(True)
        toolitem.drag_source_set(Gdk.ModifierType.BUTTON1_MASK,
                                 [Gtk.TargetEntry.new('application/x-mobius-report-opt', 0, 0)], Gdk.DragAction.COPY)
        toolitem.connect('drag-data-get', self.on_drag_data_get, stmt)

        # generate image for statement
        surface = self.get_statement_icon(stmt)
        image = Gtk.Image.new_from_surface(surface)
        image.show()

        # set toolitem
        toolitem.set_label_widget(image)

        pixbuf = Gdk.pixbuf_get_from_surface(surface, 0, 0, surface.get_width(), surface.get_height())
        toolitem.drag_source_set_icon_pixbuf(pixbuf)

        return toolitem

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief clear widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def clear(self):
        child = self.sw.get_child()
        if child:
            self.sw.remove(child)
        self.report = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_report(self, report):
        self.clear()
        self.report = report

        w = self.build_statement(report)
        self.sw.add(w)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build statement
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_statement(self, obj):
        statement = obj.statement.replace('-', '_')
        method = getattr(self, f'build_{statement}', None)

        if method:
            widget = method(obj)
            widget.obj = obj
            widget.show()

        else:
            widget = None

        return widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Build report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_report(self, obj):
        vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbox.set_spacing(5)
        vbox.set_border_width(10)
        vbox.set_visible(True)

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_border_width(5)
        hbox.set_spacing(3)
        hbox.set_visible(True)
        vbox.add_child(hbox, mobius.core.ui.box.fill_none)

        label = mobius.core.ui.label()
        label.set_markup('<b>report (</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        entry = Gtk.Entry()
        entry.set_text(obj.args)
        entry.connect('changed', self.on_entry_changed, obj, 'args')
        entry.show()
        hbox.add_child(entry, mobius.core.ui.box.fill_with_widget)

        label = mobius.core.ui.label()
        label.set_markup('<b>)</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        w = self.build_statement(obj.code)
        w.show()
        vbox.add_child(w, mobius.core.ui.box.fill_with_widget)

        return vbox.get_ui_widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build text
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_text(self, obj):
        eventbox = Gtk.EventBox()
        eventbox.modify_bg(Gtk.StateType.NORMAL, TEXT_COLOR)

        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.OUT)
        frame.show()
        eventbox.add(frame)

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_border_width(5)
        hbox.set_spacing(3)
        hbox.set_visible(True)
        frame.add(hbox.get_ui_widget())

        label = mobius.core.ui.label()
        label.set_markup('<b>text</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        frame = Gtk.Frame()
        frame.show()
        hbox.add_child(frame, mobius.core.ui.box.fill_with_widget)

        textbuffer = Gtk.TextBuffer()
        textbuffer.set_text(obj.text)
        textbuffer.connect('changed', self.on_textview_changed, obj)

        textview = Gtk.TextView.new_with_buffer(textbuffer)
        textview.set_wrap_mode(Gtk.WrapMode.WORD)
        textview.show()
        frame.add(textview)

        checkbutton = Gtk.CheckButton.new_with_label('newline')
        checkbutton.set_active(obj.newline)
        checkbutton.connect('toggled', self.on_checkbutton_toggled, obj, 'newline')
        checkbutton.show()
        hbox.add_child(checkbutton, mobius.core.ui.box.fill_none)

        return eventbox

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build verbatim
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_verbatim(self, obj):
        eventbox = Gtk.EventBox()
        eventbox.modify_bg(Gtk.StateType.NORMAL, TEXT_COLOR)

        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.OUT)
        frame.show()
        eventbox.add(frame)

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_border_width(5)
        hbox.set_spacing(3)
        hbox.set_visible(True)
        frame.add(hbox.get_ui_widget())

        label = mobius.core.ui.label()
        label.set_markup('<b>verbatim</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        frame = Gtk.Frame()
        frame.show()
        hbox.add_child(frame, mobius.core.ui.box.fill_with_widget)

        textbuffer = Gtk.TextBuffer()
        textbuffer.set_text(obj.text)
        textbuffer.connect('changed', self.on_textview_changed, obj)

        textview = Gtk.TextView.new_with_buffer(textbuffer)
        textview.set_wrap_mode(Gtk.WrapMode.WORD)
        textview.show()
        frame.add(textview)

        vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbox.set_spacing(5)
        vbox.set_visible(True)
        hbox.add_child(vbox, mobius.core.ui.box.fill_none)

        checkbutton = Gtk.CheckButton.new_with_label('newline')
        checkbutton.set_active(obj.newline)
        checkbutton.connect('toggled', self.on_checkbutton_toggled, obj, 'newline')
        checkbutton.show()
        vbox.add_child(checkbutton, mobius.core.ui.box.fill_none)

        # @begin-deprecated workaround for new option obj.use_exp_value
        if not hasattr(obj, 'use_exp_value'):
            obj.use_exp_value = True
        # @end-deprecated

        checkbutton = Gtk.CheckButton.new_with_label('${}')
        checkbutton.set_active(obj.use_exp_value)
        checkbutton.connect('toggled', self.on_checkbutton_toggled, obj, 'use_exp_value')
        checkbutton.show()
        vbox.add_child(checkbutton, mobius.core.ui.box.fill_none)

        return eventbox

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build output
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_output(self, obj):
        eventbox = Gtk.EventBox()
        eventbox.modify_bg(Gtk.StateType.NORMAL, TEXT_COLOR)

        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.OUT)
        frame.show()
        eventbox.add(frame)

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_border_width(5)
        hbox.set_spacing(3)
        hbox.set_visible(True)
        frame.add(hbox.get_ui_widget())

        label = mobius.core.ui.label()
        label.set_markup('<b>output</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        entry = Gtk.Entry()
        entry.set_text(obj.filename)
        entry.connect('changed', self.on_entry_changed, obj, 'filename')
        entry.show()
        hbox.add_child(entry, mobius.core.ui.box.fill_with_widget)

        return eventbox

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build assign
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_assign(self, obj):
        eventbox = Gtk.EventBox()
        eventbox.modify_bg(Gtk.StateType.NORMAL, CMD_COLOR)

        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.OUT)
        frame.show()
        eventbox.add(frame)

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_border_width(5)
        hbox.set_spacing(3)
        hbox.set_visible(True)
        frame.add(hbox.get_ui_widget())

        label = mobius.core.ui.label()
        label.set_markup('<b>assign</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        entry = Gtk.Entry()
        entry.set_text(obj.var)
        entry.connect('changed', self.on_entry_changed, obj, 'var')
        entry.show()
        hbox.add_child(entry, mobius.core.ui.box.fill_none)

        label = mobius.core.ui.label()
        label.set_markup('<b>value</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        entry = Gtk.Entry()
        entry.set_text(obj.value)
        entry.connect('changed', self.on_entry_changed, obj, 'value')
        entry.show()
        hbox.add_child(entry, mobius.core.ui.box.fill_with_widget)

        return eventbox

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build call
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_call(self, obj):
        eventbox = Gtk.EventBox()
        eventbox.modify_bg(Gtk.StateType.NORMAL, CMD_COLOR)

        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.OUT)
        frame.show()
        eventbox.add(frame)

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_border_width(5)
        hbox.set_spacing(3)
        hbox.set_visible(True)
        frame.add(hbox.get_ui_widget())

        label = mobius.core.ui.label()
        label.set_markup('<b>call</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        entry = Gtk.Entry()
        entry.set_text(obj.report)
        entry.connect('changed', self.on_entry_changed, obj, 'report')
        entry.show()
        hbox.add_child(entry, mobius.core.ui.box.fill_none)

        label = mobius.core.ui.label()
        label.set_markup('<b>(</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        entry = Gtk.Entry()
        entry.set_text(obj.args)
        entry.connect('changed', self.on_entry_changed, obj, 'args')
        entry.show()
        hbox.add_child(entry, mobius.core.ui.box.fill_with_widget)

        label = mobius.core.ui.label()
        label.set_markup('<b>)</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        return eventbox

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build exec
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_exec(self, obj):
        eventbox = Gtk.EventBox()
        eventbox.modify_bg(Gtk.StateType.NORMAL, CMD_COLOR)

        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.OUT)
        frame.show()
        eventbox.add(frame)

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_border_width(5)
        hbox.set_spacing(3)
        hbox.set_visible(True)
        frame.add(hbox.get_ui_widget())

        label = mobius.core.ui.label()
        label.set_markup('<b>exec</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        entry = Gtk.Entry()
        entry.set_text(obj.cmd)
        entry.connect('changed', self.on_entry_changed, obj, 'cmd')
        entry.show()
        hbox.add_child(entry, mobius.core.ui.box.fill_with_widget)

        return eventbox

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build if
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_if(self, obj):
        eventbox = Gtk.EventBox()
        eventbox.modify_bg(Gtk.StateType.NORMAL, IF_COLOR)

        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.OUT)
        frame.show()
        eventbox.add(frame)

        vbox = Gtk.VBox()
        vbox.set_border_width(5)
        vbox.set_spacing(10)
        vbox.obj = obj
        vbox.show()
        frame.add(vbox)

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(3)
        hbox.set_visible(True)
        vbox.pack_start(hbox.get_ui_widget(), False, True, 0)

        label = mobius.core.ui.label()
        label.set_markup('<b>if</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        entry = Gtk.Entry()
        entry.set_text(obj.condition)
        entry.connect('changed', self.on_entry_changed, obj, 'condition')
        entry.show()
        hbox.add_child(entry, mobius.core.ui.box.fill_with_widget)

        w = self.build_statement(obj.if_code)
        w.show()
        vbox.pack_start(w, True, True, 0)

        for item in obj.elifs:
            w, w_code = self.build_if_elif(item)

            w.show()
            vbox.pack_start(w, False, True, 0)

            w_code.show()
            vbox.pack_start(w_code, False, True, 0)

        # else code
        w = self.build_statement(obj.else_code)
        w.show()
        vbox.pack_end(w, True, True, 0)

        # else
        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(3)
        hbox.set_visible(True)
        vbox.pack_end(hbox.get_ui_widget(), False, True, 0)

        label = mobius.core.ui.label()
        label.set_markup('<b>else</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        hbox.add_filler()

        # 'add elif' button
        button = Gtk.Button()
        button.connect('clicked', self.on_add_elif, vbox, obj)
        button.show()
        hbox.add_child(button, mobius.core.ui.box.fill_none)

        button_hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        button_hbox.set_visible(True)
        button.add(button_hbox.get_ui_widget())

        image = Gtk.Image.new_from_icon_name('list-add', Gtk.IconSize.MENU)
        image.show()
        button_hbox.add_child(image, mobius.core.ui.box.fill_none)

        label = mobius.core.ui.label('elif')
        label.set_visible(True)
        button_hbox.add_child(label, mobius.core.ui.box.fill_none)

        return eventbox

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build elif
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_if_elif(self, obj):
        hbox = Gtk.HBox()
        hbox.set_spacing(3)

        label = mobius.core.ui.label()
        label.set_markup('<b>elif</b>')
        label.set_visible(True)
        hbox.pack_start(label.get_ui_widget(), False, True, 0)

        entry = Gtk.Entry()
        entry.set_text(obj.condition)
        entry.connect('changed', self.on_entry_changed, obj, 'condition')
        entry.show()
        hbox.pack_start(entry, True, True, 0)

        # 'remove elif' button
        button = Gtk.Button()
        button.show()
        hbox.pack_end(button, False, True, 0)

        button_hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        button_hbox.set_visible(True)
        button.add(button_hbox.get_ui_widget())

        image = Gtk.Image.new_from_icon_name('list-remove', Gtk.IconSize.MENU)
        image.show()
        button_hbox.add_child(image, mobius.core.ui.box.fill_none)

        label = mobius.core.ui.label('elif')
        label.set_visible(True)
        button_hbox.add_child(label, mobius.core.ui.box.fill_none)

        w = self.build_statement(obj.code)

        button.connect('clicked', self.on_remove_elif, hbox, w, obj)

        return hbox, w

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build for
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_for(self, obj):
        eventbox = Gtk.EventBox()
        eventbox.modify_bg(Gtk.StateType.NORMAL, FOR_COLOR)

        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.OUT)
        frame.show()
        eventbox.add(frame)

        vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbox.set_border_width(5)
        vbox.set_spacing(10)
        vbox.set_visible(True)
        frame.add(vbox.get_ui_widget())

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(3)
        hbox.set_visible(True)
        vbox.add_child(hbox, mobius.core.ui.box.fill_none)

        label = mobius.core.ui.label()
        label.set_markup('<b>for</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        entry = Gtk.Entry()
        entry.set_text(obj.vars)
        entry.connect('changed', self.on_entry_changed, obj, 'vars')
        entry.show()
        hbox.add_child(entry, mobius.core.ui.box.fill_none)

        label = mobius.core.ui.label()
        label.set_markup('<b>in</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        entry = Gtk.Entry()
        entry.set_text(obj.values)
        entry.connect('changed', self.on_entry_changed, obj, 'values')
        entry.show()
        hbox.add_child(entry, mobius.core.ui.box.fill_with_widget)

        w = self.build_statement(obj.code)
        w.show()
        vbox.add_child(w, mobius.core.ui.box.fill_with_widget)

        return eventbox

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build while
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_while(self, obj):
        eventbox = Gtk.EventBox()
        eventbox.modify_bg(Gtk.StateType.NORMAL, WHILE_COLOR)

        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.OUT)
        frame.show()
        eventbox.add(frame)

        vbox = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        vbox.set_border_width(5)
        vbox.set_spacing(10)
        vbox.set_visible(True)
        frame.add(vbox.get_ui_widget())

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(3)
        hbox.set_visible(True)
        vbox.add_child(hbox, mobius.core.ui.box.fill_none)

        label = mobius.core.ui.label()
        label.set_markup('<b>while</b>')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        entry = Gtk.Entry()
        entry.set_text(obj.condition)
        entry.connect('changed', self.on_entry_changed, obj, 'condition')
        entry.show()
        hbox.add_child(entry, mobius.core.ui.box.fill_with_widget)

        w = self.build_statement(obj.code)
        w.show()
        vbox.add_child(w, mobius.core.ui.box.fill_with_widget)

        return eventbox

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build block
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_block(self, obj):
        hbox = Gtk.HBox()

        label = mobius.core.ui.label('    ')
        label.set_visible(True)
        hbox.pack_start(label.get_ui_widget(), False, True, 0)

        vbox = StatementBlockWidget()
        vbox.set_border_width(10)
        vbox.obj = obj
        vbox.connect('drag-option-received', self.on_drag_option_received)
        vbox.connect('drag-stmt-received', self.on_drag_stmt_received)
        vbox.show()
        hbox.pack_start(vbox, True, True, 0)

        for stmt in obj.statements:
            w = self.build_statement(stmt)
            w.show()
            vbox.insert_child(w, -1)
            self.set_widget_draggable(w)

        return hbox

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set widget draggagle
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_widget_draggable(self, widget):
        widget.drag_source_set(Gdk.ModifierType.BUTTON1_MASK,
                               [Gtk.TargetEntry.new('application/x-mobius-report-stmt', 0, DND_TARGET_STMT)],
                               Gdk.DragAction.COPY)
        widget.connect('drag-begin', self.on_drag_begin)
        widget.connect('drag-data-get', self.on_drag_data_get)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_entry_changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_entry_changed(self, widget, obj, attrid, *args):
        setattr(obj, attrid, widget.get_text())
        self.emit('changed')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_textview_changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_textview_changed(self, textbuffer, obj, *args):
        start, end = textbuffer.get_bounds()
        obj.text = textbuffer.get_text(start, end, True)
        self.emit('changed')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_checkbutton_toggled
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_checkbutton_toggled(self, widget, obj, attrid, *args):
        setattr(obj, attrid, widget.get_active())
        self.emit('changed')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_drag_begin
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_drag_begin(self, widget, context, *args):
        surface = self.get_statement_icon(widget.obj.statement)
        pixbuf = Gdk.pixbuf_get_from_surface(surface, 0, 0, surface.get_width(), surface.get_height())
        widget.drag_source_set_icon_pixbuf(pixbuf)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_drag_option_received
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_drag_option_received(self, widget, pos, stmt_id):

        # insert new statement
        stmt = self.__mediator.call('report.new-statement', stmt_id)

        if pos == -1:
            widget.obj.statements.append(stmt)
        else:
            widget.obj.statements.insert(pos, stmt)

        # create widget and insert into block widget
        stmt_widget = self.build_statement(stmt)
        stmt_widget.show()
        widget.insert_child(stmt_widget, pos)
        self.set_widget_draggable(stmt_widget)

        # emit 'changed'
        self.emit('changed')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_drag_stmt_received
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_drag_stmt_received(self, dest_block_widget, pos):
        source_block_widget = self.dnd_widget.get_parent()
        stmt = self.dnd_widget.obj

        # check if dnd_widget is parent of dest_block_widget
        widget = dest_block_widget

        while widget.get_parent():
            if widget.get_parent() == self.dnd_widget:
                return True
            widget = widget.get_parent()

        # if source and dest widgets are the same, reorder DND widget
        if source_block_widget == dest_block_widget:
            for idx, child in enumerate(source_block_widget.obj.statements):
                if id(child) == id(stmt):
                    old_pos = idx

            if pos > old_pos:
                pos = pos - 1

            source_block_widget.obj.statements.pop(old_pos)
            source_block_widget.obj.statements.insert(pos, stmt)
            source_block_widget.reorder_child(self.dnd_widget, pos)

        # otherwise, remove from source and add to dest
        else:
            if pos == -1:
                dest_block_widget.obj.statements.append(stmt)
            else:
                dest_block_widget.obj.statements.insert(pos, stmt)

            for idx, child in enumerate(source_block_widget.obj.statements):
                if id(child) == id(stmt):
                    source_block_widget.obj.statements.pop(idx)

            source_block_widget.remove_child(self.dnd_widget)
            dest_block_widget.insert_child(self.dnd_widget, pos)

        # emit 'changed'
        self.emit('changed')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_drop_trash_can
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_drop_trash_can(self, widget, context, x, y, selection, info, utime, *args):

        # remove from block widget
        block_widget = self.dnd_widget.get_parent()
        block_widget.remove_child(self.dnd_widget)

        # remove from model
        for idx, child in enumerate(block_widget.obj.statements):
            if id(child) == id(self.dnd_widget.obj):
                block_widget.obj.statements.pop(idx)

        # emit 'changed'
        self.emit('changed')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_drag_data_get
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_drag_data_get(self, widget, context, selection, info, utime, *args):

        if info == DND_TARGET_OPTION:
            selection.set(selection.get_target(), 8, args[0].encode('utf-8'))

        elif info == DND_TARGET_STMT:
            selection.set(selection.get_target(), 8, b'')
            self.dnd_widget = widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_add_elif
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_add_elif(self, widget, vbox, obj, *args):

        # add elif to model
        stmt = self.__mediator.call('report.new-statement', 'elif')
        obj.elifs.append(stmt)

        # add elif to if widget
        w, w_code = self.build_if_elif(stmt)

        w.show()
        vbox.pack_start(w, False, True, 0)

        w_code.show()
        vbox.pack_start(w_code, False, True, 0)

        # emit 'changed'
        self.emit('changed')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_remove_elif
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_remove_elif(self, widget, hbox, elif_code, obj, *args):

        # remove elif from widget
        vbox = hbox.get_parent()

        for child in vbox.get_children():
            if child == hbox or child == elif_code:
                vbox.remove(child)

        # remove elif from model
        for idx, e in enumerate(vbox.obj.elifs):
            if id(e) == id(obj):
                vbox.obj.elifs.pop(idx)

        # emit 'changed'
        self.emit('changed')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief ReportWidget signals
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
GObject.type_register(ReportWidget)
GObject.signal_new('changed', ReportWidget,
                   GObject.SignalFlags.RUN_LAST, GObject.TYPE_BOOLEAN, ())


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Widget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Widget(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, mediator):
        self.__widget = mobius.core.ui.box(mobius.core.ui.box.orientation_vertical)
        self.__widget.set_border_width(5)
        self.__widget.set_spacing(10)
        self.__widget.set_visible(True)
        self.__mediator = mediator.copy()

        # menubar
        menubar = Gtk.MenuBar()
        menubar.show()
        self.__widget.add_child(menubar, mobius.core.ui.box.fill_none)

        item = Gtk.MenuItem.new_with_mnemonic('_File')
        item.show()
        menubar.append(item)

        menu = Gtk.Menu()
        menu.show()
        item.set_submenu(menu)

        self.new_menuitem = Gtk.MenuItem.new_with_mnemonic('_New')
        self.new_menuitem.connect("activate", self.on_report_new)
        self.new_menuitem.set_sensitive(False)
        self.new_menuitem.show()
        menu.append(self.new_menuitem)

        self.save_menuitem = Gtk.MenuItem.new_with_mnemonic('_Save')
        self.save_menuitem.connect("activate", self.on_report_save)
        self.save_menuitem.set_sensitive(False)
        self.save_menuitem.show()
        menu.append(self.save_menuitem)

        item = Gtk.MenuItem.new_with_mnemonic('_Close')
        item.connect("activate", self.on_extension_close)
        item.show()
        menu.append(item)

        item = Gtk.MenuItem.new_with_mnemonic('_Edit')
        item.show()
        menubar.append(item)

        menu = Gtk.Menu()
        menu.show()
        item.set_submenu(menu)

        self.new_folder_menuitem = Gtk.MenuItem.new_with_mnemonic('_New folder')
        self.new_folder_menuitem.connect("activate", self.on_report_new_folder)
        self.new_folder_menuitem.set_sensitive(False)
        self.new_folder_menuitem.show()
        menu.append(self.new_folder_menuitem)

        self.remove_menuitem = Gtk.MenuItem.new_with_mnemonic('_Remove')
        self.remove_menuitem.connect("activate", self.on_report_remove)
        self.remove_menuitem.set_sensitive(False)
        self.remove_menuitem.show()
        menu.append(self.remove_menuitem)

        # toolbar
        toolbar = Gtk.Toolbar()
        toolbar.set_style(Gtk.ToolbarStyle.ICONS)
        toolbar.show()
        self.__widget.add_child(toolbar, mobius.core.ui.box.fill_none)

        self.new_toolitem = Gtk.ToolButton.new()
        self.new_toolitem.set_icon_name('document-new')
        self.new_toolitem.set_sensitive(False)
        self.new_toolitem.connect("clicked", self.on_report_new)
        self.new_toolitem.show()
        self.new_toolitem.set_tooltip_text("Create report")
        toolbar.insert(self.new_toolitem, -1)

        self.save_toolitem = Gtk.ToolButton.new()
        self.save_toolitem.set_icon_name('document-save')
        self.save_toolitem.set_sensitive(False)
        self.save_toolitem.connect("clicked", self.on_report_save)
        self.save_toolitem.show()
        self.save_toolitem.set_tooltip_text("Save report")
        toolbar.insert(self.save_toolitem, -1)

        toolitem = Gtk.SeparatorToolItem()
        toolitem.show()
        toolbar.insert(toolitem, -1)

        self.new_folder_toolitem = Gtk.ToolButton.new()
        self.new_folder_toolitem.set_icon_name('folder-new')
        self.new_folder_toolitem.set_sensitive(False)
        self.new_folder_toolitem.connect("clicked", self.on_report_new_folder)
        self.new_folder_toolitem.show()
        self.new_folder_toolitem.set_tooltip_text("Create folder")
        toolbar.insert(self.new_folder_toolitem, -1)

        self.remove_toolitem = Gtk.ToolButton.new()
        self.remove_toolitem.set_icon_name('list-remove')
        self.remove_toolitem.set_sensitive(False)
        self.remove_toolitem.connect("clicked", self.on_report_remove)
        self.remove_toolitem.show()
        self.remove_toolitem.set_tooltip_text("Remove folder/report")
        toolbar.insert(self.remove_toolitem, -1)

        # hpaned
        hpaned = Gtk.HPaned()
        hpaned.show()
        self.__widget.add_child(hpaned, mobius.core.ui.box.fill_with_widget)

        # treeview
        frame = Gtk.Frame()
        frame.show()
        hpaned.pack1(frame, False, True)

        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        frame.add(sw)

        datastore = Gtk.TreeStore.new([str, str, str, int, object])
        datastore.set_sort_column_id(REPORT_NAME, Gtk.SortType.ASCENDING)

        self.treeview = Gtk.TreeView.new_with_model(datastore)
        self.treeview.set_headers_visible(False)
        self.populate_treeview()
        self.treeview.show()
        sw.add(self.treeview)

        tvcolumn = Gtk.TreeViewColumn()
        tvcolumn.set_sizing(Gtk.TreeViewColumnSizing.AUTOSIZE)
        self.treeview.append_column(tvcolumn)

        renderer = Gtk.CellRendererPixbuf()
        tvcolumn.pack_start(renderer, False)
        tvcolumn.add_attribute(renderer, 'icon-name', REPORT_ICON_NAME)

        renderer = Gtk.CellRendererText()
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', REPORT_NAME)

        selection = self.treeview.get_selection()
        selection.connect('changed', self.on_treeview_selection_changed)

        # editor
        self.report_widget = ReportWidget(self.__mediator)
        self.report_widget.connect('changed', self.on_report_changed)
        self.report_widget.show()
        hpaned.pack2(self.report_widget, True, True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_widget(self):
        return self.__widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief populate report treeview
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def populate_treeview(self):

        # insert root node
        datastore = self.treeview.get_model()
        root = datastore.append(None, ('folder', 'report', None, STATE_FOLDER, None))

        # insert folder and report nodes
        dir_iterators = {}

        for item_type, item_name in self.__mediator.call('report.iter'):
            pos = item_name.rfind('.')

            if pos != -1:
                basename = item_name[pos + 1:]
                dirname = item_name[:pos]
            else:
                dirname = ''
                basename = item_name

            # get parent iterator
            if dirname:
                iter = dir_iterators.get(dirname)
            else:
                iter = root

            # insert folder
            if item_type == 'FOLDER':
                child = datastore.append(iter, ('folder', basename, item_name, STATE_FOLDER, None))
                dir_iterators[item_name] = child

            # insert report
            else:
                datastore.append(iter, ('text-x-generic-template', basename, item_name, STATE_NOT_MODIFIED, None))

        # expand root node
        self.treeview.expand_row(datastore.get_path(root), False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief check if there are modified reports
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def is_modified(self):

        # transverse through treemodel
        def walk(model, treeiter=None):
            if treeiter:
                yield treeiter

            treeiter = model.iter_children(treeiter)
            while treeiter:
                for c in walk(model, treeiter):
                    yield c
                treeiter = model.iter_next(treeiter)

        # check if there is a modified report
        model = self.treeview.get_model()

        for treeiter in walk(model):
            state = model.get_value(treeiter, REPORT_STATE)

            if state == STATE_MODIFIED:
                return True

        return False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get new report/folder full id
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_new_item_full_id(self):
        selection = self.treeview.get_selection()
        model, iter = selection.get_selected()

        if not iter:
            return None

        # show dialog to enter item ID
        dialog = Gtk.Dialog(title='Enter new item ID', modal=True)
        dialog.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        dialog.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        dialog.set_position(Gtk.WindowPosition.CENTER)
        dialog.set_type_hint(Gdk.WindowTypeHint.DIALOG)
        dialog.set_border_width(10)

        hbox = mobius.core.ui.box(mobius.core.ui.box.orientation_horizontal)
        hbox.set_spacing(5)
        hbox.set_border_width(10)
        hbox.set_visible(True)
        dialog.vbox.pack_start(hbox.get_ui_widget(), True, True, 0)

        label = mobius.core.ui.label('ID')
        label.set_visible(True)
        hbox.add_child(label, mobius.core.ui.box.fill_none)

        entry = Gtk.Entry()
        entry.show()
        hbox.add_child(entry, mobius.core.ui.box.fill_with_widget)

        rc = dialog.run()
        item_id = entry.get_text()

        dialog.destroy()

        # if user pressed OK, evaluate full ID
        if rc == Gtk.ResponseType.OK and item_id:
            parent_id = model.get_value(iter, REPORT_ID)

            if parent_id:
                full_id = parent_id + '.' + item_id
            else:
                full_id = item_id

        else:
            full_id = None

        return full_id

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle widget stopped event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_widget_stopped(self):

        # if there are unsaved reports, show save/ignore/cancel dialog
        if self.is_modified():
            dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
            dialog.text = "Save changes before closing?"
            dialog.add_button(mobius.core.ui.message_dialog.button_yes)
            dialog.add_button(mobius.core.ui.message_dialog.button_no)
            dialog.add_button(mobius.core.ui.message_dialog.button_cancel)
            dialog.set_default_response(mobius.core.ui.message_dialog.button_cancel)
            rc = dialog.run()

            if rc == mobius.core.ui.message_dialog.button_cancel:
                return True

            elif rc == mobius.core.ui.message_dialog.button_yes:
                pass  # save

        # clean-up code
        self.__mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle close button
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_extension_close(self, widget, *args):
        self.__mediator.call('ui.working-area.close', self.working_area.id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_treeview_selection_changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_treeview_selection_changed(self, selection, *args):
        model, treeiter = selection.get_selected()

        if treeiter:
            state = model.get_value(treeiter, REPORT_STATE)

            if state == STATE_FOLDER:
                self.on_folder_selected(model, treeiter)
            else:
                self.on_report_selected(model, treeiter)

        else:
            self.on_nothing_selected()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_nothing_selected
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_nothing_selected(self):
        self.report_widget.clear()

        self.new_menuitem.set_sensitive(False)
        self.new_toolitem.set_sensitive(False)
        self.save_menuitem.set_sensitive(False)
        self.save_toolitem.set_sensitive(False)
        self.new_folder_menuitem.set_sensitive(False)
        self.new_folder_toolitem.set_sensitive(False)
        self.remove_menuitem.set_sensitive(False)
        self.remove_toolitem.set_sensitive(False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_folder_selected
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_folder_selected(self, model, treeiter):
        self.report_widget.clear()

        is_removable = (model.get_path(treeiter) != Gtk.TreePath.new_first())
        self.new_menuitem.set_sensitive(True)
        self.new_toolitem.set_sensitive(True)
        self.save_menuitem.set_sensitive(False)
        self.save_toolitem.set_sensitive(False)
        self.new_folder_menuitem.set_sensitive(True)
        self.new_folder_toolitem.set_sensitive(True)
        self.remove_menuitem.set_sensitive(is_removable)
        self.remove_toolitem.set_sensitive(is_removable)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_report_selected
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_report_selected(self, model, treeiter):

        # get report object
        report = model.get_value(treeiter, REPORT_OBJ)
        report_id = model.get_value(treeiter, REPORT_ID)
        state = model.get_value(treeiter, REPORT_STATE)

        if not report:
            report = self.__mediator.call('report.load', report_id)
            model.set_value(treeiter, REPORT_OBJ, report)

        # update window
        self.report_widget.set_report(report)

        is_modified = (state == STATE_MODIFIED)
        self.new_menuitem.set_sensitive(False)
        self.new_toolitem.set_sensitive(False)
        self.save_menuitem.set_sensitive(is_modified)
        self.save_toolitem.set_sensitive(is_modified)
        self.new_folder_menuitem.set_sensitive(False)
        self.new_folder_toolitem.set_sensitive(False)
        self.remove_menuitem.set_sensitive(True)
        self.remove_toolitem.set_sensitive(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_report_changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_report_changed(self, widget, *args):
        model, treeiter = self.treeview.get_selection().get_selected()

        if treeiter:
            model.set_value(treeiter, REPORT_STATE, STATE_MODIFIED)
            model.set_value(treeiter, REPORT_ICON_NAME, 'document-new')
            self.save_menuitem.set_sensitive(True)
            self.save_toolitem.set_sensitive(True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_report_save
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_report_save(self, widget, *args):
        model, treeiter = self.treeview.get_selection().get_selected()

        if treeiter:
            report = model.get_value(treeiter, REPORT_OBJ)
            report_id = model.get_value(treeiter, REPORT_ID)
            self.__mediator.call('report.save', report, report_id)
            model.set_value(treeiter, REPORT_STATE, STATE_NOT_MODIFIED)
            model.set_value(treeiter, REPORT_ICON_NAME, 'text-x-generic-template')

            self.save_menuitem.set_sensitive(False)
            self.save_toolitem.set_sensitive(False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_report_new
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_report_new(self, widget, *args):
        full_id = self.get_new_item_full_id()

        if full_id:
            item_id = full_id.rsplit('.', 1)[-1]
            report = self.__mediator.call('report.new')

            selection = self.treeview.get_selection()
            model, iter = selection.get_selected()
            child = model.append(iter, (Gtk.STOCK_EDIT, item_id, full_id, STATE_MODIFIED, report))
            self.treeview.expand_row(model.get_path(iter), False)
            selection.select_iter(child)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_report_new_folder
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_report_new_folder(self, widget, *args):
        full_id = self.get_new_item_full_id()

        if full_id:
            item_id = full_id.rsplit('.', 1)[-1]
            self.__mediator.call('report.new-folder', full_id)

            selection = self.treeview.get_selection()
            model, iter = selection.get_selected()
            child = model.append(iter, (Gtk.STOCK_DIRECTORY, item_id, full_id, STATE_FOLDER, None))
            self.treeview.expand_row(model.get_path(iter), False)
            selection.select_iter(child)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: on_report_remove
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_report_remove(self, widget, *args):
        selection = self.treeview.get_selection()
        model, iter = selection.get_selected()

        if not iter:
            return

        # show confirmation dialog
        item_id = model.get_value(iter, REPORT_ID)
        state = model.get_value(iter, REPORT_STATE)

        if state == STATE_FOLDER:
            item_type = 'folder'
        else:
            item_type = 'report'

        dialog = mobius.core.ui.message_dialog(mobius.core.ui.message_dialog.type_question)
        dialog.text = f"You are about to remove {item_type} '{item_id}'. Are you sure?"
        dialog.add_button(mobius.core.ui.message_dialog.button_yes)
        dialog.add_button(mobius.core.ui.message_dialog.button_no)
        dialog.set_default_response(mobius.core.ui.message_dialog.button_no)
        rc = dialog.run()

        if rc != mobius.core.ui.message_dialog.button_yes:
            return

        # remove report/folder
        if state == STATE_FOLDER:
            self.__mediator.call('report.remove-folder', item_id)
        else:
            self.__mediator.call('report.remove', item_id)

        # update window
        has_next = model.remove(iter)

        if has_next:
            selection.select_iter(iter)

        else:
            self.on_nothing_selected()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    icon = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)
    mobius.core.add_resource('menu.tools.' + EXTENSION_ID, 'Menu Tool: Report Wizard',
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
    working_area = pymobius.mediator.call('ui.working-area.get', EXTENSION_ID)

    if not working_area:
        widget = Widget(pymobius.mediator.copy())
        icon_path = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)

        working_area = pymobius.mediator.call('ui.working-area.new', EXTENSION_ID)
        working_area.set_default_size(800, 600)
        working_area.set_title(EXTENSION_NAME)
        working_area.set_icon(icon_path)
        working_area.set_widget(widget)

    working_area.show()
