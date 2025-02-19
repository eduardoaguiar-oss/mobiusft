filesystem_ntfsdir = $(datadir)/$(PACKAGE)/extensions
filesystem_ntfs_LTLIBRARIES = filesystem-ntfs.la

filesystem_ntfs_la_CXXFLAGS = -I$(top_srcdir)/src

filesystem_ntfs_la_SOURCES =					\
	*.h							\
	main.cc							\
	filesystem_impl.cc

filesystem_ntfs_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
