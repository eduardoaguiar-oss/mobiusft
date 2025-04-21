noinst_LTLIBRARIES = libpymobius-datasource.la

libpymobius_datasource_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_datasource_la_LDFLAGS = $(PY_LIBS)

libpymobius_datasource_la_SOURCES =					\
	module.hpp							\
	module.cpp							\
	datasource.hpp							\
	datasource.cpp							\
	datasource_vfs.hpp						\
	datasource_vfs.cpp						\
	func_datasource.cpp						\
	ufdr/module.hpp							\
	ufdr/module.cpp							\
	ufdr/datasource.hpp						\
	ufdr/datasource.cpp						\
	ufdr/extraction.hpp						\
	ufdr/extraction.cpp
