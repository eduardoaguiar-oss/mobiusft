#!/usr/bin/env python

import mobius
import binascii

g_count = 0
g_errors = 0
g_unittests = 0

class unittest (object):

  def __init__ (self, title):
    self.__title = title
    self.__count = 0
    self.__errors = 0
    global g_unittests
    g_unittests += 1

  def end (self):
    dots = 0 if len (self.__title) > 60 else 60 - len (self.__title)
    print ("\033[1;39m%s\033[0m %s" % (self.__title, '.' * dots), end='')

    if (self.__errors == 0):
      print ("\033[1;32mOK", end='')
    else:
      print ("\033[1;31mERROR", end='')
  
    print ("\033[0m(%d/%d)" % (self.__count - self.__errors, self.__count))

  def assert_equal (self, v1, v2):
    global g_count, g_errors
    self.__count += 1
    g_count += 1
    
    if v1 != v2:
      print ("\033[1;31mERROR:\033[0m %s == %s failed" % (v1, v2))
      self.__errors += 1
      g_errors += 1

  def assert_true (self, v1):
    global g_count, g_errors
    self.__count += 1
    g_count += 1
    
    if not v1:
      print ("\033[1;31mERROR:\033[0m %s is false" % v1)
      self.__errors += 1
      g_errors += 1

  @staticmethod
  def final_summary ():
    global g_unittests, g_count, g_errors
    print ()
    print ("unittests:", g_unittests)
    print ("tests    :", g_count)
    print ("errors   :", g_errors)

def string_to_hex (s):
  return binascii.hexlify (s)

def unittest_core_application ():
  app = mobius.core.application ()

  test = unittest ("mobius.core.application")
  test.assert_equal (app.name, "Mobius Forensic Toolkit")
  test.assert_true (len (app.version) > 0)
  test.assert_true (len (app.title) > 0)
  test.assert_true (len (app.copyright) > 0)

  test.end ()

def unittest_crypt_cipher_3des ():

  test = unittest ("mobius.crypt.cipher_3des")
  c = mobius.crypt.cipher ("3des", b"\x01\x23\x45\x67\x89\xab\xcd\xef\x23\x45\x67\x89\xab\xcd\xef\x01\x01\x23\x45\x67\x89\xab\xcd\xef")

  text = b"\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51"

  text2 = c.encrypt (text)
  test.assert_equal (string_to_hex (text2), b"06ede3d82884090aff322c19f0518486730576972a666e58b6c88cf107340d3d")
	  
  text2 = c.decrypt (text2)
  test.assert_equal (string_to_hex (text2), string_to_hex (text))
	  
  c = mobius.crypt.cipher ("3des", b"\x01\x23\x45\x67\x89\xab\xcd\xef\x23\x45\x67\x89\xab\xcd\xef\x01\x45\x67\x89\xab\xcd\xef\x01\x23")

  text2 = c.encrypt (text)
  test.assert_equal (string_to_hex (text2), b"714772f339841d34267fcc4bd2949cc3ee11c22a576a303876183f99c0b6de87")
	  
  text2 = c.decrypt (text2)
  test.assert_equal (string_to_hex (text2), string_to_hex (text))

  test.end ()

