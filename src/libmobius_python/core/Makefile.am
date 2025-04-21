noinst_LTLIBRARIES = libpymobius-core.la

libpymobius_core_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_core_la_LDFLAGS = $(PY_LIBS)

libpymobius_core_la_LIBADD =						\
        file_decoder/libpymobius-core-file_decoder.la

libpymobius_core_la_SOURCES =						\
	module.h							\
	module.cc							\
	application.h							\
	application.cc							\
	extension.h							\
	extension.cc							\
	log.h								\
	log.cc								\
	resource.h							\
	resource.cc							\
	thread_guard.h							\
	thread_guard.cc							\
	func_logf.cc							\
	func_mediator.cc						\
	func_set_logfile_path.cc					\
	func_resource.cc

SUBDIRS = 								\
	file_decoder
