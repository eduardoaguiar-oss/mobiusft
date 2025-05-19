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
import traceback

import pymobius
import mobius.core.encoder

from common import *
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Convert a LSA secret value into a printable format
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def lsa_to_string(name, value):
    '''
  Known LSA secrets:
  * $MACHINE.ACC: computer account password on the domain controller.
  * NL$KM: cached domain password hashes encryption key.
  * L$_RasDefaultCredentials#0: default dial-up/VNC parameters.
  * L$HYDRAENCKEY_28ada6da-d622-11d1-9cb9-00c04fb16e75: public RSA2 key used
    in the Remote Desktop Protocol.
  * L$RTMTIMEBOMB_1320153D-8DA3-4e8e-B27B-0D888223A588: limit date to activate
    the Windows copy.
  * RasDialParams!S-1-5-21-xxxxxxxxxx-xxxxxxxxxx-xxxxxxxxx-xxxx#0: user
    dial-up/VNC parameters.
  * DefaultPassword: default password used if Windows auto logon is enabled.
  * aspnet_WP_PASSWORD: 'ASPNET' user password.
  * SCM:{3D14228D-FBE1-11D0-995D-00C04FD919C1}\x00: IWAM_xxx user password.
  * 0083343a-f925-4ed7-b1d6-d95d17a0b57b-RemoteDesktopHelpAssistantAccount:
    'HelpAssistant' user password.
  * 0083343a-f925-4ed7-b1d6-d95d17a0b57b-RemoteDesktopHelpAssistantSID:
    'HelpAssistant' user SID.
  * c261dd33-c55b-4a37-924b-746bbf3569ad-RemoteDesktopHelpAssistantEncrypt:
    'HelpAssistant' user GUID.
  '''
    try:
        if name in ('0083343a-f925-4ed7-b1d6-d95d17a0b57b-RemoteDesktopHelpAssistantAccount',
                    'c261dd33-c55b-4a37-924b-746bbf3569ad-RemoteDesktopHelpAssistantEncrypt',
                    'SCM:{3D14228D-FBE1-11D0-995D-00C04FD919C1}\x00',
                    'aspnet_WP_PASSWORD', 'DefaultPassword') or \
                name.startswith('L$_RasDefaultCredentials#') or name.startswith('RasDialParams!'):
            return '\n'.join(value.decode('utf-16-le').rstrip('\x00').split('\x00'))

        if name == '0083343a-f925-4ed7-b1d6-d95d17a0b57b-RemoteDesktopHelpAssistantSID':
            return '-'.join([str(i) for i in struct.unpack('<cB6xBIIIII', 'S' + value)])

        if name == 'L$RTMTIMEBOMB_1320153D-8DA3-4e8e-B27B-0D888223A588':
            return get_nt_datetime(struct.unpack('<Q', value[:8])[0])

        return mobius.core.encoder.hexstring(value, ' ')

    except Exception as e:
        mobius.core.logf(f'WRN {str(e)}\n{traceback.format_exc()}')

        if isinstance(value, bytes):
            return mobius.core.encoder.hexstring(value, ' ')

        return None


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief LSA secrets report
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class LSASecretsReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'lsa-secrets'
        self.name = 'LSA secrets'
        self.group = 'os'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_report_name('%s of computer <%s>' % (self.name, get_computer_name(registry)))

        # fill data
        for key in registry.get_key_by_mask('\\HKLM\\SECURITY\\Policy\\Secrets\\*'):
            currval = get_data_as_buffer(key.get_data_by_path('Currval\\(default)'))
            oldval = get_data_as_buffer(key.get_data_by_path('Oldval\\(default)'))
            cupdtime = get_data_as_nt_datetime(key.get_data_by_path('CupdTime\\(default)'))
            oupdtime = get_data_as_nt_datetime(key.get_data_by_path('OupdTime\\(default)'))

            currval = lsa_to_string(key.name, currval)
            oldval = lsa_to_string(key.name, oldval)

            self.viewer.add_row((key.name, oupdtime, oldval))
            self.viewer.add_row((key.name, cupdtime, currval))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'tableview')

        column = self.viewer.add_column('name')
        column.is_sortable = True

        column = self.viewer.add_column('updtime', column_name='update time')
        column.is_sortable = True

        column = self.viewer.add_column('value', column_name='value')
        column.is_sortable = True

        self.viewer.set_report_id('registry.' + self.id)
        self.viewer.set_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
