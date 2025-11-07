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
import html
import json
import string

import mobius

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Format Account ID
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def format_account_id(text):
    if text and text[0] in string.digits and ':' in text:
        text = text.split(':', 1)[1]
    return text


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Emoji representation as Unicode char
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
EMOJI_CHARS = {
    'angel': '👼',
    'anger': '🤯',
    'angry': '😠',
    'bear': '🧸',
    'bike': '🚴',
    'blush': '😳',
    'bomb': '💣',
    'brokenheart': '💔',
    'cake': '🎂',
    'cash': '💰',
    'cat': '🐈',
    'coffee': '☕',
    'cool': '😎',
    'cry': '😢',
    'cwl': '😂',
    'dance': '🕺',
    'devil': '😈',
    'dog': '🐕',
    'drink': '🍸',
    'drunk': '🥴',
    'dull': '🙄',
    'explode': '💣',
    'explosion': '💣',
    'facepalm': '🤦',
    'ghost': '👻',
    'giggle': '🤭',
    'happy': '🙂',
    'heart': '❤',
    'hearteyes': '😍',
    'heidy': '🐿',
    'hug': '🧸',
    'inlove': '🥰',
    'kiss': '😗',
    'ladyvamp': '🧛',
    'ladyvampire': '🧛',
    'laugh': '😃',
    'lips': '💋',
    'lipssealed': '🤐',
    'loudlycrying': '😭',
    'mail': '✉',
    'mmm': '😋',
    'monkey': '🐒',
    'muscle': '💪',
    'music': '🎶',
    'nerd': '🤓',
    'nerdy': '🤓',
    'ninja': '🥷',
    'no': '👎',
    'ok': '👌',
    'party': '🥳',
    'pizza': '🍕',
    'praying': '🙏',
    'puke': '🤮',
    'rain': '🌧',
    'rofl': '🤣',
    'sad': '😧',
    'skull': '💀',
    'sleepy': '😪',
    'smile': '😄',
    'smirk': '😏',
    'speechless': '😐',
    'squirrel': '🐿',
    'star': '⭐',
    'stareyes': '🤩',
    'sun': '🌞',
    'surprised': '😲',
    'swear': '🤬',
    'sweat': '😓',
    'think': '🤔',
    'time': '⏲',
    'tongueout': '😛',
    'unamused': '😒',
    'vampire': '🧛',
    'victory': '✌',
    'wasntme': '🙄',
    'wave': '🌊',
    'wink': '😉',
    'worry': '😟',
    'xd': '😆',
    'yawn': '🥱',
    'yes': '👍',
    'yoga': '🧘',
}

