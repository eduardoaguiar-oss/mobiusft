mobius_pod_includedir = $(includedir)/mobius/pod
nobase_mobius_pod_include_HEADERS = *.h

noinst_LTLIBRARIES = libmobius-pod.la
libmobius_pod_la_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
libmobius_pod_la_LDFLAGS = -avoid-version

libmobius_pod_la_SOURCES =					\
	data_impl_list.cc					\
	data.cc							\
	map.cc							\
	serialize.cc						\
	unserialize.cc
