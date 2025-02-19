date_codedir = $(datadir)/$(PACKAGE)/extensions
date_code_LTLIBRARIES = date_code.la

date_code_la_CXXFLAGS = -I$(top_srcdir)/src

date_code_la_SOURCES =						\
	main.cc	

date_code_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
