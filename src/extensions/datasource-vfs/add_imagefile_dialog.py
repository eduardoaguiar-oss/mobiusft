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
from gi.repository import Gtk


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Dialog: AddImageFileDialog
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class AddImageFileDialog(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Init widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):

        # dialog
        self.__filechooser = Gtk.FileChooserDialog(title='Select image file to add')
        self.__filechooser.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        self.__filechooser.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)

        fc_filter = Gtk.FileFilter()
        fc_filter.add_pattern('*.json')
        self.__filechooser.set_filter(fc_filter)

        # add supported image file types
        imagetypes = []
        for imagetype, description, extensions, is_writeable in mobius.core.vfs.get_imagefile_implementations():
            imagetypes.append((description, imagetype, extensions.split('|')))

        last_imagetype = mobius.framework.get_config('disk.last-imagetype')

        for description, imagetype, extensions in sorted(imagetypes):
            patterns = ['*.%s' % e for e in extensions if e != '*']

            filefilter = Gtk.FileFilter()
            filefilter.imagetype = imagetype
            filefilter.set_name('%s (%s)' % (description, ', '.join(patterns)))

            for pattern in patterns:
                filefilter.add_pattern(pattern)

            self.__filechooser.add_filter(filefilter)

            if last_imagetype == imagetype:
                self.__filechooser.set_filter(filefilter)

        filefilter = Gtk.FileFilter()
        filefilter.imagetype = None
        filefilter.set_name('All files (*)')
        filefilter.add_pattern('*')

        self.__filechooser.add_filter(filefilter)

        if not last_imagetype:
            self.__filechooser.set_filter(filefilter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run dialog
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        rc = self.__filechooser.run()
        url = self.__filechooser.get_uri()
        filefilter = self.__filechooser.get_filter()
        self.__filechooser.destroy()

        # save image file type
        if filefilter and filefilter.imagetype:
            imagetype = filefilter.imagetype
            transaction = mobius.framework.new_config_transaction()
            mobius.framework.set_config('disk.last-imagetype', imagetype)
            transaction.commit()

        if rc == Gtk.ResponseType.OK:
            return url

        return None