def unittest_crypt_cipher_aes ():

  test = unittest ("mobius.crypt.cipher_aes")
	  
  plaintext = b'\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51\x30\xc8\x1c\x46\xa3\x5c\xe4\x11\xe5\xfb\xc1\x19\x1a\x0a\x52\xef\xf6\x9f\x24\x45\xdf\x4f\x9b\x17\xad\x2b\x41\x7b\xe6\x6c\x37\x10'

  iv = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f'

  # AES128 - mode ECB
  ciphertext = b'\x3a\xd7\x7b\xb4\x0d\x7a\x36\x60\xa8\x9e\xca\xf3\x24\x66\xef\x97\xf5\xd3\xd5\x85\x03\xb9\x69\x9d\xe7\x85\x89\x5a\x96\xfd\xba\xaf\x43\xb1\xcd\x7f\x59\x8e\xce\x23\x88\x1b\x00\xe3\xed\x03\x06\x88\x7b\x0c\x78\x5e\x27\xe8\xad\x3f\x82\x23\x20\x71\x04\x72\x5d\xd4'
	  
  aes1 = mobius.crypt.cipher ("aes", b'\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c')

  test.assert_equal (aes1.encrypt (plaintext), ciphertext)
  test.assert_equal (aes1.decrypt (ciphertext), plaintext)

  # AES192 - mode ECB
  ciphertext = b'\xbd\x33\x4f\x1d\x6e\x45\xf2\x5f\xf7\x12\xa2\x14\x57\x1f\xa5\xcc\x97\x41\x04\x84\x6d\x0a\xd3\xad\x77\x34\xec\xb3\xec\xee\x4e\xef\xef\x7a\xfd\x22\x70\xe2\xe6\x0a\xdc\xe0\xba\x2f\xac\xe6\x44\x4e\x9a\x4b\x41\xba\x73\x8d\x6c\x72\xfb\x16\x69\x16\x03\xc1\x8e\x0e'

  aes2 = mobius.crypt.cipher ("aes", b'\x8e\x73\xb0\xf7\xda\x0e\x64\x52\xc8\x10\xf3\x2b\x80\x90\x79\xe5\x62\xf8\xea\xd2\x52\x2c\x6b\x7b')

  test.assert_equal (aes2.encrypt (plaintext), ciphertext)
  test.assert_equal (aes2.decrypt (ciphertext), plaintext)

  # AES256 - mode ECB
  ciphertext = b'\xf3\xee\xd1\xbd\xb5\xd2\xa0\x3c\x06\x4b\x5a\x7e\x3d\xb1\x81\xf8\x59\x1c\xcb\x10\xd4\x10\xed\x26\xdc\x5b\xa7\x4a\x31\x36\x28\x70\xb6\xed\x21\xb9\x9c\xa6\xf4\xf9\xf1\x53\xe7\xb1\xbe\xaf\xed\x1d\x23\x30\x4b\x7a\x39\xf9\xf3\xff\x06\x7d\x8d\x8f\x9e\x24\xec\xc7'
	  
  aes3 = mobius.crypt.cipher ("aes", b'\x60\x3d\xeb\x10\x15\xca\x71\xbe\x2b\x73\xae\xf0\x85\x7d\x77\x81\x1f\x35\x2c\x07\x3b\x61\x08\xd7\x2d\x98\x10\xa3\x09\x14\xdf\xf4')

  test.assert_equal (aes3.encrypt (plaintext), ciphertext)
  test.assert_equal (aes3.decrypt (ciphertext), plaintext)
	  
  # AES128 - mode CBC
  ciphertext = b'\x76\x49\xab\xac\x81\x19\xb2\x46\xce\xe9\x8e\x9b\x12\xe9\x19\x7d\x50\x86\xcb\x9b\x50\x72\x19\xee\x95\xdb\x11\x3a\x91\x76\x78\xb2\x73\xbe\xd6\xb8\xe3\xc1\x74\x3b\x71\x16\xe6\x9e\x22\x22\x95\x16\x3f\xf1\xca\xa1\x68\x1f\xac\x09\x12\x0e\xca\x30\x75\x86\xe1\xa7'

  aes4a = mobius.crypt.cipher ("aes", b'\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c', 'cbc', iv)

  test.assert_equal (aes4a.encrypt (plaintext), ciphertext)
	  
  aes4b = mobius.crypt.cipher ("aes", b'\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c', 'cbc', iv)

  test.assert_equal (aes4b.decrypt (ciphertext), plaintext)
  
  test.end ()

def unittest_crypt_cipher_des ():

  test = unittest ("mobius.crypt.cipher_des")
  c = mobius.crypt.cipher ("des", b"\x13\x34\x57\x79\x9b\xbc\xdf\xf1")

  text = b"\x01\x23\x45\x67\x89\xab\xcd\xef"
  test.assert_equal (string_to_hex (text), b"0123456789abcdef")
	  
  text2 = c.encrypt (text)
  test.assert_equal (string_to_hex (text2), b"85e813540f0ab405")
	  
  text2 = c.decrypt (text2)
  test.assert_equal (string_to_hex (text2), b"0123456789abcdef")
  
  # LM password "ABCD123"
  c = mobius.crypt.cipher ("des", b"ABCD123")

  text = b"KGS!@#$%"
  test.assert_equal (string_to_hex (text), b"4b47532140232425")

  text2 = c.encrypt (text)
  test.assert_equal (string_to_hex (text2), b"6f87cd328120cc55")
  
  text2 = c.decrypt (text2)
  test.assert_equal (string_to_hex (text2), b"4b47532140232425")
  
  # DES - mode CBC
  c = mobius.crypt.cipher ("des", b"ABCDE12", 'cbc', b'\x00\x00\x00\x00\x00\x00\x00\x00')
  
  text = b"KGS!@#$%"
  test.assert_equal (string_to_hex (text), b"4b47532140232425")

  text2 = c.encrypt (text)
  test.assert_equal (string_to_hex (text2), b"722ac01404a75156")
  
  c.reset ()
  text2 = c.decrypt (text2)
  test.assert_equal (string_to_hex (text2), b"4b47532140232425")
  
  test.end ()

def unittest_crypt_cipher_rc4 ():

  # key="Key", Plaintext="Plaintext"
  test = unittest ("mobius.crypt.cipher_rc4")
  c = mobius.crypt.cipher ("rc4", b"Key")

  text = b"Plaintext"
  test.assert_equal (text, b"Plaintext")

  text2 = c.encrypt (text)
  test.assert_equal (string_to_hex (text2), b"bbf316e8d940af0ad3")

  c.reset ()
  text2 = c.decrypt (text2)
  test.assert_equal (text, text2)

  # key="Secret", Plaintext="Attack at dawn"
  c = mobius.crypt.cipher ("rc4", b"Secret")

  text = b"Attack at dawn"
  test.assert_equal (text, b"Attack at dawn")

  text2 = c.encrypt (text)
  test.assert_equal (string_to_hex (text2), b"45a01f645fc35b383552544b9bf5")

  c.reset ()
  text2 = c.decrypt (text2)
  test.assert_equal (text, text2)

  test.end ()

