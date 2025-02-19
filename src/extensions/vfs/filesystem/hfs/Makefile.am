filesystem_hfsdir = $(datadir)/$(PACKAGE)/extensions
filesystem_hfs_LTLIBRARIES = filesystem-hfs.la

filesystem_hfs_la_CXXFLAGS = -I$(top_srcdir)/src

filesystem_hfs_la_SOURCES =					\
	*.h							\
	main.cc							\
	filesystem_impl.cc

filesystem_hfs_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
