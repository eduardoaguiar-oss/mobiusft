mobius_includedir = $(includedir)/mobius
nobase_mobius_include_HEADERS = *.h *.inc

lib_LTLIBRARIES = libmobius.la
libmobius_la_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
libmobius_la_SOURCES =						\
	bytearray.cc						\
	exception.cc						\
	string_functions.cc

libmobius_la_LDFLAGS = -avoid-version $(LIBGCRYPT_LIBS) $(LIBXML2_LIBS)

libmobius_la_LIBADD =						\
	datetime/libmobius-datetime.la				\
	decoder/libmobius-decoder.la				\
	io/libmobius-io.la					\
	os/libmobius-os.la

SUBDIRS =							\
	datetime						\
	decoder							\
	io							\
	os
