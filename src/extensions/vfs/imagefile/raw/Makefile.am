imagefile_rawdir = $(datadir)/$(PACKAGE)/extensions
imagefile_raw_LTLIBRARIES = imagefile-raw.la

imagefile_raw_la_CXXFLAGS = -I$(top_srcdir)/src

imagefile_raw_la_SOURCES =					\
	*.h							\
	main.cc							\
	imagefile_impl.cc

imagefile_raw_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
