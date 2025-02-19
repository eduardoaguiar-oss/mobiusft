imagefile_talondir = $(datadir)/$(PACKAGE)/extensions
imagefile_talon_LTLIBRARIES = imagefile-talon.la

imagefile_talon_la_CXXFLAGS = -I$(top_srcdir)/src

imagefile_talon_la_SOURCES =					\
	*.h							\
	main.cc							\
	imagefile_impl.cc

imagefile_talon_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
