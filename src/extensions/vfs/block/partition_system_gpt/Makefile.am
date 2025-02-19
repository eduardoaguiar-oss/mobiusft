vfs_block_partition_system_gptdir = $(datadir)/$(PACKAGE)/extensions
vfs_block_partition_system_gpt_LTLIBRARIES = vfs_block_partition_system_gpt.la

vfs_block_partition_system_gpt_la_CXXFLAGS = -I$(top_srcdir)/src

vfs_block_partition_system_gpt_la_SOURCES =					\
	main.cc

vfs_block_partition_system_gpt_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
