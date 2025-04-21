noinst_LTLIBRARIES = libpymobius-datetime.la

libpymobius_datetime_la_CXXFLAGS = -I$(top_srcdir)/src -I.. $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_datetime_la_LDFLAGS = $(PY_LIBS)

libpymobius_datetime_la_SOURCES =			\
	module.hpp					\
	module.cpp					\
	func_new_datetime_from_fat_time.cpp		\
	func_new_datetime_from_iso_string.cpp		\
	func_new_datetime_from_nt_timestamp.cpp		\
	func_new_datetime_from_unix_timestamp.cpp
