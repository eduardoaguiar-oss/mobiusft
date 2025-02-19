filesystem_ext2dir = $(datadir)/$(PACKAGE)/extensions
filesystem_ext2_LTLIBRARIES = filesystem-ext2.la

filesystem_ext2_la_CXXFLAGS = -I$(top_srcdir)/src

filesystem_ext2_la_SOURCES =					\
	*.h							\
	main.cc							\
	filesystem_impl.cc

filesystem_ext2_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
