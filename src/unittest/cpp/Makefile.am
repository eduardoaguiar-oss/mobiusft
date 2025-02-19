unittest_PROGRAMS = unittest
unittestdir = $(datadir)/$(PACKAGE)/tools

unittest_SOURCES = unittest.cc		\
	charset.cc			\
	core.cc				\
	crypt.cc			\
	datetime.cc			\
	decoder.cc			\
	encoder.cc			\
	framework.cc			\
	imagefile.cc			\
	io.cc				\
	os.cc				\
	pod.cc				\
	string.cc			\
	system.cc			\
	bytearray.cc			\
	zlib_functions.cc

unittest_CXXFLAGS = -I$(top_srcdir)/src
unittest_LDADD =								\
	$(top_srcdir)/src/mobius/libmobius.la					\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

SUBDIRS = benchmark
