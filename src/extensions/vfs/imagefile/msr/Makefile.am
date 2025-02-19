imagefile_msrdir = $(datadir)/$(PACKAGE)/extensions
imagefile_msr_LTLIBRARIES = imagefile-msr.la

imagefile_msr_la_CXXFLAGS = -I$(top_srcdir)/src

imagefile_msr_la_SOURCES =					\
	*.h							\
	main.cc							\
	imagefile_impl.cc					\
	reader_impl_aes.cc					\
	reader_impl_blowfish.cc					\
	reader_impl_plaintext.cc

imagefile_msr_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius
