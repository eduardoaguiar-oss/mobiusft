noinst_LTLIBRARIES = libpymobius-decoder.la

libpymobius_decoder_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_decoder_la_LDFLAGS = $(PY_LIBS)

libpymobius_decoder_la_SOURCES =			\
	*.h						\
	sgml/*.h					\
	xml/*.h						\
	module.cc					\
	data_decoder.cc					\
	inifile.cc					\
	lnk.cc						\
	mfc.cc						\
	qdatastream.cc					\
	sourcecode.cc					\
	func_base16.cc					\
	func_base32.cc					\
	func_base32hex.cc				\
	func_base64.cc					\
	func_base64url.cc				\
	func_btencode.cc				\
	func_get_filetype.cc				\
	sgml/module.cc					\
	sgml/tokenizer.cc				\
	sgml/parser.cc					\
	tdf.cc						\
	xml/module.cc					\
	xml/dom.cc					\
	xml/element.cc
