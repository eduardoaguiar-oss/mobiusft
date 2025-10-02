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
import os
import os.path
import shutil
import sys
import traceback
import zipfile

import mobius
import pymobius


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Extension metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Metadata(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = ''
        self.name = ''
        self.author = ''
        self.version = ''
        self.description = ''

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get data from Python code
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def parse_from_code(self, data):
        context = {}
        bytecode = compile(data, f'metadata.py', 'exec')
        exec(bytecode, context)

        self.id = context.get('EXTENSION_ID')
        self.name = context.get('EXTENSION_NAME')
        self.author = context.get('EXTENSION_AUTHOR')
        self.version = context.get('EXTENSION_VERSION')
        self.description = context.get('EXTENSION_DESCRIPTION')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get extension path
# @param extension_id Extension ID
# @param *args sub-paths
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_path(extension_id, *args):
    app = mobius.core.application()
    extension_path = app.get_config_path('extensions')
    path = os.path.join(extension_path, extension_id, *args)

    return path


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get extension metadata
# @param extension_id Extension ID
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_metadata(extension_id):
    metadata = None
    path = get_path(extension_id, "metadata.py")

    if os.path.exists(path):
        fp = open(path)
        data = fp.read()
        fp.close()

        metadata = Metadata()
        metadata.parse_from_code(data)

    return metadata


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get metadata from extension file
# @param path Extension path
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_metadata_from_file(path):
    metadata = None

    if os.path.exists(path):
        zfile = zipfile.ZipFile(path)
        data = zfile.read('metadata.py').decode('utf-8')

        metadata = Metadata()
        metadata.parse_from_code(data)

        zfile.close()

    return metadata


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get icon data from extension file
# @param path Extension path
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_icon_data_from_file(path):
    icon_data = None

    if os.path.exists(path):
        zfile = zipfile.ZipFile(path)
        icon_data = zfile.read('icon.png')
        zfile.close()

    return icon_data


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Install extension
# @param path Extension path
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def install(path):

    # read metadata
    metadata = get_metadata_from_file(path)
    extension_id = metadata.id

    # set directory names
    extension_dir = get_path(extension_id)  # extension directory
    install_dir = extension_dir + '.install'  # installation directory
    old_dir = extension_dir + '.old'  # old version directory

    # remove previous .install and .old directory, if any
    if os.path.exists(install_dir):
        shutil.rmtree(install_dir)

    if os.path.exists(old_dir):
        shutil.rmtree(old_dir)

    # extract files to .install directory
    zfile = zipfile.ZipFile(path)
    zfile.extractall(install_dir)
    zfile.close()

    # run setup.py if it exists
    setup_py = os.path.join(install_dir, 'setup.py')

    if os.path.exists(setup_py):
        oldpwd = os.getcwd()
        os.chdir(install_dir)

        fp = open(setup_py)
        data = fp.read()
        fp.close()

        bytecode = compile(data, f'{extension_id}/setup.py', 'exec')
        exec(bytecode, {})

        os.chdir(oldpwd)

    # rename extension dir to .old
    if os.path.exists(extension_dir):
        shutil.move(extension_dir, old_dir)

    # rename .install directory to extension dir
    shutil.move(install_dir, extension_dir)

    # remove old version, if any
    if os.path.exists(old_dir):
        shutil.rmtree(old_dir)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Uninstall extension
# @param extension_id Extension ID
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def uninstall(extension_id):
    path = get_path(extension_id)
    shutil.rmtree(path)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief List installed extensions
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def installed():
    app = mobius.core.application()
    basedir = app.get_config_path("extensions")

    for filename in os.listdir(basedir):
        fullpath = os.path.join(basedir, filename)
        ext = os.path.splitext(fullpath)[1]

        if os.path.isdir(fullpath) and ext != '.install':
            yield filename


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Data
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
g_extensions = {}  # running extensions


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief service: extension.get-metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_get_metadata(extension_id):
    return get_metadata(extension_id)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief service: extension.get-metadata-from-file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_get_metadata_from_file(path):
    return get_metadata_from_file(path)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief service: extension.get-icon-data-from-file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_get_icon_data_from_file(path):
    return get_icon_data_from_file(path)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief service: extension.get-icon-path
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_get_icon_path(extension_id):
    return get_path(extension_id, 'icon.png')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief service: extension.get-resource-path
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_get_resource_path(extension_id, *args):
    return get_path(extension_id, 'resource', *args)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief service: extension.list
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_list():
    return installed()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief service: extension.is-running
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_is_running(extension_id):
    return extension_id in g_extensions


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Global variable container
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class GlobalData(object):
    pass


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service: extension.start-all
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_start_all():
    # start API
    for extension_id in installed():
        try:
            pymobius.mediator.call('extension.start-api', extension_id)
        except Exception as e:
            mobius.core.logf(
                f'ERR Error starting extension "{extension_id}" API: {str(e)}\n{traceback.format_exc()}')

    # start extensions
    for extension_id in g_extensions.keys():
        try:
            pymobius.mediator.call('extension.start', extension_id)
        except Exception as e:
            mobius.core.logf(
                f'ERR Error starting extension "{extension_id}": {str(e)}\n{traceback.format_exc()}')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service: extension.stop-all
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_stop_all():
    # stop extensions
    for extension_id in list(g_extensions.keys()):
        pymobius.mediator.call('extension.stop', extension_id)

    # stop extensions API
    for extension_id in list(g_extensions.keys()):
        pymobius.mediator.call('extension.stop-api', extension_id)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service: extension.start-api
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_start_api(extension_id):
    # instance data
    gdata = GlobalData()
    gdata.mediator = pymobius.mediator.copy()
    context = {'gdata': gdata}

    # execute code
    extension_dir = get_path(extension_id)
    sys.path.insert(0, extension_dir)

    try:
        pyfile = os.path.join(extension_dir, "main.py")

        fp = open(pyfile)
        sourcecode = fp.read()
        fp.close()

        bytecode = compile(sourcecode, f'{extension_id}/main.py', 'exec')
        exec(bytecode, context)

        start_api = context.get('pvt_start_api')
        if callable(start_api):
            start_api()

        g_extensions[extension_id] = context

    finally:
        del sys.path[0]

        for name, value in list(sys.modules.items()):
            path = getattr(value, '__file__', '') or ''

            if '/mobiusft/extensions/' in path:
                sys.modules.pop(name)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service: extension.stop-api
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_stop_api(extension_id):
    context = g_extensions.pop(extension_id)
    mediator = context.get('gdata').mediator
    mediator.clear()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service: extension.start
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_start(extension_id):
    context = g_extensions.get(extension_id)
    pvt_start = context.get('pvt_start')

    if pvt_start:
        pvt_start()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service: extension.stop
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_stop(extension_id):
    context = g_extensions.get(extension_id)
    pvt_stop = context.get('pvt_stop')

    if pvt_stop:
        pvt_stop()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service: extension.install
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_install(path):
    install(path)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service: extension.uninstall
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_extension_uninstall(extension_id):
    uninstall(extension_id)
