noinst_LTLIBRARIES = libpymobius-model.la

libpymobius_model_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_model_la_LDFLAGS = $(PY_LIBS)

libpymobius_model_la_SOURCES =						\
	*.hpp								\
	module.cpp							\
	ant.cpp								\
	case.cpp								\
	event.cpp							\
	evidence.cpp							\
	item.cpp								\
	func_case.cpp
