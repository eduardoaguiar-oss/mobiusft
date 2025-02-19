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
import mobius
import libxml2

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Extension metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
EXTENSION_ID = 'lshw'
EXTENSION_NAME = 'Lshw-agent'
EXTENSION_AUTHOR = 'Eduardo Aguiar'
EXTENSION_VERSION = '1.12'
EXTENSION_DESCRIPTION = 'Add DND for lshw -xml command output files'

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
FREQUENCY_UNITS = ['Hz', 'KHz', 'MHz', 'GHz', 'THz']
TRANSFER_UNITS = ['b/s', 'Kb/s', 'Mb/s', 'Gb/s', 'Tb/s']


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief generic data holder class
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class data_holder(object):

    def __getattr__(self, name):
        return self.__dict__.get(name)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief format number
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def format_number(value, UNITS):
    if value is None:
        return None

    idx = 0

    while value.endswith('000'):
        idx += 1
        value = value[:-3]

    return value + ' ' + UNITS[idx]


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Decoder for lshw -xml command output file
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class lshw_decoder(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # @param uri lshw output file uri
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, uri):

        # initialize attributes
        self.motherboard = None
        self.computer = None
        self.processors = []
        self.disks = []
        self.optical_drives = []
        self.gpus = []
        self.network_adapters = []
        self.sound_adapters = []

        # parse XML
        doc = libxml2.parseFile(uri)
        root = doc.getRootElement()
        node_list = self.__load_list(root)

        # retrieve data from XML nodes
        self.__retrieve_info(node_list[0])

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief retrieve info from node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_info(self, node):
        if node.classname == 'system':
            self.computer = node

            if node.settings.get('chassis') == 'desktop':
                self.computer.type = 'computer'
            else:
                self.computer.type = 'notebook'

        elif node.classname == 'bus' and node.id == 'core' and node.product:
            self.motherboard = node

        elif node.classname == 'processor' and node.id.startswith('cpu') and node.product:
            node.frequency = format_number(node.capacity, FREQUENCY_UNITS)
            self.processors.append(node)

        elif node.classname == 'memory' and node.id == 'firmware' and self.motherboard:
            self.motherboard.firmware_vendor = node.vendor
            self.motherboard.firmware_version = node.version
            self.motherboard.firmware_date = node.date

        elif node.classname == 'disk' and node.id.startswith('disk') and node.product and node.size:
            self.disks.append(node)

        elif node.classname == 'display' and node.id == 'display' and node.product:
            self.gpus.append(node)

        elif node.classname == 'multimedia' and node.id == 'multimedia' and node.product:
            self.sound_adapters.append(node)

        elif node.classname == 'network' and node.id == 'network' and node.product:
            self.network_adapters.append(node)

            if node.capacity:
                node.bandwidth = format_number(node.capacity, TRANSFER_UNITS)

            if node.clock:
                node.frequency = format_number(node.clock, FREQUENCY_UNITS)

        elif node.classname == 'disk' and node.id == 'cdrom' and node.product:
            self.optical_drives.append(node)

        # recursive into node's children
        for child in node.children:
            self.__retrieve_info(child)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief load <list>
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_list(self, node):
        nodes = []

        # parse elements
        node = node.children

        while node:
            if node.type == 'element' and node.name == 'node':
                n = self.__load_node(node)
                nodes.append(n)

            node = node.next

        return nodes

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief load <node>
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_node(self, node):

        # load properties
        n = data_holder()
        n.id = node.prop('id') or ''
        n.classname = node.prop('class') or ''
        n.description = None
        n.product = None
        n.vendor = None
        n.children = []
        n.settings = {}

        # parse elements
        node = node.children

        while node:
            if node.type == 'element' and node.name == 'node':
                child = self.__load_node(node)
                n.children.append(child)

            elif node.type == 'element' and node.name == 'configuration':
                n.settings = self.__load_configuration(node)

            elif node.type == 'element' and node.name != 'capabilities':
                setattr(n, node.name, self.__get_node_content(node))

            node = node.next

        return n

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief load <configuration>
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __load_configuration(self, node):
        settings = {}

        # parse elements
        node = node.children

        while node:
            if node.type == 'element' and node.name == 'setting':
                id = node.prop('id') or ''
                value = node.prop('value') or ''
                settings[id] = value

            node = node.next

        return settings

    def __get_node_content(self, node):
        content = node.getContent().strip()

        # normalize
        if node.name == 'vendor' and content == content.upper():
            content = content.capitalize()

        elif node.name == 'serial' and content.startswith('WD-'):
            content = content[3:]

        elif node.name == 'product':
            if content.startswith('WDC '):
                content = content[4:]

            while '  ' in content:
                content = content.replace('  ', ' ')

        return content


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    mobius.core.add_resource('ui.dnd-file.' + EXTENSION_ID, 'UI DND: lshw files', dnd_callback)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('ui.dnd-file.' + EXTENSION_ID)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Callback handler for DND
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def dnd_callback(parent, pos, uri):
    # check signature
    f = mobius.io.new_file_by_url(uri)
    fp = f.new_reader()
    data = fp.read(256)

    if not data.startswith(b'<?xml version="1.0"') or b'lshw' not in data:
        return 0

    # decode file
    data = lshw_decoder(uri)
    transaction = parent.case.new_transaction()

    # create computer item
    if data.computer.type == 'notebook':
        computer = parent.new_child('notebook', pos + 1)
    else:
        computer = parent.new_child('computer', pos + 1)

    computer.name = data.computer.product
    computer.vendor = data.computer.vendor
    computer.model = data.computer.product
    computer.serial = data.computer.serial
    computer.version = data.computer.version

    # create computer subitems
    if data.motherboard:
        child = computer.new_child('motherboard')
        child.name = data.motherboard.product
        child.vendor = data.motherboard.vendor
        child.model = data.motherboard.product
        child.serial = data.motherboard.serial
        child.firmware_vendor = data.motherboard.firmware_vendor
        child.firmware_version = data.motherboard.firmware_version
        child.firmware_date = data.motherboard.firmware_date

    for i in data.processors:
        child = computer.new_child('processor')
        child.name = i.product
        child.vendor = i.vendor
        child.model = i.product
        child.clock_frequency = i.frequency

    for i in data.disks:
        product = i.product.split('-')[0]

        child = computer.new_child('harddisk')
        child.vendor = i.vendor
        child.model_id = product
        child.serial = i.serial
        child.firmware = i.version
        child.real_capacity = i.size + " bytes"
        child.part_id = product
        child.expand_masks()

    for i in data.optical_drives:
        child = computer.new_child('opticaldrive')
        child.name = i.product
        child.vendor = i.vendor
        child.model = i.product
        child.firmware_revision = i.version

    for i in data.gpus:
        child = computer.new_child('gpu_adapter')
        child.name = i.product
        child.vendor = i.vendor
        child.model = i.product

    for i in data.sound_adapters:
        child = computer.new_child('soundcard')
        child.name = i.product
        child.vendor = i.vendor
        child.model = i.product

    for i in data.network_adapters:
        if 'wireless' in i.settings:
            child = computer.new_child('wireless')
            child.name = i.product
            child.vendor = i.vendor
            child.model = i.product
            child.mac_address = i.serial
            child.frequency = i.frequency
        else:
            child = computer.new_child('ethernet')
            child.name = i.product
            child.vendor = i.vendor
            child.model = i.product
            child.mac_address = i.serial
            child.bandwidth = i.bandwidth

    transaction.commit()

    return 1  # one item created
