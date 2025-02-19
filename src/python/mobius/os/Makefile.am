noinst_LTLIBRARIES = libpymobius-os.la
  
libpymobius_os_la_CXXFLAGS = -I$(top_srcdir)/src -I.. $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_os_la_LDFLAGS = $(PY_LIBS)

libpymobius_os_la_LIBADD =					\
	win/libpymobius-os-win.la

libpymobius_os_la_SOURCES =					\
	module.h						\
	module.cc

SUBDIRS =							\
	win
