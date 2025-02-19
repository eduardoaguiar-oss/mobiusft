vfs_block_filesystemsdir = $(datadir)/$(PACKAGE)/extensions
vfs_block_filesystems_LTLIBRARIES = vfs_block_filesystems.la

vfs_block_filesystems_la_CXXFLAGS = -I$(top_srcdir)/src

vfs_block_filesystems_la_SOURCES =					\
	main.cc

vfs_block_filesystems_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
