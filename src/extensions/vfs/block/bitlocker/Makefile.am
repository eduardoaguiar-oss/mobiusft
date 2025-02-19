vfs_block_bitlockerdir = $(datadir)/$(PACKAGE)/extensions
vfs_block_bitlocker_LTLIBRARIES = vfs_block_bitlocker.la

vfs_block_bitlocker_la_CXXFLAGS = -I$(top_srcdir)/src

vfs_block_bitlocker_la_SOURCES =					\
	main.cc

vfs_block_bitlocker_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
