imagefile_vhdxdir = $(datadir)/$(PACKAGE)/extensions
imagefile_vhdx_LTLIBRARIES = imagefile-vhdx.la

imagefile_vhdx_la_CXXFLAGS = -I$(top_srcdir)/src

imagefile_vhdx_la_SOURCES =					\
	*.h							\
	main.cc							\
	imagefile_impl.cc					\
	reader_impl.cc

imagefile_vhdx_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
