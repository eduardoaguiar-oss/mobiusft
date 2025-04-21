noinst_LTLIBRARIES = libpymobius-vfs.la

libpymobius_vfs_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_vfs_la_LDFLAGS = $(PY_LIBS)

libpymobius_vfs_la_SOURCES =						\
	module.h							\
	module.cc							\
	block.h								\
	block.cc							\
	disk.h								\
	disk.cc								\
	filesystem.h							\
	filesystem.cc							\
	imagefile.h							\
	imagefile.cc							\
	vfs.h								\
	vfs.cc								\
	func_disk.cc							\
	func_get_imagefile_implementations.cc				\
	func_new_imagefile_by_path.cc					\
	func_new_imagefile_by_url.cc					\
	func_new_imagefile_from_file.cc
