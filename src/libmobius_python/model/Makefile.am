noinst_LTLIBRARIES = libpymobius-model.la

libpymobius_model_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_model_la_LDFLAGS = $(PY_LIBS)

libpymobius_model_la_SOURCES =						\
	*.h								\
	module.cc							\
	ant.cc								\
	case.cc								\
	event.cc							\
	evidence.cc							\
	item.cc								\
	func_case.cc
