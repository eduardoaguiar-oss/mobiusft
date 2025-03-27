mobius_includedir = $(includedir)/mobius
nobase_mobius_include_HEADERS = *.h *.inc

lib_LTLIBRARIES = libmobius.la
libmobius_la_CXXFLAGS = -I$(top_srcdir)/src
libmobius_la_SOURCES =						\
	benchmark.cc						\
	bytearray.cc						\
	charset.cc						\
	exception.cc						\
	metadata.cc						\
	string_functions.cc					\
	unittest.cc

libmobius_la_LDFLAGS = -avoid-version $(LIBGCRYPT_LIBS) $(LIBXML2_LIBS)

libmobius_la_LIBADD =						\
	core/libmobius-core.la					\
	crypt/libmobius-crypt.la				\
	database/libmobius-database.la				\
	datasource/libmobius-datasource.la			\
	datetime/libmobius-datetime.la				\
	decoder/libmobius-decoder.la				\
	encoder/libmobius-encoder.la				\
	kff/libmobius-kff.la					\
	io/libmobius-io.la					\
	model/libmobius-model.la				\
	os/libmobius-os.la					\
	pod/libmobius-pod.la					\
	system/libmobius-system.la				\
	turing/libmobius-turing.la				\
	ui/libmobius-ui.la					\
	vfs/libmobius-vfs.la

SUBDIRS =							\
	core							\
	crypt							\
	database						\
	datasource						\
	datetime						\
	decoder							\
	encoder							\
	framework						\
	kff							\
	io							\
	model							\
	os							\
	pod							\
	system							\
	turing							\
	ui							\
	vfs

if HAVE_LIBZ
libmobius_la_SOURCES +=						\
	zlib_functions.h					\
	zlib_functions.cc
libmobius_la_CXXFLAGS += $(ZLIB_CXXFLAGS)
libmobius_la_LDFLAGS += $(ZLIB_LDFLAGS)
endif
