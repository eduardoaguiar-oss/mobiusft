noinst_LTLIBRARIES = libpymobius-database.la

libpymobius_database_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_database_la_LDFLAGS = $(PY_LIBS)

libpymobius_database_la_SOURCES =				\
	module.hpp						\
	module.cpp						\
	connection.hpp						\
	connection.cpp						\
	connection_set.hpp					\
	connection_set.cpp					\
	transaction.hpp						\
	transaction.cpp
