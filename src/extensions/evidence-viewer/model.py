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
import pymobius.ant.turing

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Bin2Text formatter
# @param value Bytes value
# @return String value
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def formatter_bin2text(value):
    if not value:
        return ''

    elif isinstance(value, str):
        return value

    for encoding in ('utf-8', 'cp1252', 'iso-8859-1'):
        try:
            return value.decode(encoding)
        except UnicodeDecodeError:
            pass

    return '<BINARY> ' + mobius.core.encoder.hexstring(value)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Chat message recipients formatter
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def recipients_formatter(value):
    return '\n'.join(sorted(value or []))


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Chat message text formatter
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def text_formatter(text_segments):
    text = ''
    last_type = ''

    for data in (text_segments or []):
        item_type = data.get('type')

        if text and text[-1] not in '\n ':
            text += ' '

        if text and last_type == 'end/quote':
            text += '\n'

        if item_type == 'text':
            text += pango_escape(data.get('text'))

        elif item_type == 'system':
            text += f'<span color="#77b">{pango_escape(data.get("text"))}</span>'

        elif item_type == 'href':
            text += f'<span underline="single" color="#0000ee">{pango_escape(data.get("url"))}</span>'

        elif item_type == 'start/b':
            text += '<b>'

        elif item_type == 'start/i':
            text += '<i>'

        elif item_type == 'start/s':
            text += '<s>'

        elif item_type == 'start/quote':
            text += f'<span color="#0080b0">[{data.get("timestamp")}] {pango_escape(data.get("author"))}:\n<i>'

        elif item_type == 'end/b':
            text += '</b>'

        elif item_type == 'end/i':
            text += '</i>'

        elif item_type == 'end/s':
            text += '</s>'

        elif item_type == 'end/quote':
            text += '</i></span>'

        elif item_type == 'emoji':
            code = data.get('code')
            if code:
                text += f'<span size="x-large">{code}</span>'
            else:
                text += f'<span color="#00d000" weight="bold">{pango_escape(data.get("text"))}</span>'

        elif item_type == 'flag':
            code = data.get('code')
            if code:
                text += f'<span size="x-large">{code}</span>'
            else:
                text += f'<span color="#00d000" weight="bold">{pango_escape(data.get("text"))}</span>'

        else:  # unknown markup
            text += f'<span color="#FF0000" weight="bold">UNKNOWN: {item_type} ({pango_escape(data.get("text"))})</span>'

        last_type = item_type

    return text


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Escape pango control chars from text
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pango_escape(text):
    text = text or ''
    text = text.replace('&', '&amp;')
    text = text.replace('<', '&lt;')
    text = text.replace('>', '&gt;')
    return text


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Data formatters
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
FORMATTERS = {
    "chat-message-recipients": recipients_formatter,
    "chat-message-text": text_formatter,
    "bin2text": formatter_bin2text,
    "datetime": pymobius.to_string,
    "hexstring": mobius.core.encoder.hexstring,
    "multiline": lambda lines: '\n'.join(lines or []),
    "string": pymobius.to_string,
}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Generate dict from argument dict
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def args(**kwargs):
    return kwargs


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Getter class for object attribute
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Getter(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, attr_id, attr_format=None):
        self.__attr_id = attr_id
        self.__formatter = FORMATTERS.get(attr_format)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __call__(self, obj):
        v = getattr(obj, self.__attr_id)

        if self.__formatter:
            return self.__formatter(v)

        return v


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Evidence model
#
# Each row is composed of dictionaries, with the following keys:
# id: evidence type
# name: evidence name
# description: evidence description
# view_id: (deprecated)
# master_views: list of master views for this evidence type
# detail_views: list of detail views for this evidence type
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
MODEL = [
    args(id="autofill",
         name="Autofill data",
         description="Autofill text data",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='username', is_sortable=True, first_sortable=True),
                      args(id='app_name', name="Application", is_sortable=True),
                      args(id='field_name', name="Field Name", is_sortable=True),
                      args(id="value", is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='field_name', name="Field Name"),
                      args(id='value'),
                      args(id='username')
                  ]),
         ]
         ),
    args(id="bookmarked-url",
         name="Bookmarked URLs",
         description="URL's bookmarked by users",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='creation_time', name="Creation Date/time (UTC)", format='datetime', first_sortable=True),
                      args(id='username', name="User name", is_sortable=True),
                      args(id="app_name", name="Application", is_sortable=True),
                      args(id="name", is_sortable=True),
                      args(id="url", name="URL", is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id="name"),
                      args(id="url", name="URL"),
                      args(id='folder', name="Folder name"),
                      args(id='username', name="User name"),
                      args(id='creation_time', name="Creation Date/time (UTC)"),
                  ]),
         ]
         ),
    args(id="call",
         name="Call Logs",
         description="Regular phone calls and VOIP phone calls metadata",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='timestamp', name="Date/time (UTC)", format='datetime', first_sortable=True),
                      args(id='source', is_sortable=True),
                      args(id='destination', is_sortable=True, format="multiline"),
                      args(id='duration', is_sortable=True),
                      args(id='username', name="User name", is_sortable=True),
                      args(id="app", name="Application", is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='timestamp', name="Date/time (UTC)", format='datetime'),
                      args(id='source'),
                      args(id='destination', format="multiline"),
                      args(id='duration'),
                      args(id='username', name="User name"),
                      args(id="app", name="Application"),
                  ]),
         ]),
    args(id="chat-message",
         name="Chat Messages",
         description="Instant chat messages",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='timestamp', name="Date/time (UTC)", format='datetime', first_sortable=True),
                      args(id='sender', is_sortable=True),
                      args(id='recipients', format="chat-message-recipients", is_sortable=True),
                      args(id="text", format="chat-message-text", is_sortable=True, is_markup=True),
                      args(id='username', name="User name", is_sortable=True),
                      args(id="app", name="Application", is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='timestamp', name="Date/time (UTC)"),
                      args(id='search_type', name="Type"),
                      args(id='username', name="User name"),
                      args(id="text", format="chat-message-text"),
                  ]),
         ]
         ),
    args(id="contact",
         name="Contacts",
         description="Contacts from applications",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='id', is_sortable=True),
                      args(id='name', is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='id', name="ID"),
                      args(id='name', name="Name"),
                      args(id='names', name='Names'),
                      args(id='birthday', name='Birthday'),
                      args(id='gender', name='Gender'),
                      args(id='phones', name='Phones', format="multiline"),
                      args(id='addresses', name='Addresses', format="multiline"),
                      args(id='emails', name='E-mails', format="multiline"),
                      args(id='accounts', name='User accounts', format="multiline"),
                      args(id='web_addresses', name='Web Addresses', format="multiline"),
                      args(id='notes', name='Notes', format="multiline"),
                      args(id='organizations', name='Organizations', format="multiline"),
                  ]),
         ]
         ),
    args(id="cookie",
         name="Cookies",
         description="HTTP cookies data",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='creation_time', name="Creation Date/time (UTC)", format='datetime', first_sortable=True),
                      args(id='username', name="User name", is_sortable=True),
                      args(id="app_name", name="Application", is_sortable=True),
                      args(id='domain', is_sortable=True),
                      args(id='name', is_sortable=True),
                      args(id='value', format="bin2text", is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='name'),
                      args(id='value', format="bin2text"),
                      args(id='domain'),
                      args(id='creation_time', name="Creation date/time (UTC)", format='datetime'),
                      args(id='last_access_time', name="Last access date/time (UTC)", format='datetime'),
                      args(id='expiration_time', name="Expiration date/time (UTC)", format='datetime'),
                      args(id='last_update_time', name="Last update date/time (UTC)", format='datetime'),
                      args(id='is_deleted'),
                      args(id='is_encrypted'),
                  ]),
         ]
         ),
    args(id="credit-card",
         name="Credit Cards",
         description="Credit cards information stored by applications",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='number', name="Credit Card Number", first_sortable=True),
                      args(id='company', name="Company", is_sortable=True),
                      args(id='name', name="Owner Name", is_sortable=True),
                      args(id='source', name="Source", is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='number', name="Credit Card Number"),
                      args(id='company', name="Company"),
                      args(id='name', name="Owner Name"),
                      args(id='expiration_date', name="Expiration Date"),
                      args(id='last_used_time', name="Last used date/time (UTC)"),
                      args(id='address', name="Billing Address"),
                      args(id='source', name="Source"),
                  ]),
         ]
         ),
    args(id="crypto-wallet",
         name="Crypto Wallets",
         description="Crypto Wallet addresses found",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='symbol', name="Symbol", first_sortable=True),
                      args(id='address', name="Address", is_sortable=True),
                      args(id='source', name="Source"),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='symbol', name="Symbol"),
                      args(id='address', name="Address"),
                      args(id='source', name="Source"),
                  ]),
         ]
         ),
    args(id="encryption-key",
         name="Encryption Keys",
         description="Encryption keys",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='key_type', name="Type", first_sortable=True),
                      args(id='id', name="ID", is_sortable=True),
                      args(id='value', format="hexstring"),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='key_type', name="Type"),
                      args(id='id', name="ID"),
                      args(id='value', format="hexstring"),
                  ]),
         ]
         ),
    args(id="installed-program",
         name="Installed Programs",
         description="Installed programs",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='name', is_sortable=True),
                      args(id='version'),
                      args(id='description')
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='name'),
                      args(id='version'),
                      args(id='description')
                  ]),
         ]
         ),
    args(id="instant-message",
         name="Instant Messages",
         description="SMS and other instant messages",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='timestamp', name="Date/time (UTC)", format='datetime', first_sortable=True),
                      args(id="message_type", name="Type", is_sortable=True),
                      args(id='sender', is_sortable=True),
                      args(id='recipients', format="chat-message-recipients", is_sortable=True),
                      args(id="text"),
                      args(id="app", name="Application", is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='timestamp', name="Date/time (UTC)", format='datetime'),
                      args(id="message_type", name="Type"),
                      args(id='sender'),
                      args(id='recipients', format="chat-message-recipients"),
                      args(id="text"),
                      args(id="app", name="Application"),
                  ]),
         ]
         ),

    args(id="ip-address",
         name="IP Addresses",
         description="External IP addresses recorded by applications",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='timestamp', name="Date/time (UTC)", format='datetime', first_sortable=True),
                      args(id='address', name="IP Address", is_sortable=True),
                      args(id='username', name="User name", is_sortable=True),
                      args(id="app_name", name="Application", is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='username', name="User name"),
                      args(id='timestamp', name="Date/time (UTC)"),
                      args(id='address', name="IP Address"),
                  ]),
         ]
         ),
    args(id="local-file",
         name="Local Files",
         description="Files that are/were in suspect disk, according to applications records",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='username', name="User account", is_sortable=True),
                      args(id='app_name', name="Application", first_sortable=True),
                      args(id='path', is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='username', name="User account"),
                      args(id='app_name', name="Application"),
                      args(id='path'),
                  ]),
             args(id="hashes"
                  ),
         ]
         ),
    args(id="opened-file",
         name="Opened Files",
         description="Files opened by users",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='timestamp', name="Date/time (UTC)", format='datetime', first_sortable=True),
                      args(id='username', name="User name", is_sortable=True),
                      args(id='path', is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='username', name="User name"),
                      args(id='timestamp', name="Date/time (UTC)"),
                      args(id='app_name', name="Application"),
                  ]),
         ]
         ),
    args(id="p2p-remote-file",
         name="P2P Remote Files",
         description="Files shared/uploaded by third parties using P2P networks",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='timestamp', name="Date/Time (UTC)", format='datetime', first_sortable=True),
                      args(id='ip', name="IP Address"),
                      args(id='port', name="Port number", type="int"),
                      args(id='filename', name="File name", is_sortable=True),
                      args(id='username', name="User account", is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='timestamp', name="Date/Time (UTC)", format='datetime'),
                      args(id='ip', name="IP Address"),
                      args(id='port', name="Port number"),
                      args(id='username', name="User account"),
                      args(id='app_name', name="Application"),
                      args(id='filename', name="File name"),
                  ]),
             args(id="hashes"
                  ),
         ]
         ),
    args(id="password-hash",
         name="Password Hashes",
         description="Password hashes, such as NTLM and LM",
         view_id="password-hashes",
         ),
    args(id="password",
         name="Passwords",
         description="Passwords found",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='password_type', name="Type", first_sortable=True),
                      args(id='value', name="Password"),
                      args(id='description'),
                  ],
                  exporters=[
                      args(id="wordlist", name="Word list", extensions="txt", function=pymobius.ant.turing.exporter_wordlist),
                  ])
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='password_type', name="Type"),
                      args(id='value', name="Password"),
                      args(id='account', name="Account"),
                      args(id='description'),
                  ]),
         ]
         ),
    args(id="pdi",
         name="Personal Direct Identifier",
         description="Personal Direct Identifier (PDI), such as SSN, CPF, passport number, ...",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='pdi_type', name="PDI Type", is_sortable=True),
                      args(id="value", is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='pdi_type', name="PDI Type"),
                      args(id='value'),
                  ]),
         ]
         ),
    args(id="received-file",
         name="Received Files",
         description="Files received by local users",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='timestamp', name="Date/Time (UTC)", format='datetime', first_sortable=True),
                      args(id='username', name="User name", is_sortable=True),
                      args(id='filename', name="File name", is_sortable=True),
                      args(id='path', is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='timestamp', name="Start Time (UTC)"),
                      args(id='username', name="User name"),
                      args(id='app_name', name="Application"),
                      args(id='filename', name="File name"),
                      args(id='path'),
                  ]),
             args(id="hashes"
                  ),
         ]
         ),
    args(id="searched-text",
         name="Searched Texts",
         description="Texts searched by users",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='timestamp', name="Date/time (UTC)", format='datetime', first_sortable=True),
                      args(id='search_type', name="Type", is_sortable=True),
                      args(id='username', name="User name", is_sortable=True),
                      args(id="text", is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='timestamp', name="Date/time (UTC)"),
                      args(id='search_type', name="Type"),
                      args(id='username', name="User name"),
                      args(id="text"),
                  ]),
         ]
         ),
    args(id="sent-file",
         name="Sent Files",
         description="Files sent by local users",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='timestamp', name="Date/Time (UTC)", format='datetime', first_sortable=True),
                      args(id='username', name="User account", is_sortable=True),
                      args(id='filename', name="File name", is_sortable=True),
                      args(id='path', is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='timestamp', name="Date/Time (UTC)", format='datetime'),
                      args(id='username', name="User account"),
                      args(id='app_name', name="Application"),
                      args(id='filename', name="File name"),
                      args(id='path'),
                  ]),
             args(id="hashes"
                  ),
         ]
         ),
    args(id="shared-file",
         name="Shared Files",
         description="Files shared by local users",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='username', name="User account", is_sortable=True),
                      args(id='app_name', name="Application", first_sortable=True),
                      args(id='filename', name="File name", is_sortable=True),
                      args(id='path', is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='username', name="User account"),
                      args(id='app_name', name="Application"),
                      args(id='filename', name="File name"),
                      args(id='path'),
                  ]),
             args(id="hashes"
                  ),
         ]
         ),
    args(id="trash-can-entry",
         name="Trash Can Entries",
         description="Trash can entries",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='deletion_time', name="Deletion Date/time (UTC)", format='datetime', first_sortable=True),
                      args(id='path', is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='path', name="File path"),
                      args(id='size', name="File size"),
                      args(id='deletion_time', name="Deletion Date/time (UTC)"),
                  ]),
             args(id="hashes"
                  ),
         ]
         ),
    args(id="user-account",
         name="User Accounts",
         description="User accounts",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='account_type', name='Type', first_sortable=True),
                      args(id='id', name='ID', is_sortable=True),
                      args(id='password_found'),
                      args(id='password'),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='account_type', name='Type'),
                      args(id='id', name='Account ID'),
                      args(id='name'),
                      args(id='password'),
                      args(id='password_found'),
                      args(id='names', name='Names', format="multiline"),
                      args(id='phones', name='Phones', format="multiline"),
                      args(id='addresses', name='Addresses', format="multiline"),
                      args(id='emails', name='E-mails', format="multiline"),
                      args(id='accounts', name='User accounts', format="multiline"),
                      args(id='web_addresses', name='Web Addresses', format="multiline"),
                      args(id='notes', name='Notes', format="multiline"),
                      args(id='organizations', name='Organizations', format="multiline"),
                  ]),
         ]
         ),
    args(id="visited-url",
         name="Visited URLs",
         description="URLs visited by users",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='timestamp', name="Date/time (UTC)", format='datetime', first_sortable=True),
                      args(id='username', name="User name", is_sortable=True),
                      args(id="url", name="URL", is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='timestamp', name="Date/time (UTC)"),
                      args(id="url", name="URL"),
                      args(id='title', name="Page title"),
                      args(id='username', name="User name"),
                  ]),
         ]
         ),
    args(id="wireless-connection",
         name="Wireless Connections",
         description="Wireless connections",
         master_views=[
             args(id="table",
                  columns=[
                      args(id='timestamp', name="Date/time (UTC)", format='datetime', first_sortable=True),
                      args(id='bssid', name="BSSID", is_sortable=True),
                      args(id="ssid", name="SSID", is_sortable=True),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='timestamp', name="Date/time (UTC)"),
                      args(id="bssid", name="BSSID"),
                      args(id='ssid', name="SSID"),
                  ]),
         ]
         ),
    args(id="wireless-network",
         name="Wireless Networks",
         description="Wireless networks",
         master_views=[
             args(id="table",
                  columns=[
                      args(id="ssid", name="SSID", is_sortable=True),
                      args(id='bssid', name="BSSID", is_sortable=True),
                      args(id="password", name="Password"),
                  ]),
         ],
         detail_views=[
             args(id="metadata",
                  rows=[
                      args(id='ssid', name="SSID"),
                      args(id="bssid", name="BSSID"),
                      args(id="password", name="Password"),
                  ]),
         ]
         ),
]
