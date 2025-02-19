mobius_encoder_includedir = $(includedir)/mobius/encoder
nobase_mobius_encoder_include_HEADERS = *.h

noinst_LTLIBRARIES = libmobius-encoder.la
libmobius_encoder_la_CXXFLAGS = -I$(top_srcdir)/src
libmobius_encoder_la_LDFLAGS = -avoid-version

libmobius_encoder_la_SOURCES =					\
	base64.cc						\
	data_encoder.cc						\
	hexstring.cc
