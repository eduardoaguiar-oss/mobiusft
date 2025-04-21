noinst_LTLIBRARIES = libpymobius-pod.la

libpymobius_pod_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_pod_la_LDFLAGS = $(PY_LIBS)

libpymobius_pod_la_SOURCES =			\
	module.h				\
	module.cc				\
	data.h					\
	data.cc					\
	map.h					\
	map.cc					\
	func_serialize.cc			\
	func_unserialize.cc
