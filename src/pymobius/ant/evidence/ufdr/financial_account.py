# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import mobius
import pymobius


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Handle FinancialAccount evidence
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def handler(e):
    metadata = dict(e.metadata)
    account_type = metadata.get('FinancialAccountType')

    if account_type == 'CryptocurrencyWallet':
        for asset in metadata.get('Assets', []):
            asset_metadata = dict(asset.metadata)

            evidence = pymobius.Data()
            evidence.type = 'crypto-wallet'

            evidence.attrs = {
                'symbol': asset_metadata.get('Currency'),
                'address': metadata.get('AccountID'),
                'source': metadata.get('Source'),
                'evidence_source': f"UFDR evidence #{e.id}"
            }

            evidence.metadata = mobius.pod.map()
            evidence.metadata.set('evidence-id', e.id)
            evidence.metadata.set('source-idx', e.source_index)
            evidence.metadata.set('extraction-id', e.extraction_id)
            evidence.metadata.set('deleted-state', e.deleted_state)
            evidence.metadata.set('decoding-confidence', e.decoding_confidence)
            evidence.metadata.set('service-identifier', metadata.get('ServiceIdentifier'))
            evidence.metadata.set('user-mapping', metadata.get('UserMapping'))

            yield evidence

    else:
        mobius.core.logf(f"DEV unhandled FinancialAccountType: {account_type}")
