mobius_datasource_includedir = $(includedir)/mobius/datasource
nobase_mobius_datasource_include_HEADERS =				\
	*.h								\
	ufdr/*.h

noinst_LTLIBRARIES = libmobius-datasource.la
libmobius_datasource_la_CXXFLAGS = -I$(top_srcdir)/src
libmobius_datasource_la_LDFLAGS = -avoid-version

libmobius_datasource_la_SOURCES =					\
	datasource.cc							\
	datasource_vfs.cc						\
	datasource_impl_vfs.cc						\
	ufdr/datasource_impl.cc						\
	ufdr/datasource.cc						\
	ufdr/extraction.cc
