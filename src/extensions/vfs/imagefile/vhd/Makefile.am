imagefile_vhddir = $(datadir)/$(PACKAGE)/extensions
imagefile_vhd_LTLIBRARIES = imagefile-vhd.la

imagefile_vhd_la_CXXFLAGS = -I$(top_srcdir)/src

imagefile_vhd_la_SOURCES =					\
	*.h							\
	main.cc							\
	imagefile_impl.cc					\
	reader_impl_dynamic.cc					\
	reader_impl_fixed.cc

imagefile_vhd_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
