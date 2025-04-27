mobius_os_includedir = $(includedir)/mobius/os
#nobase_mobius_os_include_HEADERS = *.h

noinst_LTLIBRARIES = libmobius-os.la
libmobius_os_la_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
libmobius_os_la_LDFLAGS = -avoid-version

libmobius_os_la_SOURCES =

libmobius_os_la_LIBADD =				\
	win/libmobius-os-win.la

SUBDIRS =						\
	win

