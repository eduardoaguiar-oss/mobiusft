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
import ctypes
import datetime
import fcntl
import os
import struct
import threading
import traceback

import cairo
import mobius
import pymobius
from gi.repository import GLib
from gi.repository import GObject
from gi.repository import Gdk
from gi.repository import Gtk
from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief enumerates
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SECTOR_UNDEFINED, SECTOR_READ, SECTOR_BAD = range(3)
DENSITY_SD, DENSITY_DD, DENSITY_HD, DENSITY_ED = range(4)
FLOPPY_FORMAT, FLOPPY_SIDES, FLOPPY_DENSITY, FLOPPY_TRANSFER_RATE, FLOPPY_TRACKS, FLOPPY_SECTORS_PER_TRACK, FLOPPY_TOTAL_SECTORS, FLOPPY_SIZE = range(
    8)
STATUS_SECTORS, STATUS_READ, STATUS_BAD, STATUS_RECOVERED = range(4)
LISTVIEW_ICON, LISTVIEW_NAME, LISTVIEW_VALUE = range(3)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief sector map constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
CELLSIZE = 10
XCELLS = 72
YCELLS = 40  # 72 * 40 = 2880 sectors (2HD floppy)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief floppy constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SECTOR_SIZE = 512
FDGETPRM = ${fdgetprm}
FDGETDRVPRM = ${fdgetdrvprm}
FDRESET = 0x254
FD_RESET_ALWAYS = 2


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Store floppy metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class FloppyMetadata(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.format = ''
        self.sides = -1
        self.density = ''
        self.transfer_rate = -1
        self.tracks = -1
        self.sectors_per_track = -1
        self.total_sectors = -1
        self.size = -1


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Floppy device
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Floppy(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__device = '/dev/fd0'
        self.__fd = None

        # get read function
        self.__libc = ctypes.CDLL('libc.so.6')
        self.__read_function = self.__libc.read

        # create an alignment buffer for direct IO
        LINUX_PAGE_ALIGNMENT = 4096
        self.__read_buffer = ctypes.create_string_buffer(LINUX_PAGE_ALIGNMENT + SECTOR_SIZE)
        self.__read_offset = LINUX_PAGE_ALIGNMENT - (ctypes.addressof(self.__read_buffer) & (LINUX_PAGE_ALIGNMENT - 1))
        self.__read_pointer = ctypes.addressof(self.__read_buffer) + self.__read_offset

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve floppy metadata
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def retrieve_metadata(self):

        # open device
        fd = os.open(self.__device, os.O_RDONLY)
        # fcntl.ioctl (fd, FDRESET, FD_RESET_ALWAYS)

        # read drive params
        drive_struct = struct.pack('128x')
        drive_struct = fcntl.ioctl(fd, FDGETDRVPRM, drive_struct)
        cmos_type = struct.unpack('b127x', drive_struct)[0]

        # read disk params
        floppy_struct = struct.pack('32x')
        floppy_struct = fcntl.ioctl(fd, FDGETPRM, floppy_struct)
        sectors, sect_per_track, sides, tracks, rate = struct.unpack('IIII5xB10x', floppy_struct)

        # close floppy
        os.close(fd)

        # fill floppy_metadata
        metadata = FloppyMetadata()
        metadata.sides = 2

        # evaluate density
        rate_flags = rate & 0x83

        if rate_flags == 0:
            metadata.density = 'HD'

        elif rate_flags in (1, 2, 0x80):
            metadata.density = 'DD'

        elif rate_flags == 3:
            metadata.density = 'ED'

        else:
            metadata.density = 'SD'

        # evaluate format
        if cmos_type > 2:
            metadata.format = '3.5'
        else:
            metadata.format = '5.25'

        # evaluate transfer_rate
        if rate & 0x80 == 0:
            rate = rate + 4
        else:
            rate = rate & 0x3
        metadata.transfer_rate = [250, 150, 125, 500, 500, 300, 250, 1000][rate]

        # other information
        metadata.tracks = tracks
        metadata.sectors_per_track = sect_per_track
        metadata.total_sectors = sectors
        metadata.size = sectors * SECTOR_SIZE

        return metadata

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve sector from floppy
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def retrieve_sector(self, sectnum):

        # open floppy device, if necessary
        if not self.__fd:
            self.__fd = os.open(self.__device, os.O_RDONLY | os.O_DIRECT)

        # read sector
        status = SECTOR_BAD
        data = '\0' * SECTOR_SIZE

        try:
            os.lseek(self.__fd, sectnum * SECTOR_SIZE, 0)
            ssize = self.__read_function(self.__fd, self.__read_pointer, SECTOR_SIZE)

            if ssize == SECTOR_SIZE:
                status = SECTOR_READ
                data = self.__read_buffer[self.__read_offset:self.__read_offset + SECTOR_SIZE]

        except EOFError as e:
            mobius.core.logf('WRN ' + str(e) + ' ' + traceback.format_exc())

        return status, data


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Store floppy image
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class FloppyImage(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, path):
        self.data = ''
        self.status = ''
        self.sectors = -1
        self.bad_sectors = -1
        self.path = path
        self.image_path = os.path.splitext(path)[0] + '.raw'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set image size in sectors
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_sectors(self, sectors):
        self.sectors = sectors
        self.bad_sectors = 0
        self.status = [SECTOR_UNDEFINED] * sectors
        self.data = '\0' * sectors * SECTOR_SIZE

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get sector status
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_status(self, i):
        return self.status[i]

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set sector status
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_status(self, i, status):
        self.status = self.status[:i] + status + self.status[i+1:]

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set sector data
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def write_sector(self, i, data):
        self.data = self.data[:i * SECTOR_SIZE] + data + self.data[(i + 1) * SECTOR_SIZE:]

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Open image
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def open(self):
        # read metadata file
        fp = open(self.path)
        lines = [l[:-1] for l in fp]
        fp.close()

        self.bad_sectors = int(lines[0])
        self.sectors = int(lines[1])

        map_val = {' ': SECTOR_UNDEFINED, '.': SECTOR_READ, '*': SECTOR_BAD}
        self.status = [map_val.get(c) for c in lines[2]]

        # read image file
        fp = open(self.image_path)
        self.data = fp.read()
        fp.close()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Save image
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def save(self):
        # save metadata file
        fp = open(self.path, 'w')
        fp.write('%d\n' % self.bad_sectors)
        fp.write('%d\n' % self.sectors)

        map_val = {SECTOR_UNDEFINED: ' ', SECTOR_READ: '.', SECTOR_BAD: '*'}
        status = ''.join([map_val[i] for i in self.status])
        fp.write('%s\n' % status)
        fp.close()

        # save image file
        fp = open(self.image_path, 'w')
        fp.write(self.data)
        fp.close()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief SectorMapWidget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SectorMapWidget(Gtk.DrawingArea):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize sector map
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        GObject.GObject.__init__(self)
        self.set_size_request(XCELLS * CELLSIZE + 1, YCELLS * CELLSIZE + 1)
        self.connect('draw', self.__on_draw)

        self.status = None
        self.xcells = XCELLS
        self.ycells = YCELLS
        self.__valid_gradient = False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief event: draw
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_draw(self, widget, cr, *args):

        # evaluate gradients, if necessary
        if not self.__valid_gradient:
            width = self.xcells * CELLSIZE
            height = self.ycells * CELLSIZE

            self.__grad_bad = cairo.LinearGradient(0, 0, width, height)
            self.__grad_bad.add_color_stop_rgb(0, 1.0, 0.0, 0.0)
            self.__grad_bad.add_color_stop_rgb(width, 0.4, 0.0, 0.0)

            self.__grad_read = cairo.LinearGradient(0, 0, width, height)
            self.__grad_read.add_color_stop_rgb(0, 0.0, 0.0, 1.0)
            self.__grad_read.add_color_stop_rgb(width, 0.0, 0.0, 0.4)

            self.__grad_undefined = cairo.LinearGradient(0, 0, width, height)
            self.__grad_undefined.add_color_stop_rgb(0, 0.8, 0.8, 0.8)
            self.__grad_undefined.add_color_stop_rgb(width, 0.4, 0.4, 0.4)

            self.__valid_gradient = True

        # evaluate sector area
        rect = widget.get_allocation()
        ix = 0  # rect.x // CELLSIZE
        iy = 0  # rect.y // CELLSIZE
        fx = (rect.width + rect.x) // CELLSIZE + 1  # (event.area.x + event.area.width) // CELLSIZE + 1
        fy = (rect.height + rect.y) // CELLSIZE + 1  # (event.area.y + event.area.height) // CELLSIZE + 1

        # draw
        self.__draw_area(cr, ix, iy, fx, fy)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Clear sector map
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def clear(self):
        self.status = None
        self.xcells = XCELLS
        self.ycells = YCELLS
        self.set_size_request(XCELLS * CELLSIZE + 1, YCELLS * CELLSIZE + 1)
        self.queue_draw()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set status of map
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_status(self, status):
        self.status = status

        # request new widget size
        old_xcells = self.xcells
        old_ycells = self.ycells

        self.xcells = XCELLS
        self.ycells = len(status) // XCELLS

        # resize area, if necessary
        if self.xcells != old_xcells or self.ycells != old_ycells:
            rect = self.get_allocation()
            gc = self.get_style().bg_gc[Gtk.StateType.NORMAL]
            # self.window.draw_rectangle (gc, True, rect.x, rect.y, rect.width, rect.height)

            self.set_size_request(self.xcells * CELLSIZE, self.ycells * CELLSIZE)
            self.__valid_gradient = False

        # request drawing
        self.queue_draw()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Set sector status
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_sector(self, sector, status):
        self.status = self.status[:sector] + [status] + self.status[sector + 1:]

        cr = self.window.cairo_create()
        self.__draw_sector(sector, status, cr)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Draw area
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __draw_area(self, cr, ix, iy, fx, fy):

        # clip to drawing_area
        rect = self.get_allocation()
        fx = min(fx, self.xcells - 1)
        fy = min(fy, self.ycells - 1)

        # fill rectangle
        # surface = cairo.ImageSurface (cairo.Format.ARGB32, rect.width, rect.height)
        # cr = cairo.Context (surface)

        cr.set_line_width(1.0)
        cr.set_source_rgb(0.0, 0.0, 0.0)
        cr.rectangle(ix * CELLSIZE, iy * CELLSIZE, (fx + 1) * CELLSIZE + 1, (fy + 1) * CELLSIZE + 1)
        cr.fill()

        # draw sectors
        for y in range(iy, fy + 1):
            for x in range(ix, fx + 1):
                sector = y * self.xcells + x

                if self.status:
                    status = self.status[sector]

                else:
                    status = SECTOR_UNDEFINED

                self.__draw_sector(sector, status, cr)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Draw sector
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __draw_sector(self, sector, status, cr):
        x = sector % XCELLS
        y = sector // XCELLS

        if status == SECTOR_UNDEFINED:
            cr.set_source(self.__grad_undefined)

        elif status == SECTOR_READ:
            cr.set_source(self.__grad_read)

        elif status == SECTOR_BAD:
            cr.set_source(self.__grad_bad)

        cr.rectangle(x * CELLSIZE + 1, y * CELLSIZE + 1, CELLSIZE - 1, CELLSIZE - 1)
        cr.fill()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief view: floppy imager
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class FloppyImagerView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.name = 'floppy imager mode'

        icon_path = self.__mediator.call('extension.get-icon-path', EXTENSION_ID)
        self.icon_data = open(icon_path, 'rb').read()

        # build widget
        self.__widget = Widget()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set data to be viewed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_data(self, itemlist):

        if len(itemlist) == 1:
            self.__widget.set_item(itemlist[0])

        else:
            self.__widget.reset_item()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Widget
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Widget(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__widget = mobius.ui.box(mobius.ui.box.orientation_vertical)
        self.__widget.set_border_width(10)
        self.__widget.set_spacing(10)
        self.__widget.set_visible(True)
        self.mediator = pymobius.mediator.copy()

        # sector map
        hbox = mobius.ui.box(mobius.ui.box.orientation_horizontal)
        hbox.set_visible(True)
        self.__widget.add_child(hbox, mobius.ui.box.fill_with_widget)

        hbox.add_filler()

        vbox = mobius.ui.box(mobius.ui.box.orientation_vertical)
        vbox.set_visible(True)
        hbox.add_child(vbox, mobius.ui.box.fill_none)

        hbox.add_filler()

        label = mobius.ui.label()
        label.set_markup('<b>Sector map</b>')
        label.set_halign(mobius.ui.label.align_left)
        label.set_valign(mobius.ui.label.align_top)
        label.set_visible(True)
        vbox.add_child(label, mobius.ui.box.fill_none)

        self.sector_map = SectorMapWidget()
        self.sector_map.set_size_request(721, 401)
        self.sector_map.set_sensitive(False)
        self.sector_map.show()
        vbox.add_child(self.sector_map, mobius.ui.box.fill_none)

        # map legend
        hbox = mobius.ui.box(mobius.ui.box.orientation_horizontal)
        hbox.set_spacing(30)
        hbox.set_visible(True)
        vbox.add_child(hbox, mobius.ui.box.fill_none)

        hbox_read = mobius.ui.box(mobius.ui.box.orientation_horizontal)
        hbox_read.set_spacing(3)
        hbox_read.set_visible(True)
        hbox.add_child(hbox_read, mobius.ui.box.fill_none)

        image = self.get_image_from_color(0, 0, .82)
        image.show()
        hbox_read.add_child(image, mobius.ui.box.fill_none)

        label = mobius.ui.label('Read')
        label.set_visible(True)
        hbox_read.add_child(label, mobius.ui.box.fill_with_widget)

        hbox_bad = mobius.ui.box(mobius.ui.box.orientation_horizontal)
        hbox_bad.set_spacing(3)
        hbox_bad.set_visible(True)
        hbox.add_child(hbox_bad, mobius.ui.box.fill_none)

        image = self.get_image_from_color(.82, 0, 0)
        image.show()
        hbox_bad.add_child(image, mobius.ui.box.fill_none)

        label = mobius.ui.label('Bad')
        label.set_visible(True)
        hbox_bad.add_child(label, mobius.ui.box.fill_with_widget)

        hbox_undef = mobius.ui.box(mobius.ui.box.orientation_horizontal)
        hbox_undef.set_spacing(3)
        hbox_undef.set_visible(True)
        hbox.add_child(hbox_undef, mobius.ui.box.fill_none)

        image = self.get_image_from_color(.6, .6, .6)
        image.show()
        hbox_undef.add_child(image, mobius.ui.box.fill_none)

        label = mobius.ui.label('Undefined')
        label.set_visible(True)
        hbox_undef.add_child(label, mobius.ui.box.fill_with_widget)

        # buttons
        hbox = mobius.ui.box(mobius.ui.box.orientation_horizontal)
        hbox.set_visible(True)
        self.__widget.add_child(hbox, mobius.ui.box.fill_none)

        button = mobius.ui.button()
        button.set_icon_by_name('window-close')
        button.set_text('_Close')
        button.set_visible(True)
        button.set_callback('clicked', self.__on_extension_close)
        hbox.add_child(button, mobius.ui.box.fill_none)

        hbox.add_filler()

        self.__retrieve_button = mobius.ui.button()
        self.__retrieve_button.set_icon_by_name('document-open')
        self.__retrieve_button.set_text('_Retrieve')
        self.__retrieve_button.set_sensitive(False)
        self.__retrieve_button.set_visible(True)
        self.__retrieve_button.set_callback('clicked', self.__on_retrieve_data)
        hbox.add_child(self.__retrieve_button, mobius.ui.box.fill_none)

        # status bar
        frame = Gtk.Frame()
        frame.set_shadow_type(Gtk.ShadowType.IN)
        frame.show()
        self.__widget.add_child(frame, mobius.ui.box.fill_none)

        self.__status_label = mobius.ui.label()
        self.__status_label.set_halign(mobius.ui.label.align_left)
        self.__status_label.set_visible(True)
        frame.add(self.__status_label.get_ui_widget())

        # specific data
        self.item = None
        self.image = None
        self.running = False

        # connect to events
        self.mediator.connect('case.selection-changed', self.on_case_selection_changed)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_widget(self):
        return self.__widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate image for a given color
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_image_from_color(self, r, g, b):
        surface = cairo.ImageSurface(cairo.Format.ARGB32, 16, 16)
        cr = cairo.Context(surface)

        cr.set_line_width(1.0)
        cr.set_source_rgba(r, g, b, 1.0)
        cr.rectangle(0, 0, 16, 16)
        cr.fill()

        cr.set_source_rgba(0.0, 0.0, 0.0, 1.0)
        cr.rectangle(0, 0, 16, 16)
        cr.stroke()

        pixbuf = Gdk.pixbuf_get_from_surface(surface, 0, 0, surface.get_width(), surface.get_height())
        image = Gtk.Image.new_from_pixbuf(pixbuf)
        return image

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set child widget sensitive
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_child_sensitive(self):
        browse_enabled = self.item is not None
        setup_enabled = self.item is not None and not self.running
        retrieve_enabled = self.image is not None and not self.running and (
                    self.image.bad_sectors == -1 or self.image.bad_sectors > 0)

        self.sector_map.set_sensitive(browse_enabled)
        self.__retrieve_button.set_sensitive(retrieve_enabled)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set current item
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_item(self, item):
        case = item.case

        image_path = case.create_path(os.path.join('image', f'item-{item.uid:05d}.metadata'))
        if not image_path:
            return

        # load image
        self.image_path = image_path
        self.image = FloppyImage(self.image_path)

        if os.path.exists(self.image_path):
            self.image.open()
            self.sector_map.set_status(self.image.status)
        else:
            self.sector_map.clear()

        # update window
        self.set_child_sensitive()
        self.__status_label.set_text('Hit retrieve button to start')
        self.item = item

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief reset current item
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def reset_item(self):
        self.item = None
        self.image = None
        self.running = False

        self.sector_map.clear()
        self.__status_label.set_text('Select floppy item to start...')

        self.set_child_sensitive()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle close button
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_extension_close(self):
        self.mediator.call('ui.working-area.close', self.working_area.id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle widget stopped event
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_widget_stopped(self):
        self.mediator.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle case selection changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_case_selection_changed(self, case, itemlist):
        if self.running:
            return

        if len(itemlist) == 1 and itemlist[0].category == 'floppy':
            item = itemlist[0]
            if not self.item or self.item.uid != item.uid:
                self.set_item(item)
        else:
            if self.item:
                self.reset_item()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Start data retrieval
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_retrieve_data(self):
        self.running = True
        self.set_child_sensitive()

        # retrieve disk metadata
        floppy = Floppy()

        try:
            metadata = floppy.retrieve_metadata()
            self.item.format = metadata.format
            self.item.sides = str(metadata.sides)
            self.item.density = metadata.density
            self.item.real_capacity = str(metadata.size)
            self.item.tracks = str(metadata.tracks)
            self.item.sectors_per_track = str(metadata.sectors_per_track)
            self.item.sectors = str(metadata.total_sectors)
            self.item.transfer_rate = '%d kb/s' % metadata.transfer_rate

        except Exception as e:
            GLib.idle_add(self.__status_label.set_markup, f'<b>Error:</b>{e}')
            self.running = False
            return

        # start retrieve thread
        threading.Thread(target=self.on_thread_retrieve_data, args=(floppy, metadata.total_sectors))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief data retrieval thread
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_thread_retrieve_data(self, floppy, sectors):
        guard = mobius.core.thread_guard()

        # set data
        if self.image.data == '':
            self.image.set_sectors(sectors)
            self.sector_map.set_status(self.image.status)

        bad_sectors = 0

        # retrieve image
        try:
            for i in range(sectors):

                if self.image.get_status(i) != SECTOR_READ:
                    GLib.idle_add(self.__status_label.set_text, f'Retrieving sector {i:d}')
                    status, data = floppy.retrieve_sector(i)

                    if status != self.image.get_status(i):
                        GLib.idle_add(self.sector_map.set_sector, i, status)
                        self.image.set_status(i, status)
                        self.image.write_sector(i, data)

                    if status == SECTOR_BAD:
                        bad_sectors = bad_sectors + 1

        except Exception as e:
            GLib.idle_add(self.__status_label.set_markup, f'<b>Error:</b>{e}')
            self.running = False
            return

        # calculate recovered sectors
        if self.image.bad_sectors > bad_sectors:
            recovered_sectors = self.image.bad_sectors - bad_sectors
        else:
            recovered_sectors = 0

        # update image
        self.image.bad_sectors = bad_sectors
        self.image.save()

        # create disk
        disk = mobius.vfs.new_disk_by_path(self.image.image_path)

        uname_data = os.uname()
        acquisition_platform = f'{uname_data[0]} {uname_data[2]} on {uname_data[4]}'
        acquisition_tool = f'{EXTENSION_NAME} v{EXTENSION_VERSION}'
        acquisition_user = os.getlogin()
        acquisition_time = datetime.datetime.now().isoformat()

        disk.set_attribute('acquisition_tool', acquisition_tool)
        disk.set_attribute('acquisition_platform', acquisition_platform)
        disk.set_attribute('acquisition_user', acquisition_user)
        disk.set_attribute('acquisition_time', acquisition_time)

        # set item datasource
        vfs = mobius.vfs.vfs()
        vfs.add_disk(disk)
        datasource = mobius.datasource.new_datasource_from_vfs(vfs)

        transaction = self.item.new_transaction()
        self.item.set_datasource(datasource)
        transaction.commit()

        # update window
        self.running = False

        text = f'Image file retrieved. Sectors: {self.image.sectors:d}. Bad: {self.image.bad_sectors:d}'
        if recovered_sectors > 0:
            text += f'. Recovered: {recovered_sectors:d}'

        GLib.idle_add(self.__status_label.set_text, text)
        GLib.idle_add(self.set_child_sensitive)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    icon = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)
    mobius.core.add_resource('menu.tools.' + EXTENSION_ID, 'Menu Tool: Floppy Imager',
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
        widget = Widget()
        icon_path = pymobius.mediator.call('extension.get-icon-path', EXTENSION_ID)

        working_area = pymobius.mediator.call('ui.working-area.new', EXTENSION_ID)
        working_area.set_default_size(400, 500)
        working_area.set_title(EXTENSION_NAME)
        working_area.set_icon(icon_path)
        working_area.set_widget(widget)

    working_area.show()
