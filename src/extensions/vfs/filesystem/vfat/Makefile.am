filesystem_vfatdir = $(datadir)/$(PACKAGE)/extensions
filesystem_vfat_LTLIBRARIES = filesystem-vfat.la

filesystem_vfat_la_CXXFLAGS = -I$(top_srcdir)/src

filesystem_vfat_la_SOURCES =					\
	*.h							\
	main.cc							\
	filesystem_impl.cc

filesystem_vfat_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