UNKNOWN_EMOJIS = set()

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Flags representation as Unicode char
# @see ISO-3166
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
FLAG_CHARS = {
    "ad": "🇦🇩",
    "ae": "🇦🇪",
    "af": "🇦🇫",
    "ag": "🇦🇬",
    "ai": "🇦🇮",
    "al": "🇦🇱",
    "am": "🇦🇲",
    "ao": "🇦🇴",
    "aq": "🇦🇶",
    "ar": "🇦🇷",
    "as": "🇦🇸",
    "at": "🇦🇹",
    "au": "🇦🇺",
    "aw": "🇦🇼",
    "ax": "🇦🇽",
    "az": "🇦🇿",
    "ba": "🇧🇦",
    "bb": "🇧🇧",
    "bd": "🇧🇩",
    "be": "🇧🇪",
    "bf": "🇧🇫",
    "bg": "🇧🇬",
    "bh": "🇧🇭",
    "bi": "🇧🇮",
    "bj": "🇧🇯",
    "bl": "🇧🇱",
    "bm": "🇧🇲",
    "bn": "🇧🇳",
    "bo": "🇧🇴",
    "bq": "🇧🇶",
    "br": "🇧🇷",
    "bs": "🇧🇸",
    "bt": "🇧🇹",
    "bv": "🇧🇻",
    "bw": "🇧🇼",
    "by": "🇧🇾",
    "bz": "🇧🇿",
    "ca": "🇨🇦",
    "cc": "🇨🇨",
    "cd": "🇨🇩",
    "cf": "🇨🇫",
    "cg": "🇨🇬",
    "ch": "🇨🇭",
    "ci": "🇨🇮",
    "ck": "🇨🇰",
    "cl": "🇨🇱",
    "cm": "🇨🇲",
    "cn": "🇨🇳",
    "co": "🇨🇴",
    "cr": "🇨🇷",
    "cu": "🇨🇺",
    "cv": "🇨🇻",
    "cw": "🇨🇼",
    "cx": "🇨🇽",
    "cy": "🇨🇾",
    "cz": "🇨🇿",
    "de": "🇩🇪",
    "dj": "🇩🇯",
    "dk": "🇩🇰",
    "dm": "🇩🇲",
    "do": "🇩🇴",
    "dz": "🇩🇿",
    "ec": "🇪🇨",
    "ee": "🇪🇪",
    "eg": "🇪🇬",
    "eh": "🇪🇭",
    "er": "🇪🇷",
    "es": "🇪🇸",
    "et": "🇪🇹",
    "fi": "🇫🇮",
    "fj": "🇫🇯",
    "fk": "🇫🇰",
    "fm": "🇫🇲",
    "fo": "🇫🇴",
    "fr": "🇫🇷",
    "ga": "🇬🇦",
    "gb": "🇬🇧",
    "gd": "🇬🇩",
    "ge": "🇬🇪",
    "gf": "🇬🇫",
    "gg": "🇬🇬",
    "gh": "🇬🇭",
    "gi": "🇬🇮",
    "gl": "🇬🇱",
    "gm": "🇬🇲",
    "gn": "🇬🇳",
    "gp": "🇬🇵",
    "gq": "🇬🇶",
    "gr": "🇬🇷",
    "gs": "🇬🇸",
    "gt": "🇬🇹",
    "gu": "🇬🇺",
    "gw": "🇬🇼",
    "gy": "🇬🇾",
    "hk": "🇭🇰",
    "hm": "🇭🇲",
    "hn": "🇭🇳",
    "hr": "🇭🇷",
    "ht": "🇭🇹",
    "hu": "🇭🇺",
    "id": "🇮🇩",
    "ie": "🇮🇪",
    "il": "🇮🇱",
    "im": "🇮🇲",
    "in": "🇮🇳",
    "io": "🇮🇴",
    "iq": "🇮🇶",
    "ir": "🇮🇷",
    "is": "🇮🇸",
    "it": "🇮🇹",
    "je": "🇯🇪",
    "jm": "🇯🇲",
    "jo": "🇯🇴",
    "jp": "🇯🇵",
    "ke": "🇰🇪",
    "kg": "🇰🇬",
    "kh": "🇰🇭",
    "ki": "🇰🇮",
    "km": "🇰🇲",
    "kn": "🇰🇳",
    "kp": "🇰🇵",
    "kr": "🇰🇷",
    "kw": "🇰🇼",
    "ky": "🇰🇾",
    "kz": "🇰🇿",
    "la": "🇱🇦",
    "lb": "🇱🇧",
    "lc": "🇱🇨",
    "li": "🇱🇮",
    "lk": "🇱🇰",
    "lr": "🇱🇷",
    "ls": "🇱🇸",
    "lt": "🇱🇹",
    "lu": "🇱🇺",
    "lv": "🇱🇻",
    "ly": "🇱🇾",
    "ma": "🇲🇦",
    "mc": "🇲🇨",
    "md": "🇲🇩",
    "me": "🇲🇪",
    "mf": "🇲🇫",
    "mg": "🇲🇬",
    "mh": "🇲🇭",
    "mk": "🇲🇰",
    "ml": "🇲🇱",
    "mm": "🇲🇲",
    "mn": "🇲🇳",
    "mo": "🇲🇴",
    "mp": "🇲🇵",
    "mq": "🇲🇶",
    "mr": "🇲🇷",
    "ms": "🇲🇸",
    "mt": "🇲🇹",
    "mu": "🇲🇺",
    "mv": "🇲🇻",
    "mw": "🇲🇼",
    "mx": "🇲🇽",
    "my": "🇲🇾",
    "mz": "🇲🇿",
    "na": "🇳🇦",
    "nc": "🇳🇨",
    "ne": "🇳🇪",
    "nf": "🇳🇫",
    "ng": "🇳🇬",
    "ni": "🇳🇮",
    "nl": "🇳🇱",
    "no": "🇳🇴",
    "np": "🇳🇵",
    "nr": "🇳🇷",
    "nu": "🇳🇺",
    "nz": "🇳🇿",
    "om": "🇴🇲",
    "pa": "🇵🇦",
    "pe": "🇵🇪",
    "pf": "🇵🇫",
    "pg": "🇵🇬",
    "ph": "🇵🇭",
    "pk": "🇵🇰",
    "pl": "🇵🇱",
    "pm": "🇵🇲",
    "pn": "🇵🇳",
    "pr": "🇵🇷",
    "ps": "🇵🇸",
    "pt": "🇵🇹",
    "pw": "🇵🇼",
    "py": "🇵🇾",
    "qa": "🇶🇦",
    "re": "🇷🇪",
    "ro": "🇷🇴",
    "rs": "🇷🇸",
    "ru": "🇷🇺",
    "rw": "🇷🇼",
    "sa": "🇸🇦",
    "sb": "🇸🇧",
    "sc": "🇸🇨",
    "sd": "🇸🇩",
    "se": "🇸🇪",
    "sg": "🇸🇬",
    "sh": "🇸🇭",
    "si": "🇸🇮",
    "sj": "🇸🇯",
    "sk": "🇸🇰",
    "sl": "🇸🇱",
    "sm": "🇸🇲",
    "sn": "🇸🇳",
    "so": "🇸🇴",
    "sr": "🇸🇷",
    "ss": "🇸🇸",
    "st": "🇸🇹",
    "sv": "🇸🇻",
    "sx": "🇸🇽",
    "sy": "🇸🇾",
    "sz": "🇸🇿",
    "tc": "🇹🇨",
    "td": "🇹🇩",
    "tf": "🇹🇫",
    "tg": "🇹🇬",
    "th": "🇹🇭",
    "tj": "🇹🇯",
    "tk": "🇹🇰",
    "tl": "🇹🇱",
    "tm": "🇹🇲",
    "tn": "🇹🇳",
    "to": "🇹🇴",
    "tr": "🇹🇷",
    "tt": "🇹🇹",
    "tv": "🇹🇻",
    "tw": "🇹🇼",
    "tz": "🇹🇿",
    "ua": "🇺🇦",
    "ug": "🇺🇬",
    "um": "🇺🇲",
    "us": "🇺🇸",
    "uy": "🇺🇾",
    "uz": "🇺🇿",
    "va": "🇻🇦",
    "vc": "🇻🇨",
    "ve": "🇻🇪",
    "vg": "🇻🇬",
    "vi": "🇻🇮",
    "vn": "🇻🇳",
    "vu": "🇻🇺",
    "wf": "🇼🇫",
    "ws": "🇼🇸",
    "ye": "🇾🇪",
    "yt": "🇾🇹",
    "za": "🇿🇦",
    "zm": "🇿🇲",
    "zw": "🇿🇼"
}

