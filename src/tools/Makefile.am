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

casetree_SOURCES = casetree.cc
casetree_CXXFLAGS = $(LIBTSK_CXXFLAGS) -I$(top_srcdir)/src
casetree_LDFLAGS = $(LIBTSK_LDFLAGS)
casetree_LDADD = $(top_srcdir)/src/mobius/libmobius.la			\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

dirfs_SOURCES = dirfs.cc
dirfs_CXXFLAGS = $(LIBTSK_CXXFLAGS) -I$(top_srcdir)/src
dirfs_LDFLAGS = $(LIBTSK_LDFLAGS)
dirfs_LDADD = $(top_srcdir)/src/mobius/libmobius.la			\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

dirtree_SOURCES = dirtree.cc
dirtree_CXXFLAGS = -I$(top_srcdir)/src
dirtree_LDADD = $(top_srcdir)/src/mobius/libmobius.la			\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

filesystem_scan_SOURCES = filesystem_scan.cc
filesystem_scan_CXXFLAGS = -I$(top_srcdir)/src
filesystem_scan_LDADD = $(top_srcdir)/src/mobius/libmobius.la		\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

hashfs_SOURCES = hashfs.cc
hashfs_CXXFLAGS = $(LIBTSK_CXXFLAGS) -I$(top_srcdir)/src
hashfs_LDFLAGS = $(LIBTSK_LDFLAGS)
hashfs_LDADD = $(top_srcdir)/src/mobius/libmobius.la			\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

hive_info_SOURCES = hive_info.cc
hive_info_CXXFLAGS = $(LIBTSK_CXXFLAGS) -I$(top_srcdir)/src
hive_info_LDFLAGS = $(LIBTSK_LDFLAGS)
hive_info_LDADD = $(top_srcdir)/src/mobius/libmobius.la			\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

hive_scan_SOURCES = hive_scan.cc
hive_scan_CXXFLAGS = $(LIBTSK_CXXFLAGS) -I$(top_srcdir)/src
hive_scan_LDFLAGS = $(LIBTSK_LDFLAGS)
hive_scan_LDADD = $(top_srcdir)/src/mobius/libmobius.la			\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

imagefile_cat_SOURCES = imagefile_cat.cc
imagefile_cat_CXXFLAGS = -I$(top_srcdir)/src
imagefile_cat_LDADD = $(top_srcdir)/src/mobius/libmobius.la		\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

imagefile_info_SOURCES = imagefile_info.cc
imagefile_info_CXXFLAGS = -I$(top_srcdir)/src
imagefile_info_LDADD = $(top_srcdir)/src/mobius/libmobius.la		\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

imagefile_convert_SOURCES = imagefile_convert.cc
imagefile_convert_CXXFLAGS = -I$(top_srcdir)/src
imagefile_convert_LDADD = $(top_srcdir)/src/mobius/libmobius.la		\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

list_resources_SOURCES = list_resources.cc
list_resources_CXXFLAGS = -I$(top_srcdir)/src
list_resources_LDADD = $(top_srcdir)/src/mobius/libmobius.la		\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

vfs_info_SOURCES = vfs_info.cc
vfs_info_CXXFLAGS = $(LIBTSK_CXXFLAGS) -I$(top_srcdir)/src
vfs_info_LDFLAGS = $(LIBTSK_LDFLAGS)
vfs_info_LDADD = $(top_srcdir)/src/mobius/libmobius.la			\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

if HAVE_LIBUDEV
tools_PROGRAMS += device_list disk_list

device_list_SOURCES = device_list.cc
device_list_CXXFLAGS = $(LIBUDEV_CXXFLAGS) -I$(top_srcdir)/src
device_list_LDFLAGS = $(LIBUDEV_FLAGS)
device_list_LDADD = $(top_srcdir)/src/mobius/libmobius.la		\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

disk_list_SOURCES = disk_list.cc
disk_list_CXXFLAGS = $(LIBUDEV_CXXFLAGS) -I$(top_srcdir)/src
disk_list_LDFLAGS = $(LIBUDEV_FLAGS)
disk_list_LDADD = $(top_srcdir)/src/mobius/libmobius.la			\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la
endif
