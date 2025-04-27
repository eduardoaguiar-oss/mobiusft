mobius_kff_includedir = $(includedir)/mobius/kff
nobase_mobius_kff_include_HEADERS = *.h

noinst_LTLIBRARIES = libmobius-kff.la
libmobius_kff_la_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
libmobius_kff_la_LDFLAGS = -avoid-version

libmobius_kff_la_SOURCES =					\
	kff.cc							\
	hashset.cc
