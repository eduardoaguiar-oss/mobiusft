app_emuletorrentdir = $(datadir)/$(PACKAGE)/extensions
app_emuletorrent_LTLIBRARIES = app-emuletorrent.la

app_emuletorrent_la_CXXFLAGS = -I$(top_srcdir)/src

app_emuletorrent_la_SOURCES =						\
	*.h								\
	main.cc								\
	evidence_loader_impl.cc						\
	file_bt_fastresume.cc						\
	file_ed2k_fastresume.cc

app_emuletorrent_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -L$(top_srcdir)/src/mobius/framework -lmobius_framework -lmobius