UNKNOWN_FLAGS = set()

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Skype entities
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ENTITIES = {
    'amp': '&',
    'lt': '<',
    'gt': '>',
    'apos': "'",
    'quot': '"'
}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Skype message parser
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class MessageParser(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, text):
        self.__elements = []
        self.__text = text

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse message
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def parse(self):
        self.__parser = mobius.core.decoder.sgml.parser(self.__text)
        e = self.__parser.get()

        while e.type != 0:

            if e.type == 1:  # text
                self.__parse_text(e)

            elif e.type == 2:  # <tag>
                self.__parse_tag_open(e)

            elif e.type == 3:  # </tag>
                self.__parse_tag_close(e)

            elif e.type == 4:  # <tag/>
                self.__parse_tag_empty(e)

            elif e.type == 5:  # &entity;
                self.__parse_entity(e)

            e = self.__parser.get()

        return self.__elements

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse notice
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def parse_notice(self):
        l = json.loads(self.__text)

        if len(l) > 0:
            c = l[0]
            text = ''

            for at in c.get('attachments', []):
                text += at.get('content', {}).get('text')

            if text:
                element = {'type': 'system', 'text': text}
                self.add_element(element)

        return self.__elements

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse popcard
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def parse_popcard(self):
        l = json.loads(self.__text)

        if len(l) > 0:
            c = l[0]
            text = c.get('content', {}).get('text')

            if text:
                element = {'type': 'system', 'text': text}
                self.add_element(element)

        return self.__elements

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <text>
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __parse_text(self, e):
        element = {'type': 'text', 'text': e.text}
        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <tag>
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __parse_tag_open(self, e):
        tag = e.text

        f = getattr(self, '_parse_%s' % tag, None)
        if f:
            f(e)

        else:
            mobius.core.logf('DEV app.skype: unknown tag open <%s>' % tag)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse </tag>
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __parse_tag_close(self, e):
        element = None
        tag = e.text

        if tag == 'b':
            element = {'type': 'end/b'}

        elif tag == 'i':
            element = {'type': 'end/i'}

        elif tag == 's':
            element = {'type': 'end/s'}

        elif tag == 'quote':
            element = {'type': 'end/quote'}

        else:
            mobius.core.logf('DEV app.skype: unknown tag close </%s>' % tag)

        if element:
            self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <tag/>
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __parse_tag_empty(self, e):
        tag = e.text
        mobius.core.logf('DEV app.skype: unknown tag empty <%s/>' % tag)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse entity
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __parse_entity(self, e):
        element = {
            'type': 'text',
            'text': html.unescape('&' + e.text + ';')
        }
        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get value from <tag>value</tag>
    # @param e Current element
    # @return Value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_tag_value(self, e):
        tag = e.text
        value = None

        while e.type != 0 and (e.type != 3 or e.text != tag):
            if e.type == 1:
                value = e.text
            e = self.__parser.get()

        return value

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Add text element to list
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_element(self, element):

        # get previous element
        if len(self.__elements) > 0:
            p_element = self.__elements[-1]
        else:
            p_element = None

        # merge two consecutive 'text' or 'system'
        if p_element and element['type'] == p_element['type'] == 'text':
            p_element['text'] = p_element['text'] + element['text']

        elif p_element and element['type'] == p_element['type'] == 'system':
            p_element['text'] = p_element['text'] + '. ' + element['text']

        # otherwise, create new element
        else:
            self.__elements.append(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <b> node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_b(self, e):
        element = {'type': 'start/b'}
        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <i> node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_i(self, e):
        element = {'type': 'start/i'}
        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <s> node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_s(self, e):
        element = {'type': 'start/s'}
        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <a> node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_a(self, e):
        element = {'type': 'href', 'url': html.unescape(e.attributes.get('href'))}
        self.add_element(element)

        value = self.__get_tag_value(e)  # ignore text and </a>

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <ss></ss> tag
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_ss(self, e):
        element = {'type': 'emoji'}

        # get ss_type
        ss_type = e.attributes.get('type')

        if ss_type in EMOJI_CHARS:
            element['code'] = EMOJI_CHARS.get(ss_type)

        elif ss_type not in UNKNOWN_EMOJIS:
            mobius.core.logf('DEV app.skype: unknown emoji "%s"' % ss_type)
            UNKNOWN_EMOJIS.add(ss_type)

        # get text
        text = self.__get_tag_value(e)
        if text:
            element['text'] = text

        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <flag></flag> tag
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_flag(self, e):
        element = {'type': 'flag'}

        # get country
        country = e.attributes.get('country')

        if country in FLAG_CHARS:
            element['code'] = FLAG_CHARS.get(country)

        elif country not in UNKNOWN_FLAGS:
            mobius.core.logf('DEV app.skype: unknown flag "%s"' % country)
            UNKNOWN_FLAGS.add(country)

        # get text
        text = self.__get_tag_value(e)
        if text:
            element['text'] = text

        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <partlist> node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_partlist(self, e):
        parts = []

        # root node = <partlist>
        if e.type != 2 or e.text != 'partlist':
            mobius.core.logf('WRN app.skype: invalid <partlist> ' + text)
            return

        partlist_type = e.attributes.get('type')

        # get all <part> nodes
        e = self.__parser.get()

        while e.type != 0 and (e.type != 3 or e.text != 'partlist'):

            if e.type == 2 and e.text == 'part':  # <part>
                part_id = e.attributes.get('identity')
                part_name = ''

            elif e.type == 2 and e.text == 'name':  # <name>
                part_name = self.__get_tag_value(e)

            elif e.type == 3 and e.text == 'part':  # </part>
                parts.append((part_id, part_name))

            e = self.__parser.get()

        # create element
        if partlist_type == 'ended':
            text = 'Call ended.'

        elif partlist_type == 'started':
            text = 'Call started.'

        else:
            mobius.core.logf('DEV unknown partlist type "%s"' % partlist_type)
            text = ''

        text += ' Participants: ' + ', '.join(
            account + ' (' + name + ')' if name else account for (account, name) in sorted(parts))

        element = {'type': 'system', 'text': text}
        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <contacts> node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_contacts(self, e):
        contacts = []

        # root node = <contacts>
        if e.type != 2 or e.text != 'contacts':
            mobius.core.logf('WRN app.skype: invalid <contacts> ' + e.text)
            return

        # get all <c> nodes
        e = self.__parser.get()

        while e.type != 0 and (e.type != 3 or e.text != 'contacts'):

            if e.type in (2, 4) and e.text == 'c':  # <c> or <c/>
                t = e.attributes.get('t')

                if t != 's':
                    mobius.core.logf('DEV app.skype: new contact "t" value: ' + t)

                account_id = e.attributes.get('s')
                account_name = e.attributes.get('f')
                contacts.append((account_id, account_name))

            e = self.__parser.get()

        # create element
        text = 'Contacts: ' + ', '.join(
            account + ' (' + name + ')' if name else account for (account, name) in sorted(contacts))

        element = {'type': 'system', 'text': text}
        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <files> node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_files(self, e):
        filelist = []

        e = self.__parser.get()

        # get all <file> nodes
        while e.type != 0 and (e.type != 3 or e.text != 'files'):

            if e.type in (2, 4) and e.text == 'file':  # <file> or <file/>
                size = int(e.attributes.get('size', 0))
                status = e.attributes.get('status')
                timestamp = e.attributes.get('tid')

                if e.type == 2:
                    name = self.__get_tag_value(e)
                else:
                    name = None

                filelist.append((size, status, timestamp, name))

            e = self.__parser.get()

        # create element
        text = None

        if len(filelist) == 1:
            size, status, timestamp, name = filelist[0]
            text = 'File "%s" sent (size: %d bytes)' % (name, size)

        elif len(filelist) > 1:
            text = 'Files %s sent' % ', '.join('"%s"' % name for size, status, timestamp, name in filelist)

        if text:
            element = {'type': 'system', 'text': text}
            self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <quote> node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_quote(self, e):
        timestamp = e.attributes.get('timestamp')
        author_id = e.attributes.get('author')
        author_name = e.attributes.get('authorname')

        author = author_id
        if author_name:
            author += ' (' + author_name + ')'

        element = {'type': 'start/quote', 'author': author}
        if timestamp:
            element['timestamp'] = mobius.core.datetime.new_datetime_from_unix_timestamp(int(timestamp))

        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <legacyquote></legacyquote> tag
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_legacyquote(self, e):
        self.__get_tag_value(e)  # ignore text and </legacyquote>

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <sms> tag
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_sms(self, e):
        msg = e.attributes.get('alt')
        timestamp = None
        targets = []

        # process until </sms>
        e = self.__parser.get()

        while e.type != 0 and (e.type != 3 or e.text != 'sms'):
            etext = e.text

            if e.type == 2 and etext == 'sendtimestamp':
                sendtimestamp = self.__get_tag_value(e)

            elif e.type == 2 and etext == 'target':
                target = self.__get_tag_value(e)
                targets.append(target)

            e = self.__parser.get()

        # create element
        if timestamp:
            timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(int(timestamp))
            text = 'SMS message sent at %s to %s: %s' % (timestamp, ', '.join(targets), msg)

        else:
            text = 'SMS message sent to %s: %s' % (', '.join(targets), msg)

        element = {'type': 'system', 'text': text}
        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <URIObject> tag
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_URIObject(self, e):
        type = e.attributes.get('type')
        uri = e.attributes.get('uri')
        url_thumbnail = e.attributes.get('url_thumbnail')
        ams_id = e.attributes.get('ams_id')
        name = None
        meta_type = None
        meta_name = None
        filesize = None
        description = None
        title = None
        targets = []

        # process until </URIObject>
        e = self.__parser.get()

        while e.type != 0 and (e.type != 3 or e.text != 'URIObject'):
            etext = e.text

            if e.type == 2:  # <tag>
                if etext == 'a':
                    pass

                elif etext == 'Description':
                    description = self.__get_tag_value(e)

                elif etext == 'Title':
                    title = self.__get_tag_value(e)

                elif etext == 'FileSize':
                    value = self.__get_tag_value(e)
                    if value:
                        filesize = int(value)

                elif etext == 'OriginalName':
                    name = e.attributes.get('v')

                elif etext == 'target':
                    target = self.__get_tag_value(e)
                    targets.append(target)

                elif etext == 'meta':
                    meta_type = e.attributes.get('type')
                    meta_name = e.attributes.get('originalName')

                else:
                    mobius.core.logf(
                        'DEV app.skype: unhandled URIObject sub-tag <%s>: %s' % (e.text, e.attributes.value))

            elif e.type == 4:  # <tag/>
                if etext == 'Description':
                    pass

                elif etext == 'FileSize':
                    v = e.attributes.get('v')
                    if v:
                        filesize = int(v)

                elif etext == 'meta':
                    meta_type = e.attributes.get('type')
                    meta_name = e.attributes.get('originalName')

                elif etext == 'OriginalName':
                    name = e.attributes.get('v')

                elif etext == 'Title':
                    pass

                else:
                    mobius.core.logf(
                        'DEV app.skype: unhandled URIObject sub-tag <%s/>: %s' % (e.text, e.attributes.value))

            e = self.__parser.get()

        # create element
        text = 'File "%s" shared' % (name or meta_name)

        if type != None:
            text += '\nType: %s' % type

        if filesize != None:
            text += '\nSize: %d bytes' % filesize

        if title:
            text += '\nTitle: %s' % title

        if description:
            text += '\nDescription: %s' % description

        text += '\nURL: %s' % uri

        element = {'type': 'system', 'text': text}
        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <addmember> tag
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_addmember(self, e):
        timestamp = None
        initiator = None
        target = None

        # process until </addmember>
        e = self.__parser.get()

        while e.type != 0 and (e.type != 3 or e.text != 'addmember'):
            etext = e.text

            if e.type == 2:  # <tag>

                if etext == 'eventtime':
                    s = int(self.__get_tag_value(e)) // 1000
                    timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(s)

                elif etext == 'initiator':
                    initiator = format_account_id(self.__get_tag_value(e))

                elif etext == 'target':
                    target = format_account_id(self.__get_tag_value(e))

            e = self.__parser.get()

        # create element
        text = 'Member "%s"' % target

        if initiator and initiator != target:
            text += ' added to chat by user "%s"' % initiator

        else:
            text += ' entered chat'

        if timestamp:
            text += ' at %s' % timestamp

        element = {'type': 'system', 'text': text}
        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <deletemember> tag
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_deletemember(self, e):
        timestamp = None
        initiator = None
        target = None

        # process until </deletemember>
        e = self.__parser.get()

        while e.type != 0 and (e.type != 3 or e.text != 'deletemember'):
            etext = e.text

            if e.type == 2:  # <tag>

                if etext == 'eventtime':
                    s = int(self.__get_tag_value(e)) // 1000
                    timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(s)

                elif etext == 'initiator':
                    initiator = format_account_id(self.__get_tag_value(e))

                elif etext == 'target':
                    target = format_account_id(self.__get_tag_value(e))

            e = self.__parser.get()

        # create element
        text = 'Member "%s"' % target

        if initiator and initiator != target:
            text += ' removed from chat by user "%s"' % initiator

        else:
            text += ' left chat'

        if timestamp:
            text += ' at %s' % timestamp

        element = {'type': 'system', 'text': text}
        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <topicupdate> tag
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_topicupdate(self, e):
        timestamp = None
        initiator = None
        value = None

        # process until </topicupdate>
        e = self.__parser.get()

        while e.type != 0 and (e.type != 3 or e.text != 'topicupdate'):
            etext = e.text

            if e.type == 2:  # <tag>

                if etext == 'eventtime':
                    s = int(self.__get_tag_value(e)) // 1000
                    timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(s)

                elif etext == 'initiator':
                    initiator = format_account_id(self.__get_tag_value(e))

                elif etext == 'value':
                    value = self.__get_tag_value(e)

            e = self.__parser.get()

        # create element
        text = 'Topic updated to "%s"' % value

        if initiator:
            text += '  by user "%s"' % initiator

        if timestamp:
            text += ' at %s' % timestamp

        element = {'type': 'system', 'text': text}
        self.add_element(element)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Parse <historydisclosedupdate> tag
    # @see https://docs.microsoft.com/en-us/javascript/api/botbuilder/iconversationupdate?view=botbuilder-ts-3.0#botbuilder-iconversationupdate-historydisclosed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def _parse_historydisclosedupdate(self, e):
        timestamp = None
        initiator = None
        value = None

        # process until </historydisclosedupdate>
        e = self.__parser.get()

        while e.type != 0 and (e.type != 3 or e.text != 'historydisclosedupdate'):
            etext = e.text

            if e.type == 2:  # <tag>

                if etext == 'eventtime':
                    s = int(self.__get_tag_value(e)) // 1000
                    timestamp = mobius.core.datetime.new_datetime_from_unix_timestamp(s)

                elif etext == 'initiator':
                    initiator = format_account_id(self.__get_tag_value(e))

                elif etext == 'value':
                    value = self.__get_tag_value(e)

            e = self.__parser.get()

        # create element
        if value == "True":
            text = 'History disclosed'
        else:
            text = 'History closed'

        if initiator:
            text += '  by user "%s"' % initiator

        if timestamp:
            text += ' at %s' % timestamp

        element = {'type': 'system', 'text': text}
        self.add_element(element)
