noinst_LTLIBRARIES = libpymobius-decoder.la

libpymobius_decoder_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_decoder_la_LDFLAGS = $(PY_LIBS)

libpymobius_decoder_la_SOURCES =			\
	*.hpp						\
	sgml/*.hpp					\
	xml/*.hpp						\
	module.cpp					\
	data_decoder.cpp					\
	inifile.cpp					\
	lnk.cpp						\
	mfc.cpp						\
	qdatastream.cpp					\
	sourcecode.cpp					\
	func_base16.cpp					\
	func_base32.cpp					\
	func_base32hex.cpp				\
	func_base64.cpp					\
	func_base64url.cpp				\
	func_btencode.cpp				\
	func_get_filetype.cpp				\
	sgml/module.cpp					\
	sgml/tokenizer.cpp				\
	sgml/parser.cpp					\
	tdf.cpp						\
	xml/module.cpp					\
	xml/dom.cpp					\
	xml/element.cpp