def unittest_crypt_cipher_rot13 ():

  # nowhere <-> abjurer
  test = unittest ("mobius.crypt.cipher_rot13")
  c = mobius.crypt.cipher ("rot13")

  text = b"nowhere"
  test.assert_equal (text, b"nowhere")

  text2 = c.encrypt (text)
  test.assert_equal (text2, b"abjurer")

  # ARES <-> NERF
  c = mobius.crypt.cipher ("rot13")

  text = b"ARES"
  test.assert_equal (text, b"ARES")

  text2 = c.encrypt (text)
  test.assert_equal (text2, b"NERF")

  test.end ()

def unittest_crypt_cipher_zip ():

  # key="Key", Plaintext="Plaintext"
  test = unittest ("mobius.crypt.cipher_zip")
  c = mobius.crypt.cipher ("zip", b"Key")

  text = b"Plaintext"
  test.assert_equal (text, b"Plaintext")

  text2 = c.encrypt (text)
  test.assert_equal (string_to_hex (text2), b"fe1995e4fe54a8c6f3")

  c.reset ()
  text2 = c.decrypt (text2)
  test.assert_equal (text, text2)

  # key="Secret", Plaintext="Attack at dawn"
  c = mobius.crypt.cipher ("zip", b"Secret")

  text = b"Attack at dawn"
  test.assert_equal (text, b"Attack at dawn")

  text2 = c.encrypt (text)
  test.assert_equal (string_to_hex (text2), b"7595da02f5ec5c2c78755fd4069e")

  c.reset ()
  text2 = c.decrypt (text2)
  test.assert_equal (text, text2)

  test.end ()

def unittest_crypt_hash_adler32 ():
  test = unittest ("mobius.crypt.hash_adler32")

  h1 = mobius.crypt.hash ("adler32")
  test.assert_equal (h1.get_hex_digest (), "00000001")

  h2 = mobius.crypt.hash ("adler32")
  h2.update (b'abc')
  test.assert_equal (h2.get_hex_digest (), "024d0127")

  h2.update (b'd')
  test.assert_equal (h2.get_hex_digest (), "03d8018b")

  h3 = mobius.crypt.hash ("adler32")
  h3.update (b'abcd')
  test.assert_equal (h3.get_hex_digest (), h2.get_hex_digest ())

  test.end ()

def unittest_crypt_hash_crc32 ():
  test = unittest ("mobius.crypt.hash_crc32")

  h1 = mobius.crypt.hash ("crc32")
  test.assert_equal (h1.get_hex_digest (), "00000000")

  h2 = mobius.crypt.hash ("crc32")
  h2.update (b'abc')
  test.assert_equal (h2.get_hex_digest (), "352441c2")

  h2.update (b'd')
  test.assert_equal (h2.get_hex_digest (), "ed82cd11")

  h3 = mobius.crypt.hash ("crc32")
  h3.update (b'abcd')
  test.assert_equal (h3.get_hex_digest (), h2.get_hex_digest ())

  test.end ()

