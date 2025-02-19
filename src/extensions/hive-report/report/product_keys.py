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

from common import *
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Product keys report
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ProductKeyReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'product-keys'
        self.name = 'Product keys'
        self.group = 'app'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_report_name('%s of computer <%s>' % (self.name, get_computer_name(registry)))

        # Microsoft products
        for product_name, product_key in self.get_microsoft_product_keys(registry):
            self.viewer.add_row((product_name, product_key))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate Microsoft products keys
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_microsoft_product_keys(self, registry):

        # Microsoft Windows
        key = registry.get_key_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion')
        if key:
            product_name = get_data_as_string(key.get_data_by_name('ProductName'))
            digital_product_id = get_data_as_buffer(key.get_data_by_name('DigitalProductId'))
            product_key = get_product_key(digital_product_id)
            yield product_name, product_key

        # Internet Explorer
        key = registry.get_key_by_path('HKLM\\SOFTWARE\\Microsoft\\Internet Explorer')
        if key:
            product_name = 'Windows Internet Explorer v' + get_data_as_string(key.get_data_by_name('Version'))
            digital_product_id = get_data_as_buffer(key.get_data_by_path('Registration\\DigitalProductID'))
            product_key = get_product_key(digital_product_id)
            yield product_name, product_key

        # Microsoft Office
        for guid_key in registry.get_key_by_mask('\\HKLM\\SOFTWARE\\Microsoft\\Office\\*\\Registration'):
            product_name = get_data_as_string(guid_key.get_data_by_name('ProductName')) or get_data_as_string(
                guid_key.get_data_by_name('ConvertToEdition'))
            digital_product_id = get_data_as_buffer(guid_key.get_data_by_name('DigitalProductId'))

            if product_name and digital_product_id:
                product_key = get_product_key(digital_product_id)
                yield product_name, product_key

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'tableview')

        column = self.viewer.add_column('product')
        column.is_sortable = True

        column = self.viewer.add_column('key')
        column.is_sortable = True

        self.viewer.set_report_id('registry.' + self.id)
        self.viewer.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
