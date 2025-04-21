noinst_LTLIBRARIES = libpymobius-vfs.la

libpymobius_vfs_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_vfs_la_LDFLAGS = $(PY_LIBS)

libpymobius_vfs_la_SOURCES =						\
	module.hpp							\
	module.cpp							\
	block.hpp								\
	block.cpp							\
	disk.hpp								\
	disk.cpp								\
	filesystem.hpp							\
	filesystem.cpp							\
	imagefile.hpp							\
	imagefile.cpp							\
	vfs.hpp								\
	vfs.cpp								\
	func_disk.cpp							\
	func_get_imagefile_implementations.cpp				\
	func_new_imagefile_by_path.cpp					\
	func_new_imagefile_by_url.cpp					\
	func_new_imagefile_from_file.cpp
