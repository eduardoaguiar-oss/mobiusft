noinst_LTLIBRARIES = libpymobius-crypt.la

libpymobius_crypt_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_crypt_la_LDFLAGS = $(PY_LIBS)

libpymobius_crypt_la_SOURCES =				\
	module.h					\
	module.cc					\
	cipher.h					\
	cipher.cc					\
	hash.h						\
	hash.cc						\
	hmac.h						\
	hmac.cc						\
	func_cipher.cc					\
	func_hash_digest.cc				\
	func_pbkdf1.cc					\
	func_pbkdf2_hmac.cc				\
	func_rot13.cc
