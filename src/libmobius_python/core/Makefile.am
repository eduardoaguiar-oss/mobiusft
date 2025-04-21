noinst_LTLIBRARIES = libpymobius-core.la

libpymobius_core_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_core_la_LDFLAGS = $(PY_LIBS)

libpymobius_core_la_LIBADD =						\
        file_decoder/libpymobius-core-file_decoder.la

libpymobius_core_la_SOURCES =						\
	module.hpp							\
	module.cpp							\
	application.hpp							\
	application.cpp							\
	extension.hpp							\
	extension.cpp							\
	log.hpp								\
	log.cpp								\
	resource.hpp							\
	resource.cpp							\
	thread_guard.hpp							\
	thread_guard.cpp							\
	func_logf.cpp							\
	func_mediator.cpp						\
	func_set_logfile_path.cpp					\
	func_resource.cpp

SUBDIRS = 								\
	file_decoder
