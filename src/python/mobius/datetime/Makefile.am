noinst_LTLIBRARIES = libpymobius-datetime.la

libpymobius_datetime_la_CXXFLAGS = -I$(top_srcdir)/src -I.. $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_datetime_la_LDFLAGS = $(PY_LIBS)

libpymobius_datetime_la_SOURCES =			\
	module.h					\
	module.cc					\
	func_new_datetime_from_fat_time.cc		\
	func_new_datetime_from_iso_string.cc		\
	func_new_datetime_from_nt_timestamp.cc		\
	func_new_datetime_from_unix_timestamp.cc
