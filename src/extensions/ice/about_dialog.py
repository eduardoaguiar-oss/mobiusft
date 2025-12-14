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
import pymobius
from gi.repository import Gtk

from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief About Dialog
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class AboutDialog(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Build widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        app = mobius.core.application()
        self.__widget = Gtk.AboutDialog()

        self.__widget.set_program_name(app.name)
        self.__widget.set_version('v' + app.version)
        self.__widget.set_copyright(app.copyright)
        self.__widget.set_authors(['Eduardo Aguiar <aguiar@protonmail.ch>'])
        self.__widget.set_comments('Open-source forensic toolkit')
        self.__widget.set_website('https://www.nongnu.org/mobiusft')
        self.__widget.set_website_label('https://www.nongnu.org/mobiusft')

        # logo
        logo_path = pymobius.mediator.call('extension.get-resource-path', EXTENSION_ID, 'logo.svg')
        image = mobius.core.ui.new_icon_by_path(logo_path, mobius.core.ui.icon.size_extra_large)
        icon = image.get_ui_widget().get_pixbuf()
        self.__widget.set_logo(icon)

        # license
        license_path = pymobius.mediator.call('extension.get-resource-path', EXTENSION_ID, 'gpl.txt')
        license_text = open(license_path, 'r').read()
        self.__widget.set_license(license_text)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__widget.run()
        self.__widget.destroy()
