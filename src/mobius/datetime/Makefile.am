mobius_datetime_includedir = $(includedir)/mobius/datetime
nobase_mobius_datetime_include_HEADERS = *.h

AUTOMAKE_OPTIONS = nostdinc

noinst_LTLIBRARIES = libmobius-datetime.la
libmobius_datetime_la_CXXFLAGS = -I$(top_srcdir)/src
libmobius_datetime_la_SOURCES =		\
	conv_iso_string.h		\
	conv_iso_string.cc		\
	conv_fat_time.cc		\
	conv_julian.h			\
	conv_julian.cc			\
	conv_nt_timestamp.cc		\
	conv_unix_timestamp.cc		\
	date.h				\
	date.cc				\
	datetime.h			\
	datetime.cc			\
	time.h				\
	time.cc				\
	timedelta.h			\
	timedelta.cc
libmobius_datetime_la_LDFLAGS = -avoid-version
