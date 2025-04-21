noinst_LTLIBRARIES = libpymobius-pod.la

libpymobius_pod_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_pod_la_LDFLAGS = $(PY_LIBS)

libpymobius_pod_la_SOURCES =			\
	module.hpp				\
	module.cpp				\
	data.hpp					\
	data.cpp					\
	map.hpp					\
	map.cpp					\
	func_serialize.cpp			\
	func_unserialize.cpp
