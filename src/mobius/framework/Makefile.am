mobius_framework_includedir = $(includedir)/mobius/framework
nobase_mobius_framework_include_HEADERS = *.h

lib_LTLIBRARIES = libmobius_framework.la
libmobius_framework_la_CXXFLAGS = -I$(top_srcdir)/src

libmobius_framework_la_LDFLAGS = -avoid-version $(LIBGCRYPT_LIBS)

libmobius_framework_la_SOURCES =					\
	attribute.cc							\
	category.cc							\
	config.cc							\
	evidence_flag.cc						\
	evidence_loader.cc

SUBDIRS =
