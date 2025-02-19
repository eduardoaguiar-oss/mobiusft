imagefile_splitdir = $(datadir)/$(PACKAGE)/extensions
imagefile_split_LTLIBRARIES = imagefile-split.la

imagefile_split_la_CXXFLAGS = -I$(top_srcdir)/src

imagefile_split_la_SOURCES =					\
	*.h							\
	main.cc							\
	imagefile_impl.cc					\
	reader_impl.cc						\
	writer_impl.cc

imagefile_split_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
