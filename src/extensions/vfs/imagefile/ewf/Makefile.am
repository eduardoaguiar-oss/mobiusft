imagefile_ewfdir = $(datadir)/$(PACKAGE)/extensions
imagefile_ewf_LTLIBRARIES = imagefile-ewf.la

imagefile_ewf_la_CXXFLAGS = -I$(top_srcdir)/src

imagefile_ewf_la_SOURCES =						\
	*.h								\
	main.cc								\
	imagefile_impl.cc						\
	reader_impl.cc							\
	writer_impl.cc							\
	segment_decoder.cc						\
	segment_writer.cc

imagefile_ewf_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -L$(top_srcdir)/src/mobius/framework -lmobius -lmobius_framework


tools_PROGRAMS = ewf_decoder
toolsdir = $(datadir)/$(PACKAGE)/tools

ewf_decoder_SOURCES =							\
	tools/ewf_decoder.cc						\
	segment_decoder.cc

ewf_decoder_CXXFLAGS = -I$(top_srcdir)/src
ewf_decoder_LDADD = $(top_srcdir)/src/mobius/libmobius.la		\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la
