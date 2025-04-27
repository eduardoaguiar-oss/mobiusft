tools_PROGRAMS =							\
	casetree							\
	dirfs								\
	dirtree								\
	filesystem_scan							\
	hashfs								\
	hive_info							\
	hive_scan							\
	imagefile_cat							\
	imagefile_convert						\
	imagefile_info							\
	list_resources							\
	vfs_info
		
toolsdir = $(datadir)/$(PACKAGE)/tools

casetree_SOURCES = casetree.cpp
casetree_CXXFLAGS = $(LIBTSK_CXXFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/src
casetree_LDFLAGS = $(LIBTSK_LDFLAGS) -L$(top_srcdir)/src/libmobius_core/build -L$(top_srcdir)/src/libmobius_framework/build -lmobius_core -lmobius_framework
casetree_LDADD = $(top_srcdir)/src/mobius/libmobius.la

dirfs_SOURCES = dirfs.cpp
dirfs_CXXFLAGS = $(LIBTSK_CXXFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/src
dirfs_LDFLAGS = $(LIBTSK_LDFLAGS) -L$(top_srcdir)/src/libmobius_core/build -L$(top_srcdir)/src/libmobius_framework/build -lmobius_core -lmobius_framework
dirfs_LDADD = $(top_srcdir)/src/mobius/libmobius.la

dirtree_SOURCES = dirtree.cpp
dirtree_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
dirtree_LDFLAGS = -L$(top_srcdir)/src/libmobius_core/build -L$(top_srcdir)/src/libmobius_framework/build -lmobius_core -lmobius_framework
dirtree_LDADD = $(top_srcdir)/src/mobius/libmobius.la

filesystem_scan_SOURCES = filesystem_scan.cpp
filesystem_scan_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
filesystem_scan_LDFLAGS = -L$(top_srcdir)/src/libmobius_core/build -L$(top_srcdir)/src/libmobius_framework/build -lmobius_core -lmobius_framework
filesystem_scan_LDADD = $(top_srcdir)/src/mobius/libmobius.la

hashfs_SOURCES = hashfs.cpp
hashfs_CXXFLAGS = $(LIBTSK_CXXFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/src
hashfs_LDFLAGS = $(LIBTSK_LDFLAGS) -L$(top_srcdir)/src/libmobius_core/build -L$(top_srcdir)/src/libmobius_framework/build -lmobius_core -lmobius_framework
hashfs_LDADD = $(top_srcdir)/src/mobius/libmobius.la

hive_info_SOURCES = hive_info.cpp
hive_info_CXXFLAGS = $(LIBTSK_CXXFLAGS) -I$(top_srcdir)/src
hive_info_LDFLAGS = $(LIBTSK_LDFLAGS) -L$(top_srcdir)/src/libmobius_core/build -L$(top_srcdir)/src/libmobius_framework/build -lmobius_core -lmobius_framework
hive_info_LDADD = $(top_srcdir)/src/mobius/libmobius.la	

hive_scan_SOURCES = hive_scan.cpp
hive_scan_CXXFLAGS = $(LIBTSK_CXXFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/src
hive_scan_LDFLAGS = $(LIBTSK_LDFLAGS) -L$(top_srcdir)/src/libmobius_core/build -L$(top_srcdir)/src/libmobius_framework/build -lmobius_core -lmobius_framework
hive_scan_LDADD = $(top_srcdir)/src/mobius/libmobius.la

imagefile_cat_SOURCES = imagefile_cat.cpp
imagefile_cat_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
imagefile_cat_LDFLAGS = $(LIBTSK_LDFLAGS) -L$(top_srcdir)/src/libmobius_core/build -L$(top_srcdir)/src/libmobius_framework/build -lmobius_core -lmobius_framework
imagefile_cat_LDADD = $(top_srcdir)/src/mobius/libmobius.la

imagefile_info_SOURCES = imagefile_info.cpp
imagefile_info_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
imagefile_info_LDFLAGS = $(LIBTSK_LDFLAGS) -L$(top_srcdir)/src/libmobius_core/build -L$(top_srcdir)/src/libmobius_framework/build -lmobius_core -lmobius_framework
imagefile_info_LDADD = $(top_srcdir)/src/mobius/libmobius.la

imagefile_convert_SOURCES = imagefile_convert.cpp
imagefile_convert_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
imagefile_convert_LDFLAGS = $(LIBTSK_LDFLAGS) -L$(top_srcdir)/src/libmobius_core/build -L$(top_srcdir)/src/libmobius_framework/build -lmobius_core -lmobius_framework
imagefile_convert_LDADD = $(top_srcdir)/src/mobius/libmobius.la

list_resources_SOURCES = list_resources.cpp
list_resources_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
list_resources_LDFLAGS = $(LIBTSK_LDFLAGS) -L$(top_srcdir)/src/libmobius_core/build -L$(top_srcdir)/src/libmobius_framework/build -lmobius_core -lmobius_framework
list_resources_LDADD = $(top_srcdir)/src/mobius/libmobius.la

vfs_info_SOURCES = vfs_info.cpp
vfs_info_CXXFLAGS = $(LIBTSK_CXXFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/src
vfs_info_LDFLAGS = $(LIBTSK_LDFLAGS) -L$(top_srcdir)/src/libmobius_core/build -L$(top_srcdir)/src/libmobius_framework/build -lmobius_core -lmobius_framework
vfs_info_LDADD = $(top_srcdir)/src/mobius/libmobius.la

if HAVE_LIBUDEV
tools_PROGRAMS += device_list disk_list

device_list_SOURCES = device_list.cpp
device_list_CXXFLAGS = $(LIBUDEV_CXXFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/src
device_list_LDFLAGS = $(LIBUDEV_FLAGS) -L$(top_srcdir)/src/libmobius_core/build -L$(top_srcdir)/src/libmobius_framework/build -lmobius_core -lmobius_framework
device_list_LDADD = $(top_srcdir)/src/mobius/libmobius.la

disk_list_SOURCES = disk_list.cpp
disk_list_CXXFLAGS = $(LIBUDEV_CXXFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/src
disk_list_LDFLAGS = $(LIBUDEV_FLAGS) -L$(top_srcdir)/src/libmobius_core/build -L$(top_srcdir)/src/libmobius_framework/build -lmobius_core -lmobius_framework
disk_list_LDADD = $(top_srcdir)/src/mobius/libmobius.la
endif
