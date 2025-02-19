mobius_system_includedir = $(includedir)/mobius/system

nobase_mobius_system_include_HEADERS =			\
	dynamic_lib.h					\
	group.h						\
	user.h

noinst_LTLIBRARIES = libmobius-system.la

libmobius_system_la_CXXFLAGS = -I$(top_srcdir)/src
libmobius_system_la_SOURCES =				\
	dynamic_lib.cc					\
	group.cc					\
	user.cc

libmobius_system_la_LDFLAGS = -avoid-version

if HAVE_LIBUDEV
libmobius_system_la_SOURCES += 				\
	device.cc					\
	device_list.cc
	
nobase_mobius_system_include_HEADERS +=			\
	device.h					\
	device_list.h
endif
