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
# @brief O.S. info report
# @author Eduardo Aguiar
# @reference http://support.microsoft.com/kb/102986
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class OSInfoReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'os-info'
        self.name = 'General info'
        self.group = 'os'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear_values()

        # set report name
        self.viewer.set_report_name('O.S. general info of computer <%s>' % get_computer_name(registry))

        # initialize data
        product_name = ''
        csd_version = ''
        build = ''
        install_date = ''
        product_id = ''
        product_key = ''
        registered_owner = ''
        registered_organization = ''
        default_user_name = ''
        default_domain_name = ''
        max_activation_date = ''
        last_shutdown_time = ''
        last_user_name = ''
        timezone_bias = ''

        # HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion values
        cv_key = registry.get_key_by_path('HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion')

        if cv_key:
            product_name = get_data_as_string(cv_key.get_data_by_name('ProductName'))
            csd_version = get_data_as_string(cv_key.get_data_by_name('CSDVersion'))
            build = get_data_as_string(cv_key.get_data_by_name('BuildLab'))
            install_date = get_data_as_unix_datetime(cv_key.get_data_by_name('InstallDate'))
            product_id = get_data_as_string(cv_key.get_data_by_name('ProductId'))
            registered_owner = get_data_as_string(cv_key.get_data_by_name('RegisteredOwner'))
            registered_organization = get_data_as_string(cv_key.get_data_by_name('RegisteredOrganization'))

            data = cv_key.get_data_by_name('DigitalProductId')
            product_key = get_product_key(data.data) if data else ''

            # HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\WinLogon values
            winlogon_key = cv_key.get_key_by_name("WinLogon")

            if winlogon_key:
                default_user_name = get_data_as_string(winlogon_key.get_data_by_name('DefaultUserName'))
                default_domain_name = get_data_as_string(winlogon_key.get_data_by_name('DefaultDomainName'))
                last_user_name = get_data_as_string(winlogon_key.get_data_by_name('LastUsedUserName'))

        # max activation date
        value = get_data_as_nt_datetime(registry.get_data_by_path(
            '\\HKLM\\SECURITY\\Policy\\Secrets\\L$RTMTIMEBOMB_1320153D-8DA3-4e8e-B27B-0D888223A588\\CurrVal\\(default)'))

        # last shutdown time
        last_shutdown_time = get_data_as_nt_datetime(
            registry.get_data_by_path('HKLM\\SYSTEM\\CurrentControlSet\\Control\\Windows\\ShutdownTime'))

        # timezone
        # @see http://support.microsoft.com/kb/102986
        tz_key = registry.get_key_by_path('HKLM\\SYSTEM\\CurrentControlSet\\Control\\TimeZoneInformation')

        if tz_key:
            data = tz_key.get_data_by_name('ActiveTimeBias')

            if data:
                tz = data.get_data_as_dword()

                if tz == 0:
                    timezone_bias = 'UTC'

                elif tz:
                    sign = '-' if tz > 0 else '+'
                    timezone_bias = '%s%02d:%02d from GMT' % (sign, tz // 60, tz % 60)

        # set values
        self.viewer.set_value('product_name', product_name)
        self.viewer.set_value('csd_version', csd_version)
        self.viewer.set_value('build', build)
        self.viewer.set_value('install_date', install_date)
        self.viewer.set_value('max_activation_date', max_activation_date)
        self.viewer.set_value('product_id', product_id)
        self.viewer.set_value('product_key', product_key)
        self.viewer.set_value('registered_owner', registered_owner)
        self.viewer.set_value('registered_organization', registered_organization)
        self.viewer.set_value('default_user', default_user_name)
        self.viewer.set_value('default_domain', default_domain_name)
        self.viewer.set_value('last_user_name', last_user_name)
        self.viewer.set_value('last_shutdown', last_shutdown_time)
        self.viewer.set_value('timezone_bias', timezone_bias)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'attribute-list')
        self.viewer.set_report_id('registry.' + self.id)
        self.viewer.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
        self.viewer.add_value('product_name', 'product name')
        self.viewer.add_value('csd_version', 'CSD Version')
        self.viewer.add_value('build', 'build')
        self.viewer.add_value('install_date', 'install date')
        self.viewer.add_value('max_activation_date', 'max activation date')
        self.viewer.add_value('product_id', 'product id')
        self.viewer.add_value('product_key', 'product key')
        self.viewer.add_value('registered_owner', 'registered owner')
        self.viewer.add_value('registered_organization', 'registered organization')
        self.viewer.add_value('default_user', 'default user name')
        self.viewer.add_value('default_domain', 'default domain name')
        self.viewer.add_value('last_user_name', 'last used user name')
        self.viewer.add_value('last_shutdown', 'last shutdown date/time')
        self.viewer.add_value('timezone_bias', 'timezone bias')
        self.viewer.add_value('timezone_name', 'timezone standard name')


