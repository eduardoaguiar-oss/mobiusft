filesystem_isodir = $(datadir)/$(PACKAGE)/extensions
filesystem_iso_LTLIBRARIES = filesystem-iso.la

filesystem_iso_la_CXXFLAGS = -I$(top_srcdir)/src

filesystem_iso_la_SOURCES =					\
	*.h							\
	main.cc							\
	filesystem_impl.cc

filesystem_iso_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
