noinst_LTLIBRARIES = libpymobius-crypt.la

libpymobius_crypt_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_crypt_la_LDFLAGS = $(PY_LIBS)

libpymobius_crypt_la_SOURCES =				\
	module.hpp					\
	module.cpp					\
	cipher.hpp					\
	cipher.cpp					\
	hash.hpp						\
	hash.cpp						\
	hmac.hpp						\
	hmac.cpp						\
	func_cipher.cpp					\
	func_hash_digest.cpp				\
	func_pbkdf1.cpp					\
	func_pbkdf2_hmac.cpp				\
	func_rot13.cpp
