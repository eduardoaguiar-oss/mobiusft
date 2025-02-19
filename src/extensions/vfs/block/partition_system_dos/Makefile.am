vfs_block_partition_system_dosdir = $(datadir)/$(PACKAGE)/extensions
vfs_block_partition_system_dos_LTLIBRARIES = vfs_block_partition_system_dos.la

vfs_block_partition_system_dos_la_CXXFLAGS = -I$(top_srcdir)/src

vfs_block_partition_system_dos_la_SOURCES =					\
	main.cc

vfs_block_partition_system_dos_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
