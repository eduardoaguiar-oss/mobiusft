mobius_database_includedir = $(includedir)/mobius/database
nobase_mobius_database_include_HEADERS = *.h

noinst_LTLIBRARIES = libmobius-database.la
libmobius_database_la_CXXFLAGS = -I$(top_srcdir)/src
libmobius_database_la_LDFLAGS = -avoid-version
libmobius_database_la_SOURCES =					\
	connection.cc						\
	connection_pool.cc					\
	connection_set.cc					\
	database.cc						\
	exception.inc						\
	meta_table.cc						\
	statement.cc						\
	transaction.cc
