mobius_vfs_includedir = $(includedir)/mobius/vfs
nobase_mobius_vfs_include_HEADERS =					\
	*.h								\
	tsk/*.h

noinst_LTLIBRARIES = libmobius-vfs.la
libmobius_vfs_la_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
libmobius_vfs_la_LDFLAGS = -avoid-version

libmobius_vfs_la_SOURCES =						\
	block.cc							\
	block_impl_disk.cc						\
	block_impl_slice.cc						\
	disk.cc								\
	disk_impl_device.cc						\
	disk_impl_imagefile.cc						\
	filesystem.cc							\
	imagefile.cc							\
	segment_array.cc						\
	util.cc								\
	vfs.cc								\
	tsk/adaptor.cc							\
	tsk/exception.cc						\
	tsk/file_impl.cc						\
	tsk/folder_impl.cc						\
	tsk/fs_file.cc							\
	tsk/reader_impl_file.cc						\
	tsk/reader_impl_stream.cc					\
	tsk/stream_impl.cc
