filesystem_exfatdir = $(datadir)/$(PACKAGE)/extensions
filesystem_exfat_LTLIBRARIES = filesystem-exfat.la

filesystem_exfat_la_CXXFLAGS = -I$(top_srcdir)/src

filesystem_exfat_la_SOURCES =					\
	*.h							\
	main.cc							\
	filesystem_impl.cc

filesystem_exfat_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
