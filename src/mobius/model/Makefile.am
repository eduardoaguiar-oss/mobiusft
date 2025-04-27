mobius_model_includedir = $(includedir)/mobius/model
nobase_mobius_model_include_HEADERS = *.h

noinst_LTLIBRARIES = libmobius-model.la
libmobius_model_la_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
libmobius_model_la_LDFLAGS = -avoid-version

libmobius_model_la_SOURCES =						\
	ant.cc								\
	case.cc								\
	case_schema.cc							\
	event.cc							\
	evidence.cc							\
	item.cc								\
	item_ant.cc
