imagefile_solodir = $(datadir)/$(PACKAGE)/extensions
imagefile_solo_LTLIBRARIES = imagefile-solo.la

imagefile_solo_la_CXXFLAGS = -I$(top_srcdir)/src

imagefile_solo_la_SOURCES =					\
	*.h							\
	main.cc							\
	imagefile_impl.cc

imagefile_solo_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
