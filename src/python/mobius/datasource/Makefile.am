noinst_LTLIBRARIES = libpymobius-datasource.la

libpymobius_datasource_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_datasource_la_LDFLAGS = $(PY_LIBS)

libpymobius_datasource_la_SOURCES =					\
	module.h							\
	module.cc							\
	datasource.h							\
	datasource.cc							\
	datasource_vfs.h						\
	datasource_vfs.cc						\
	func_datasource.cc						\
	ufdr/module.h							\
	ufdr/module.cc							\
	ufdr/datasource.h						\
	ufdr/datasource.cc						\
	ufdr/extraction.h						\
	ufdr/extraction.cc
