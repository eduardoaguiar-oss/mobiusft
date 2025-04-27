mobius_decoder_includedir = $(includedir)/mobius/decoder
nobase_mobius_decoder_include_HEADERS =			\
	*.h						\
	sgml/*.h					\
	xml/*.h

noinst_LTLIBRARIES = libmobius-decoder.la

libmobius_decoder_la_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src

libmobius_decoder_la_SOURCES =				\
	base64.cc					\
	btencode.cc					\
	data_decoder.cc					\
	filetype.cc					\
	hexstring.cc					\
	inifile.cc					\
	lnk.cc						\
	mfc.cc						\
	qdatastream.cc					\
	sourcecode.cc					\
	sgml/tokenizer.cc				\
	sgml/parser.cc					\
	tdf.cc						\
	xml/dom.cc					\
	xml/element.cc

libmobius_decoder_la_LDFLAGS = -avoid-version
