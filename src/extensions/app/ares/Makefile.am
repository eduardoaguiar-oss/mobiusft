app_aresdir = $(datadir)/$(PACKAGE)/extensions
app_ares_LTLIBRARIES = app-ares.la

app_ares_la_CXXFLAGS = -I$(top_srcdir)/src

app_ares_la_SOURCES =							\
	*.h								\
	main.cc								\
	common.cc							\
	decoder_impl_arestra.cc						\
	decoder_impl_pbthash.cc						\
	decoder_impl_phash.cc						\
	decoder_impl_shareh.cc						\
	decoder_impl_sharel.cc						\
	decoder_impl_torrenth.cc					\
	evidence_loader_impl.cc						\
	file_arestra.cc							\
	file_pbthash.cc							\
	file_phash.cc							\
	file_shareh.cc							\
	file_sharel.cc							\
	file_torrenth.cc

app_ares_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -L$(top_srcdir)/src/mobius/framework -lmobius_framework -lmobius
