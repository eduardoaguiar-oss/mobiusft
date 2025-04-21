noinst_LTLIBRARIES = libpymobius-system.la

libpymobius_system_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_system_la_LDFLAGS = $(PY_LIBS)

libpymobius_system_la_SOURCES =			\
	module.hpp				\
	module.cpp				\
	device.hpp				\
	device.cpp				\
	func_get_devices.cpp