def unittest_crypt_hash_md4 ():
  test = unittest ("mobius.crypt.hash_md4")

  # RFC 1321 - section A.5
  h1 = mobius.crypt.hash ("md4")
  test.assert_equal (h1.get_hex_digest (), "31d6cfe0d16ae931b73c59d7e0c089c0")

  h2 = mobius.crypt.hash ("md4")
  h2.update (b"a")
  test.assert_equal (h2.get_hex_digest (), "bde52cb31de33e46245e05fbdbd6fb24")

  h3 = mobius.crypt.hash ("md4")
  h3.update (b"abc")
  test.assert_equal (h3.get_hex_digest (), "a448017aaf21d8525fc10ae87aa6729d")

  h4 = mobius.crypt.hash ("md4")
  h4.update (b"message digest")
  test.assert_equal (h4.get_hex_digest (), "d9130a8164549fe818874806e1c7014b")

  h5 = mobius.crypt.hash ("md4")
  h5.update (b"abcdefghijklmnopqrstuvwxyz")
  test.assert_equal (h5.get_hex_digest (), "d79e1c308aa5bbcdeea8ed63df412da9")
  
  h6 = mobius.crypt.hash ("md4")
  h6.update (b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789")
  test.assert_equal (h6.get_hex_digest (), "043f8582f241db351ce627e153e7f0e4")
  
  h7 = mobius.crypt.hash ("md4")
  h7.update (b"12345678901234567890123456789012345678901234567890123456789012345678901234567890")
  test.assert_equal (h7.get_hex_digest (), "e33b4ddc9c38f2199c3e7b164fcc0536")
  
  test.end ()

def unittest_crypt_hash_md5 ():
  test = unittest ("mobius.crypt.hash_md5")

  # RFC 1321 - section A.5
  h1 = mobius.crypt.hash ("md5")
  test.assert_equal (h1.get_hex_digest (), "d41d8cd98f00b204e9800998ecf8427e")

  h2 = mobius.crypt.hash ("md5")
  h2.update (b"a")
  test.assert_equal (h2.get_hex_digest (), "0cc175b9c0f1b6a831c399e269772661")

  h3 = mobius.crypt.hash ("md5")
  h3.update (b"abc")
  test.assert_equal (h3.get_hex_digest (), "900150983cd24fb0d6963f7d28e17f72")

  h4 = mobius.crypt.hash ("md5")
  h4.update (b"message digest")
  test.assert_equal (h4.get_hex_digest (), "f96b697d7cb7938d525a2f31aaf161d0")

  h5 = mobius.crypt.hash ("md5")
  h5.update (b"abcdefghijklmnopqrstuvwxyz")
  test.assert_equal (h5.get_hex_digest (), "c3fcd3d76192e4007dfb496cca67e13b")
  
  h6 = mobius.crypt.hash ("md5")
  h6.update (b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789")
  test.assert_equal (h6.get_hex_digest (), "d174ab98d277d9f5a5611c2c9f419d9f")
  
  h7 = mobius.crypt.hash ("md5")
  h7.update (b"12345678901234567890123456789012345678901234567890123456789012345678901234567890")
  test.assert_equal (h7.get_hex_digest (), "57edf4a22be3c955ac49da2e2107b67a")
  
  h8 = mobius.crypt.hash ("md5")
  h8.update (b'\0' * 1000)
  test.assert_equal (h8.get_hex_digest (), "ede3d3b685b4e137ba4cb2521329a75e")

  test.end ()

def unittest_crypt_hash_sha1 ():
  test = unittest ("mobius.crypt.hash_sha1")

  # RFC 3174 - section 7.3
  h1 = mobius.crypt.hash ("sha1")
  test.assert_equal (h1.get_hex_digest (), "da39a3ee5e6b4b0d3255bfef95601890afd80709")

  h2 = mobius.crypt.hash ("sha1")
  h2.update (b"abc")
  test.assert_equal (h2.get_hex_digest (), "a9993e364706816aba3e25717850c26c9cd0d89d")

  h3 = mobius.crypt.hash ("sha1")
  h3.update (b"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")
  test.assert_equal (h3.get_hex_digest (), "84983e441c3bd26ebaae4aa1f95129e5e54670f1")

  h4 = mobius.crypt.hash ("sha1")
  
  for i in range (1000000):
    h4.update (b'a')

  test.assert_equal (h4.get_hex_digest (), "34aa973cd4c4daa4f61eeb2bdbad27316534016f")

  h5 = mobius.crypt.hash ("sha1")
  
  for i in range (20):
    h5.update (b"01234567012345670123456701234567")

  test.assert_equal (h5.get_hex_digest (), "dea356a2cddd90c7a7ecedc5ebb563934f460452")
  
  test.end ()

def unittest_crypt_hash_sha2_224 ():
  test = unittest ("mobius.crypt.hash_sha2_224")

  h1 = mobius.crypt.hash ("sha2_224")
  test.assert_equal (h1.get_hex_digest (), "d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f")

  h2 = mobius.crypt.hash ("sha2_224")
  h2.update (b"abc")
  test.assert_equal (h2.get_hex_digest (), "23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7")

  h3 = mobius.crypt.hash ("sha2_224")
  h3.update (b"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")
  test.assert_equal (h3.get_hex_digest (), "75388b16512776cc5dba5da1fd890150b0c6455cb4f58b1952522525")
  
  test.end ()

def unittest_crypt_hash_sha2_256 ():
  test = unittest ("mobius.crypt.hash_sha2_256")

  h1 = mobius.crypt.hash ("sha2_256")
  test.assert_equal (h1.get_hex_digest (), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")

  h2 = mobius.crypt.hash ("sha2_256")
  h2.update (b"abc")
  test.assert_equal (h2.get_hex_digest (), "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")

  h3 = mobius.crypt.hash ("sha2_256")
  h3.update (b"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")
  test.assert_equal (h3.get_hex_digest (), "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1")
  
  test.end ()

def unittest_crypt_hash_sha2_384 ():
  test = unittest ("mobius.crypt.hash_sha2_384")

  h1 = mobius.crypt.hash ("sha2_384")
  test.assert_equal (h1.get_hex_digest (), "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b")

  h2 = mobius.crypt.hash ("sha2_384")
  h2.update (b"abc")
  test.assert_equal (h2.get_hex_digest (), "cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7")

  h3 = mobius.crypt.hash ("sha2_384")
  h3.update (b"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu")
  test.assert_equal (h3.get_hex_digest (), "09330c33f71147e83d192fc782cd1b4753111b173b3b05d22fa08086e3b0f712fcc7c71a557e2db966c3e9fa91746039")
  
  test.end ()

def unittest_crypt_hash_sha2_512 ():
  test = unittest ("mobius.crypt.hash_sha2_512")

  h1 = mobius.crypt.hash ("sha2_512")
  test.assert_equal (h1.get_hex_digest (), "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e")

  h2 = mobius.crypt.hash ("sha2_512")
  h2.update (b"abc")
  test.assert_equal (h2.get_hex_digest (), "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f")

  h3 = mobius.crypt.hash ("sha2_512")
  h3.update (b"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu")
  test.assert_equal (h3.get_hex_digest (), "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909")
  
  test.end ()

def unittest_crypt_hash_sha2_512_224 ():
  test = unittest ("mobius.crypt.hash_sha2_512_224")

  h1 = mobius.crypt.hash ("sha2_512_224")
  test.assert_equal (h1.get_hex_digest (), "6ed0dd02806fa89e25de060c19d3ac86cabb87d6a0ddd05c333b84f4")

  h2 = mobius.crypt.hash ("sha2_512_224")
  h2.update (b"abc")
  test.assert_equal (h2.get_hex_digest (), "4634270f707b6a54daae7530460842e20e37ed265ceee9a43e8924aa")

  h3 = mobius.crypt.hash ("sha2_512_224")
  h3.update (b"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu")
  test.assert_equal (h3.get_hex_digest (), "23fec5bb94d60b23308192640b0c453335d664734fe40e7268674af9")
  
  test.end ()

def unittest_crypt_hash_sha2_512_256 ():
  test = unittest ("mobius.crypt.hash_sha2_512_256")

  h1 = mobius.crypt.hash ("sha2_512_256")
  test.assert_equal (h1.get_hex_digest (), "c672b8d1ef56ed28ab87c3622c5114069bdd3ad7b8f9737498d0c01ecef0967a")

  h2 = mobius.crypt.hash ("sha2_512_256")
  h2.update (b"abc")
  test.assert_equal (h2.get_hex_digest (), "53048e2681941ef99b2e29b76b4c7dabe4c2d0c634fc6d46e0e2f13107e7af23")

  h3 = mobius.crypt.hash ("sha2_512_256")
  h3.update (b"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu")
  test.assert_equal (h3.get_hex_digest (), "3928e184fb8690f840da3988121d31be65cb9d3ef83ee6146feac861e19b563a")
  
  test.end ()

def unittest_crypt_hash_zip ():
  h1 = mobius.crypt.hash ("zip")
  h2 = mobius.crypt.hash ("zip")
  h3 = mobius.crypt.hash ("zip")

  test = unittest ("mobius.crypt.hash_zip")
  test.assert_equal (h1.get_hex_digest (), "123456782345678934567890")
  test.assert_equal (h2.get_hex_digest (), "123456782345678934567890")
  test.assert_equal (h3.get_hex_digest (), "123456782345678934567890")

  h2.update (b"abc")
  test.assert_equal (h2.get_hex_digest (), "5dd2af4d589d03b43cf5ffa4")

  h2.update (b'd')
  test.assert_equal (h2.get_hex_digest (), "42ef4ac38d167254428e6d93")
  
  h3.update (b"abcd")
  test.assert_equal (h3.get_hex_digest (), h2.get_hex_digest ())

  test.end ()

def unittest_crypt_hmac ():
  test = unittest ("mobius.crypt.hmac")
  
  # RFC 2202 - section 2
  h1 = mobius.crypt.hmac (b'\x0b' * 16)
  h1.update (b"Hi There")
  test.assert_equal (h1.get_hex_digest (), "9294727a3638bb1c13f48ef8158bfc9d")

  h2 = mobius.crypt.hmac (b"Jefe")
  h2.update (b"what do ya want for nothing?")
  test.assert_equal (h2.get_hex_digest (), "750c783e6ab0b503eaa86e310a5db738")

  h3 = mobius.crypt.hmac (b'\xaa' * 16)
  h3.update (b'\xdd' * 50)
  test.assert_equal (h3.get_hex_digest (), "56be34521d144c88dbb8c733f0e8b3f6")

  h4 = mobius.crypt.hmac (b"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19")
  h4.update (b'\xcd' * 50)
  test.assert_equal (h4.get_hex_digest (), "697eaf0aca3a3aea3a75164746ffaa79")

  h5 = mobius.crypt.hmac (b'\x0c' * 16)
  h5.update (b"Test With Truncation")
  test.assert_equal (h5.get_hex_digest (), "56461ef2342edc00f9bab995690efd4c")

  h6 = mobius.crypt.hmac (b'\xaa' * 80)
  h6.update (b"Test Using Larger Than Block-Size Key - Hash Key First")
  test.assert_equal (h6.get_hex_digest (), "6b1ab7fe4bd7bf8f0b62e6ce61b9d0cd")

  h7 = mobius.crypt.hmac (b'\xaa' * 80)
  h7.update (b"Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data")
  test.assert_equal (h7.get_hex_digest (), "6f630fad67cda0ee1fb1f562db3aa53e")  

  # RFC 2202 - section 3
  h8 = mobius.crypt.hmac (b'\x0b' * 20, "sha1")
  h8.update (b"Hi There")
  test.assert_equal (h8.get_hex_digest (), "b617318655057264e28bc0b6fb378c8ef146be00")

  h9 = mobius.crypt.hmac (b"Jefe", "sha1")
  h9.update (b"what do ya want for nothing?")
  test.assert_equal (h9.get_hex_digest (), "effcdf6ae5eb2fa2d27416d5f184df9c259a7c79")

  h10 = mobius.crypt.hmac (b'\xaa' * 20, "sha1")
  h10.update (b'\xdd' * 50)
  test.assert_equal (h10.get_hex_digest (), "125d7342b9ac11cd91a39af48aa17b4f63f175d3")

  h11 = mobius.crypt.hmac (b"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19", "sha1")
  h11.update (b'\xcd' * 50)
  test.assert_equal (h11.get_hex_digest (), "4c9007f4026250c6bc8414f9bf50c86c2d7235da")

  h12 = mobius.crypt.hmac (b'\x0c' * 20, "sha1")
  h12.update (b"Test With Truncation")
  test.assert_equal (h12.get_hex_digest (), "4c1a03424b55e07fe7f27be1d58bb9324a9a5a04")

  h13 = mobius.crypt.hmac (b'\xaa' * 80, "sha1")
  h13.update (b"Test Using Larger Than Block-Size Key - Hash Key First")
  test.assert_equal (h13.get_hex_digest (), "aa4ae5e15272d00e95705637ce8a3b55ed402112")

  h14 = mobius.crypt.hmac (b'\xaa' * 80, "sha1")
  h14.update (b"Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data")
  test.assert_equal (h14.get_hex_digest (), "e8e99d0f45237d786d6bbaa7965c7808bbff1a91")

  # RFC 4231 - section 4.2
  h15a = mobius.crypt.hmac (b'\x0b' * 20, "sha2_224")
  h15a.update (b"Hi There")
  test.assert_equal (h15a.get_hex_digest (), "896fb1128abbdf196832107cd49df33f47b4b1169912ba4f53684b22")

  h15b = mobius.crypt.hmac (b'\x0b' * 20, "sha2_256")
  h15b.update (b"Hi There")
  test.assert_equal (h15b.get_hex_digest (), "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7")

  h15c = mobius.crypt.hmac  (b'\x0b' * 20, "sha2_384")
  h15c.update (b"Hi There")
  test.assert_equal (h15c.get_hex_digest (), "afd03944d84895626b0825f4ab46907f15f9dadbe4101ec682aa034c7cebc59cfaea9ea9076ede7f4af152e8b2fa9cb6")

  h15d = mobius.crypt.hmac (b'\x0b' * 20, "sha2_512")
  h15d.update (b"Hi There")
  test.assert_equal (h15d.get_hex_digest (), "87aa7cdea5ef619d4ff0b4241a1d6cb02379f4e2ce4ec2787ad0b30545e17cdedaa833b7d6b8a702038b274eaea3f4e4be9d914eeb61f1702e696c203a126854")

  # RFC 4231 - section 4.3
  h16a = mobius.crypt.hmac (b"Jefe", "sha2_224")
  h16a.update (b"what do ya want for nothing?")
  test.assert_equal (h16a.get_hex_digest (), "a30e01098bc6dbbf45690f3a7e9e6d0f8bbea2a39e6148008fd05e44")

  h16b = mobius.crypt.hmac (b"Jefe", "sha2_256")
  h16b.update (b"what do ya want for nothing?")
  test.assert_equal (h16b.get_hex_digest (), "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843")

  h16c = mobius.crypt.hmac (b"Jefe", "sha2_384")
  h16c.update (b"what do ya want for nothing?")
  test.assert_equal (h16c.get_hex_digest (), "af45d2e376484031617f78d2b58a6b1b9c7ef464f5a01b47e42ec3736322445e8e2240ca5e69e2c78b3239ecfab21649")

  h16d = mobius.crypt.hmac (b"Jefe", "sha2_512")
  h16d.update (b"what do ya want for nothing?")
  test.assert_equal (h16d.get_hex_digest (), "164b7a7bfcf819e2e395fbe73b56e0a387bd64222e831fd610270cd7ea2505549758bf75c05a994a6d034f65f8f0e6fdcaeab1a34d4a6b4b636e070a38bce737")

  # RFC 4231 - section 4.4
  h17a = mobius.crypt.hmac (b'\xaa' * 20, "sha2_224")
  h17a.update (b'\xdd' * 50)
  test.assert_equal (h17a.get_hex_digest (), "7fb3cb3588c6c1f6ffa9694d7d6ad2649365b0c1f65d69d1ec8333ea")

  h17b = mobius.crypt.hmac (b'\xaa' * 20, "sha2_256")
  h17b.update (b'\xdd' * 50)
  test.assert_equal (h17b.get_hex_digest (), "773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe")

  h17c = mobius.crypt.hmac (b'\xaa' * 20, "sha2_384")
  h17c.update (b'\xdd' * 50)
  test.assert_equal (h17c.get_hex_digest (), "88062608d3e6ad8a0aa2ace014c8a86f0aa635d947ac9febe83ef4e55966144b2a5ab39dc13814b94e3ab6e101a34f27")

  h17d = mobius.crypt.hmac (b'\xaa' * 20, "sha2_512")
  h17d.update (b'\xdd' * 50)
  test.assert_equal (h17d.get_hex_digest (), "fa73b0089d56a284efb0f0756c890be9b1b5dbdd8ee81a3655f83e33b2279d39bf3e848279a722c806b485a47e67c807b946a337bee8942674278859e13292fb")

  # RFC 4231 - section 4.5
  k18 = b"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19"

  h18a = mobius.crypt.hmac (k18, "sha2_224")
  h18a.update (b'\xcd' * 50)
  test.assert_equal (h18a.get_hex_digest (), "6c11506874013cac6a2abc1bb382627cec6a90d86efc012de7afec5a")

  h18b = mobius.crypt.hmac (k18, "sha2_256")
  h18b.update (b'\xcd' * 50)
  test.assert_equal (h18b.get_hex_digest (), "82558a389a443c0ea4cc819899f2083a85f0faa3e578f8077a2e3ff46729665b")

  h18c = mobius.crypt.hmac (k18, "sha2_384")
  h18c.update (b'\xcd' * 50)
  test.assert_equal (h18c.get_hex_digest (), "3e8a69b7783c25851933ab6290af6ca77a9981480850009cc5577c6e1f573b4e6801dd23c4a7d679ccf8a386c674cffb")

  h18d = mobius.crypt.hmac (k18, "sha2_512")
  h18d.update (b'\xcd' * 50)
  test.assert_equal (h18d.get_hex_digest (), "b0ba465637458c6990e5a8c5f61d4af7e576d97ff94b872de76f8050361ee3dba91ca5c11aa25eb4d679275cc5788063a5f19741120c4f2de2adebeb10a298dd")

  # RFC 4231 - section 4.7
  k19 = b'\xaa' * 131
  data19 = b"Test Using Larger Than Block-Size Key - Hash Key First"

  h19a = mobius.crypt.hmac (k19, "sha2_224")
  h19a.update (data19)
  test.assert_equal (h19a.get_hex_digest (), "95e9a0db962095adaebe9b2d6f0dbce2d499f112f2d2b7273fa6870e")

  h19b = mobius.crypt.hmac (k19, "sha2_256")
  h19b.update (data19)
  test.assert_equal (h19b.get_hex_digest (), "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54")

  h19c = mobius.crypt.hmac (k19, "sha2_384")
  h19c.update (data19)
  test.assert_equal (h19c.get_hex_digest (), "4ece084485813e9088d2c63a041bc5b44f9ef1012a2b588f3cd11f05033ac4c60c2ef6ab4030fe8296248df163f44952")

  h19d = mobius.crypt.hmac (k19, "sha2_512")
  h19d.update (data19)
  test.assert_equal (h19d.get_hex_digest (), "80b24263c7c1a3ebb71493c1dd7be8b49b46d1f41b4aeec1121b013783f8f3526b56d037e05f2598bd0fd2215d6a1e5295e64f73f63f0aec8b915a985d786598")

  # RFC 4231 - section 4.8
  k20 = k19
  data20 = b"This is a test using a larger than block-size key and a larger than block-size data. The key needs to be hashed before being used by the HMAC algorithm."
  
  h20a = mobius.crypt.hmac (k20, "sha2_224")
  h20a.update (data20)
  test.assert_equal (h20a.get_hex_digest (), "3a854166ac5d9f023f54d517d0b39dbd946770db9c2b95c9f6f565d1")

  h20b = mobius.crypt.hmac (k20, "sha2_256")
  h20b.update (data20)
  test.assert_equal (h20b.get_hex_digest (), "9b09ffa71b942fcb27635fbcd5b0e944bfdc63644f0713938a7f51535c3a35e2")

  h20c = mobius.crypt.hmac (k20, "sha2_384")
  h20c.update (data20)
  test.assert_equal (h20c.get_hex_digest (), "6617178e941f020d351e2f254e8fd32c602420feb0b8fb9adccebb82461e99c5a678cc31e799176d3860e6110c46523e")

  h20d = mobius.crypt.hmac (k20, "sha2_512")
  h20d.update (data20)
  test.assert_equal (h20d.get_hex_digest (), "e37b6a775dc87dbaa4dfa9f96e5e3ffddebd71f8867289865df5a32d20cdc944b6022cac3c4982b10d5eeb55c3e4de15134676fb6de0446065c97440fa8c6a58")
  
  test.end ()

def unittest_io_file ():

  test = unittest ("mobius.io.file")
  f = mobius.io.new_file_by_path ("/dev/zero")
  reader = f.new_reader ()

  # initial values
  test.assert_equal (reader.size, 0)
  test.assert_equal (reader.tell (), 0)
  
  # read bytes
  data = reader.read (16)
  test.assert_equal (len (data), 16)
  test.assert_equal (data[0], 0)
  test.assert_equal (data[15], 0)
  
  # values
  test.assert_equal (reader.tell (), 16)

  # seek
  reader.seek (100)
  test.assert_equal (reader.tell (), 100)

  # read more bytes
  data = reader.read (16)
  test.assert_equal (len (data), 16)
  test.assert_equal (data[0], 0)
  test.assert_equal (data[15], 0)

  # values
  test.assert_equal (reader.tell (), 116)

  test.end ()

def unittest_turing_hash_nt ():
  test = unittest ("mobius.os.win.hash_nt")

  def hash_nt (passwd):
    return binascii.hexlify (mobius.os.win.hash_nt (passwd))

  test.assert_equal (hash_nt (""), b"31d6cfe0d16ae931b73c59d7e0c089c0")
  test.assert_equal (hash_nt ("123456"), b"32ed87bdb5fdc5e9cba88547376818d4")
  test.assert_equal (hash_nt ("user"), b"57d583aa46d571502aad4bb7aea09c70")
  test.end ()

def unittest_turing_hash_lm ():
  test = unittest ("mobius.os.win.hash_lm")

  def hash_lm (passwd):
    return binascii.hexlify (mobius.os.win.hash_lm (passwd))

  test.assert_equal (hash_lm (""), b"aad3b435b51404eeaad3b435b51404ee")
  test.assert_equal (hash_lm ("123456"), b"44efce164ab921caaad3b435b51404ee")
  test.assert_equal (hash_lm ("user"), b"22124ea690b83bfbaad3b435b51404ee")
  test.assert_equal (hash_lm ("USER"), b"22124ea690b83bfbaad3b435b51404ee")
  test.end ()

def unittest_turing_hash_msdcc1 ():
  test = unittest ("mobius.os.win.hash_msdcc1")

  def hash_msdcc1 (passwd, username):
    return binascii.hexlify (mobius.os.win.hash_msdcc1 (passwd, username))

  test.assert_equal (hash_msdcc1 ("", "root"), b"176a4c2bd45ac73687676c2f09045353")
  test.assert_equal (hash_msdcc1 ("test2", "test2"), b"ab60bdb4493822b175486810ac2abe63")
  test.assert_equal (hash_msdcc1 ("test2", "TEST2"), b"ab60bdb4493822b175486810ac2abe63")
  test.end ()

def unittest_turing_hash_msdcc2 ():
  test = unittest ("mobius.os.win.hash_msdcc2")

  def hash_msdcc2 (passwd, username, iterations = 10240):
    return binascii.hexlify (mobius.os.win.hash_msdcc2 (passwd, username, iterations))

  test.assert_equal (hash_msdcc2 ("", "bin"), b"c0cbe0313a861062e29f92ede58f9b36")
  test.assert_equal (hash_msdcc2 ("test1", "test1"), b"607bbe89611e37446e736f7856515bf8")
  test.assert_equal (hash_msdcc2 ("qerwt", "Joe"), b"e09b38f84ab0be586b730baf61781e30")
  test.assert_equal (hash_msdcc2 ("12345", "Joe"), b"6432f517a900b3fc34ffe57f0f346e16")
  test.end ()

def unittest_pod ():
  test = unittest ("mobius.pod")
  
  m = mobius.pod.map ({'x' : 5, 'y' : 1.5, 'l' : [1, 3, 5]})
  
  # from/to bytearray
  data = mobius.pod.serialize (m)
  m2 = mobius.pod.unserialize (data)
  
  test.assert_equal (m2.get ('x'), 5)
  test.assert_equal (m2.get ('y'), 1.5)
  test.assert_equal (m2.get ('l'), [1, 3, 5])
  
  # from/to reader/writer
  f = mobius.io.new_file_by_path ('/tmp/data.pod')
  mobius.pod.serialize (f.new_writer (), m)
  m2 = mobius.pod.unserialize (f.new_reader ())

  test.assert_equal (m2.get ('x'), 5)
  test.assert_equal (m2.get ('y'), 1.5)
  test.assert_equal (m2.get ('l'), [1, 3, 5])

  test.end ()

# main code
unittest_core_application ()
unittest_crypt_cipher_3des ()
unittest_crypt_cipher_aes ()
unittest_crypt_cipher_des ()
unittest_crypt_cipher_rc4 ()
unittest_crypt_cipher_rot13 ()
unittest_crypt_cipher_zip ()
unittest_crypt_hash_adler32 ()
unittest_crypt_hash_crc32 ()
unittest_crypt_hash_md4 ()
unittest_crypt_hash_md5 ()
unittest_crypt_hash_sha1 ()
unittest_crypt_hash_sha2_224 ()
unittest_crypt_hash_sha2_256 ()
unittest_crypt_hash_sha2_384 ()
unittest_crypt_hash_sha2_512 ()
unittest_crypt_hash_sha2_512_224 ()
unittest_crypt_hash_sha2_512_256 ()
unittest_crypt_hash_zip ()
unittest_crypt_hmac ()
unittest_io_file ()
unittest_turing_hash_nt ()
unittest_turing_hash_lm ()
unittest_turing_hash_msdcc1 ()
unittest_turing_hash_msdcc2 ()
unittest_pod ()
unittest.final_summary ()
