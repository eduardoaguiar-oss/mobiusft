imagefile_dossierdir = $(datadir)/$(PACKAGE)/extensions
imagefile_dossier_LTLIBRARIES = imagefile-dossier.la

imagefile_dossier_la_CXXFLAGS = -I$(top_srcdir)/src

imagefile_dossier_la_SOURCES =					\
	*.h							\
	main.cc							\
	imagefile_impl.cc

imagefile_dossier_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
