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
import binascii

import pymobius
import pymobius.registry.cached_credentials

from common import *
from metadata import *


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Cached domain credentials report
# @author Vladimir Santos and Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class UserCachedCredentialReport(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.id = 'cached-credentials'
        self.name = 'Cached credentials'
        self.group = 'user'

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, registry):
        self.viewer.clear()

        # set report name
        self.viewer.set_master_report_name('%s of computer <%s>' % (self.name, get_computer_name(registry)))

        # fill data
        for value_name, cache in pymobius.registry.cached_credentials.get(registry):
            mscachehash = binascii.b2a_hex(cache.mscachehash)
            chksum = binascii.b2a_hex(cache.chksum)
            iv = binascii.b2a_hex(cache.iv)
            encryption_flags = '0x%08x' % cache.encryption_flags

            # format data
            revision = '%d.%d' % (cache.revision >> 16, cache.revision & 0xffff)

            if cache.encryption_flags:
                if cache.encryption_algorithm == 0:
                    encryption_algorithm = 'MSDCCv1'

                elif cache.encryption_algorithm == 10:
                    encryption_algorithm = 'MSDCCv2'

                else:
                    encryption_algorithm = 'unknown'

            else:
                encryption_algorithm = 'not encrypted'

            self.viewer.add_row((cache.username, cache.rid, cache.gid, cache.name, cache.fullname, cache.email,
                                 cache.computer, cache.home_dir, cache.profile_path, cache.home_dir_letter,
                                 cache.logon_script, cache.last_logon_date, cache.logon_domain, cache.domain,
                                 cache.dns_domain, cache.domain_sid, mscachehash, encryption_flags,
                                 encryption_algorithm, revision, chksum, iv))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build viewer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def build_viewer(self):
        self.viewer = pymobius.mediator.call('ui.new-widget', 'widetableview')

        column = self.viewer.add_column('username')
        column.is_sortable = True

        column = self.viewer.add_column('RID', column_type='int')
        column.is_sortable = True

        column = self.viewer.add_column('GID', column_type='int')
        column.is_sortable = True

        column = self.viewer.add_column('name')
        column.is_visible = False

        column = self.viewer.add_column('fullname')
        column.is_visible = False

        column = self.viewer.add_column('email', column_name='e-mail')
        column.is_visible = False

        column = self.viewer.add_column('computer', column_name='computer name')
        column.is_visible = False

        column = self.viewer.add_column('home_dir', column_name='home directory')
        column.is_visible = False

        column = self.viewer.add_column('profile_path', column_name='profile path')
        column.is_visible = False

        column = self.viewer.add_column('home_dir_letter', column_name='home drive letter')
        column.is_visible = False

        column = self.viewer.add_column('logon_script', column_name='logon script')
        column.is_visible = False

        column = self.viewer.add_column('last_logon_date', column_name='last logon date')
        column.is_sortable = True

        column = self.viewer.add_column('logon_domain', column_name='logon domain name')
        column.is_visible = False

        column = self.viewer.add_column('domain', column_name='domain name')
        column.is_visible = False

        column = self.viewer.add_column('dns_domain', column_name='DNS domain name')
        column.is_visible = False

        column = self.viewer.add_column('domain_sid', column_name='domain SID')
        column.is_visible = False

        column = self.viewer.add_column('mscachehash', column_name='ms-cache hash')
        column.is_sortable = True

        column = self.viewer.add_column('encryption_flags', column_name='encryption flags')
        column.is_visible = False

        column = self.viewer.add_column('encryption_algorithm', column_name='encryption algorithm')
        column.is_visible = False

        column = self.viewer.add_column('revision')
        column.is_visible = False

        column = self.viewer.add_column('chksum', column_name='MD5 digest')
        column.is_visible = False

        column = self.viewer.add_column('iv', column_name='initialization vector')
        column.is_visible = False

        self.viewer.set_master_report_id('registry.' + self.id)
        self.viewer.set_master_report_app('%s v%s' % (EXTENSION_NAME, EXTENSION_VERSION))
