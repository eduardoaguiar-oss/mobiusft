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
from gi.repository import Gtk


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Dialog: ImagefileChooserDialog
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ImagefileChooserDialog(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Init widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, name):

        # dialog
        self.__filechooser = Gtk.FileChooserDialog(title=f'Export {name} as', action=Gtk.FileChooserAction.SAVE)
        self.__filechooser.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL)
        self.__filechooser.add_button(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        self.__filechooser.set_do_overwrite_confirmation(True)
        self.__filechooser.set_current_name("imagefile")

        imagetypes = []
        for imagetype, description, extensions, is_writeable in mobius.vfs.get_imagefile_implementations():
            if is_writeable:
                imagetypes.append((description, imagetype, extensions.split('|')))

        for description, imagetype, extensions in sorted(imagetypes):
            patterns = ['*.%s' % e for e in extensions if e != '*']
            if '*' in extensions:
                patterns.append('*')

            filefilter = Gtk.FileFilter()
            filefilter.imagetype = imagetype
            filefilter.extension = extensions[0]
            filefilter.set_name('%s (%s)' % (description, ', '.join(patterns)))

            for pattern in patterns:
                filefilter.add_pattern(pattern)

            self.__filechooser.add_filter(filefilter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run dialog
    # @return URL, imagetype if file was selected
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):

        # run dialog
        rc = self.__filechooser.run()

        url = self.__filechooser.get_uri()
        filefilter = self.__filechooser.get_filter()
        imagetype = None

        self.__filechooser.destroy()

        # get selected filefilter
        if filefilter:
            imagetype = filefilter.imagetype
            extension = filefilter.extension

            if url.count('.') == 0:
                url = url + '.' + extension

        # return URL and imagetype, if any
        if rc == Gtk.ResponseType.OK:
            return url, imagetype

        return None, None
