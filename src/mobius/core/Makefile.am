mobius_core_includedir = $(includedir)/mobius/core
nobase_mobius_core_include_HEADERS =					\
	*.h								\
	file_decoder/*.h

noinst_LTLIBRARIES = libmobius-core.la
libmobius_core_la_CXXFLAGS = -I$(top_srcdir)/src
libmobius_core_la_LDFLAGS = -avoid-version

libmobius_core_la_SOURCES =						\
	application.cc							\
	extension.cc							\
	extension_set.cc						\
	log.cc								\
	mediator.cc							\
	resource.cc							\
	thread_guard.cc							\
	file_decoder/decoder.cc						\
	file_decoder/entry.cc						\
	file_decoder/metadata.cc					\
	file_decoder/section.cc
