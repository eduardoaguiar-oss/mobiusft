noinst_LTLIBRARIES = libpymobius-framework.la

libpymobius_framework_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_framework_la_LDFLAGS = $(PY_LIBS)

libpymobius_framework_la_SOURCES =					\
	*.h								\
	module.cc							\
	attribute.cc							\
	category.cc							\
	evidence_loader.cc						\
	func_config.cc
