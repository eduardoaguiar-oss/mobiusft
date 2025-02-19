vfs_block_partition_system_apmdir = $(datadir)/$(PACKAGE)/extensions
vfs_block_partition_system_apm_LTLIBRARIES = vfs_block_partition_system_apm.la

vfs_block_partition_system_apm_la_CXXFLAGS = -I$(top_srcdir)/src

vfs_block_partition_system_apm_la_SOURCES =					\
	main.cc

vfs_block_partition_system_apm_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
