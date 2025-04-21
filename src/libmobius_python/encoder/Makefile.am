noinst_LTLIBRARIES = libpymobius-encoder.la

libpymobius_encoder_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_encoder_la_LDFLAGS = $(PY_LIBS)

libpymobius_encoder_la_SOURCES =			\
	module.hpp					\
	module.cpp					\
	func_base16.cpp					\
	func_base32.cpp					\
	func_base32hex.cpp				\
	func_base64.cpp					\
	func_base64url.cpp				\
	func_hexstring.cpp
