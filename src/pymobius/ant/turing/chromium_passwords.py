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
import traceback
import mobius
import mobius.core.crypt
import mobius.core.os
import mobius.core.turing
import pymobius
import pymobius.app.chromium

DPAPI_GUID = b'\xd0\x8c\x9d\xdf\x01\x15\xd1\x11\x8c\x7a\x00\xc0\x4f\xc2\x97\xeb'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve and decrypt Chromium Passwords
# @author Eduardo Aguiar
# @see https://xenarmor.com/how-to-recover-saved-passwords-google-chrome/
# @see https://github.com/mlkui/chrome-cookie-password-decryption
# @see https://gist.github.com/neonfury/a34a2aadc7c084f08cb046728cd25b54
# @see https://0x00sec.org/t/malware-development-1-password-stealers-chrome/33571
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Ant(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, item):
        self.name = 'Turing Chromium Passwords'
        self.version = '1.2'
        self.__item = item
        self.__secrets = []
        self.__urls = set()
        self.__count = 0
        self.__total = 0

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run ant
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.__secrets = []
        self.__urls = set()
        self.__count = 0
        self.__total = 0

        try:
            self.__retrieve_chromium_passwords()
        except Exception as e:
            mobius.core.logf('WRN %s %s' % (str(e), traceback.format_exc()))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_export_data</i>: Export data to mediator ant
    # @param ant Mediator ant object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_export_data(self, ant):
        pass

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_stop</i>: Terminate this ant
    # @param ant Mediator ant object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_stop(self, ant):

        # add URLs to Turing, as IE entropy hashes
        turing = mobius.core.turing.turing()
        transaction = turing.new_transaction()

        for url in self.__urls:
            h = mobius.core.encoder.hexstring(mobius.core.os.win.hash_ie_entropy(url))
            turing.set_hash('ie.entropy', h, url)

        transaction.commit()

        # write status
        mobius.core.logf(f"INF {self.__count}/{self.__total} Chromium password(s) decrypted")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Event <i>on_key</i>: Process key object
    # @param ant Mediator ant object
    # @param key Key object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def on_key(self, ant, key):

        # try to use the newly found key to decrypt secrets
        for secret in self.__secrets:

            try:
                if secret.is_found:
                    pass  # ignore secrets already found

                elif secret.type == 'dpapi.v10_master_key':
                    self.__on_key_dpapi_v10_master_key(ant, key, secret)

                elif secret.type == 'dpapi.login_data':
                    self.__on_key_dpapi_login_data(ant, key, secret)

                elif secret.type == 'v10.login_data':
                    self.__on_key_v10_login_data(ant, key, secret)

            except Exception as e:
                mobius.core.logf('WRN %s %s' % (str(e), traceback.format_exc()))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Try to decrypt DPAPI v10 master key secret
    # @param ant Ant
    # @param key Key
    # @param secret Secret
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_key_dpapi_v10_master_key(self, ant, key, secret):
        blob = secret.blob

        if key.type == 'dpapi.user' and blob.master_key_guid == key.id and not blob.is_decrypted():

            if blob.decrypt(key.value):
                mobius.core.logf(f"INF DPAPI v10 Master Key blob {blob.master_key_guid} decrypted")

                # create ID for this key
                h = mobius.core.crypt.hash("md5")
                h.update(secret.source.encode("utf-8"))
                key_id = h.get_hex_digest().upper()

                # create key object
                k = pymobius.Data()
                k.type = 'chromium.v10'
                k.id = key_id
                k.value = blob.plain_text
                k.encrypted_value = key.value

                k.metadata = []
                k.metadata.append(("Source", secret.source))
                k.metadata.append(("Application", secret.app_name))
                k.metadata.append(("DPAPI master key GUID", blob.master_key_guid))
                k.metadata.append(("DPAPI blob description", blob.description))
                ant.add_key(k)

                secret.is_found = True

            else:
                mobius.core.logf(f"WRN DPAPI v10 Master Key blob {blob.master_key_guid} could not be decrypted")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Try to decrypt DPAPI login data
    # @param ant Ant
    # @param key Key
    # @param secret Secret
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_key_dpapi_login_data(self, ant, key, secret):
        blob = secret.blob

        if key.type == 'dpapi.user' and blob.master_key_guid == key.id and not blob.is_decrypted():

            if blob.decrypt(key.value):
                mobius.core.logf(f"INF DPAPI login data blob {blob.master_key_guid} decrypted")
                password = blob.plain_text.rstrip(b'\0')

                if password and password[-1] > 16:
                    url = secret.p.origin_url
                    uri = mobius.core.io.uri(url)
                    domain = uri.get_host()

                    p = pymobius.Data()
                    p.type = f'net.http/{domain}'
                    p.value = password.decode('utf-8')
                    p.description = f"Web password. URL: {secret.p.origin_url}"

                    p.metadata = []
                    p.metadata.append(("Source", secret.p.source))
                    p.metadata.append(("Action URL", secret.p.action_url))
                    p.metadata.append(("URL", secret.p.origin_url))
                    p.metadata.append(("Domain", domain))
                    p.metadata.append(("User ID", secret.p.username))
                    p.metadata.append(("Sign On Realm", secret.p.signon_realm))
                    p.metadata.append(("Date Created", secret.p.date_created))
                    p.metadata.append(("Date Last Used", secret.p.date_last_used))
                    p.metadata.append(("Date Password Modified", secret.p.date_password_modified))
                    p.metadata.append(("Times Used", secret.p.times_used))
                    p.metadata.append(("Profile path", secret.profile.path))
                    p.metadata.append(("Application", secret.profile.app_name))

                    self.__count += 1
                    ant.add_password(p)

                secret.is_found = True

            else:
                mobius.core.logf(f"WRN DPAPI login data blob {blob.master_key_guid} could not be decrypted")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Try to decrypt v10 login data
    # @param ant Ant
    # @param key Key
    # @param secret Secret
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_key_v10_login_data(self, ant, key, secret):

        if key.type == 'chromium.v10':
            payload = secret.data
            version = payload[:3]
            iv = payload[3:15]
            ciphertext = payload[15:-16]
            tag = payload[-16:]

            cipher = mobius.core.crypt.new_cipher_gcm('aes', key.value, iv)
            plaintext = cipher.decrypt(ciphertext)

            if cipher.check_tag(tag):
                mobius.core.logf(f"INF login data {version} decrypted")

                url = secret.p.origin_url
                uri = mobius.core.io.uri(url)
                domain = uri.get_host()

                p = pymobius.Data()
                p.type = f'net.http/{domain}'
                p.value = plaintext.decode('utf-8')
                p.description = f"Web password. URL: {secret.p.origin_url}"

                p.metadata = []
                p.metadata.append(("Source", secret.p.source))
                p.metadata.append(("Record ID", secret.p.id))
                p.metadata.append(("Domain", uri.get_host()))
                p.metadata.append(("Action URL", secret.p.action_url))
                p.metadata.append(("URL", secret.p.origin_url))
                p.metadata.append(("User ID", secret.p.username))
                p.metadata.append(("Sign On Realm", secret.p.signon_realm))
                p.metadata.append(("Date Created", secret.p.date_created))
                p.metadata.append(("Date Last Used", secret.p.date_last_used))
                p.metadata.append(("Date Password Modified", secret.p.date_password_modified))
                p.metadata.append(("Times Used", secret.p.times_used))
                p.metadata.append(("Profile path", secret.profile.path))
                p.metadata.append(("Application", secret.profile.app_name))

                self.__count += 1
                ant.add_password(p)

                secret.is_found = True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve Chromium browsers passwords
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_chromium_passwords(self):
        model = pymobius.app.chromium.model(self.__item)

        self.__retrieve_chromium_v10_keys(model)
        self.__retrieve_chromium_from_profiles(model)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve v10 encryption keys from Chromium browsers
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_chromium_v10_keys(self, model):
        v10_master_keys = {}

        # get unique v10 encryption keys
        for profile in model.get_profiles():
            try:
                v10_encrypted_key = profile.local_state.get('v10_encrypted_key')

                if v10_encrypted_key:
                    secret = pymobius.Data()
                    secret.source = profile.local_state_path
                    secret.app_name = profile.app_name
                    v10_master_keys[v10_encrypted_key] = secret

            except Exception as e:
                mobius.core.logf('WRN %s %s' % (str(e), traceback.format_exc()))

        # create a secret object for each v10 encryption key found
        for mk, secret in v10_master_keys.items():
            secret.type = 'dpapi.v10_master_key'
            secret.blob = mobius.core.os.win.dpapi.blob(mk[5:])
            secret.encrypted_value = secret.blob
            secret.is_found = False
            self.__secrets.append(secret)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve Chromium browsers passwords from profiles
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_chromium_from_profiles(self, model):

        for profile in model.get_profiles():
            try:
                self.__retrieve_chromium_passwords_from_profile(profile)
            except Exception as e:
                mobius.core.logf('WRN %s %s' % (str(e), traceback.format_exc()))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Retrieve Chromium passwords from profile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_chromium_passwords_from_profile(self, profile):

        for p in profile.get_stored_passwords():
            self.__urls.add(p.origin_url)

            if p.has_password:

                if DPAPI_GUID in p.password:  # Up to version 79
                    secret = pymobius.Data()
                    secret.type = 'dpapi.login_data'
                    secret.blob = mobius.core.os.win.dpapi.blob(p.password)
                    secret.profile = profile
                    secret.p = p
                    secret.is_found = False
                    self.__secrets.append(secret)
                    self.__total += 1

                elif p.password.startswith(b'v10'):  # Version 80 and up
                    secret = pymobius.Data()
                    secret.type = 'v10.login_data'
                    secret.data = p.password
                    secret.profile = profile
                    secret.p = p
                    secret.is_found = False
                    self.__secrets.append(secret)
                    self.__total += 1

                else:
                    mobius.core.logf('DEV Unknown password value:\n' + pymobius.dump(p.password))
