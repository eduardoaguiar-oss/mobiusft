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
import pymobius
import pymobius.extension
import mobius
import glob
import os
import os.path
import shutil


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Application class
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Application(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run application
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):

        # extension data services
        pymobius.mediator.advertise('extension.get-metadata', pymobius.extension.svc_extension_get_metadata)
        pymobius.mediator.advertise('extension.get-metadata-from-file',
                                    pymobius.extension.svc_extension_get_metadata_from_file)
        pymobius.mediator.advertise('extension.get-icon-data-from-file',
                                    pymobius.extension.svc_extension_get_icon_data_from_file)
        pymobius.mediator.advertise('extension.get-icon-path', pymobius.extension.svc_extension_get_icon_path)
        pymobius.mediator.advertise('extension.get-resource-path', pymobius.extension.svc_extension_get_resource_path)

        # extension running services
        pymobius.mediator.advertise('extension.is-running', pymobius.extension.svc_extension_is_running)
        pymobius.mediator.advertise('extension.start-api', pymobius.extension.svc_extension_start_api)
        pymobius.mediator.advertise('extension.stop-api', pymobius.extension.svc_extension_stop_api)
        pymobius.mediator.advertise('extension.start', pymobius.extension.svc_extension_start)
        pymobius.mediator.advertise('extension.stop', pymobius.extension.svc_extension_stop)
        pymobius.mediator.advertise('extension.start-all', pymobius.extension.svc_extension_start_all)
        pymobius.mediator.advertise('extension.stop-all', pymobius.extension.svc_extension_stop_all)
        pymobius.mediator.advertise('extension.list', pymobius.extension.svc_extension_list)
        pymobius.mediator.advertise('extension.install', pymobius.extension.svc_extension_install)
        pymobius.mediator.advertise('extension.uninstall', pymobius.extension.svc_extension_uninstall)

        # configure user account
        self.__configure_user()

        app = mobius.core.application()
        app.start()

        # start extensions API
        pymobius.mediator.call('extension.start-all')

        # start application
        pymobius.mediator.emit('app.starting')
        pymobius.mediator.call('app.start')

        # stop extensions
        pymobius.mediator.emit('app.stopping')
        pymobius.mediator.call('extension.stop-all')

        app.stop()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Configure application for current user
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __configure_user(self):
        app = mobius.core.application()
        extensions_dir = app.get_config_path('extensions')

        if not os.path.exists(extensions_dir):
            os.makedirs(extensions_dir)
            dist_dir = app.get_data_path('extensions')

            # install Python extensions
            for path in sorted(glob.glob(f'{dist_dir}/*.mobius')):
                mobius.core.logf(f'INF Installing extension {os.path.basename(path)}')
                pymobius.extension.install(path)
