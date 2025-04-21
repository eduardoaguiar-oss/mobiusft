noinst_LTLIBRARIES = libpymobius-encoder.la

libpymobius_encoder_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_encoder_la_LDFLAGS = $(PY_LIBS)

libpymobius_encoder_la_SOURCES =			\
	module.h					\
	module.cc					\
	func_base16.cc					\
	func_base32.cc					\
	func_base32hex.cc				\
	func_base64.cc					\
	func_base64url.cc				\
	func_hexstring.cc
