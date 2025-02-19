noinst_LTLIBRARIES = libpymobius-database.la

libpymobius_database_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_database_la_LDFLAGS = $(PY_LIBS)

libpymobius_database_la_SOURCES =				\
	module.h						\
	module.cc						\
	connection.h						\
	connection.cc						\
	connection_set.h					\
	connection_set.cc					\
	transaction.h						\
	transaction.cc
