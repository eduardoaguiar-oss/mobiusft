app_shareazadir = $(datadir)/$(PACKAGE)/extensions
app_shareaza_LTLIBRARIES = app-shareaza.la

app_shareaza_la_CXXFLAGS = -I$(top_srcdir)/src

app_shareaza_la_SOURCES =						\
	*.h								\
	main.cc								\
	evidence_loader_impl.cc						\
	file_searches_dat.cc						\
	CBTInfo.cc							\
	CDownload.cc							\
	CDownloadSource.cc						\
	CLibrary.cc							\
	CLibraryFile.cc							\
	CLibraryFolder.cc						\
	CManagedSearch.cc						\
	CMatchFile.cc							\
	CMatchList.cc							\
	CQueryHit.cc							\
	CQuerySearch.cc							\
	CShareaza.cc							\
	CSharedSource.cc						\
	CThumbCache.cc							\
	CWndSearch.cc							\
	CXMLElement.cc

app_shareaza_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -L$(top_srcdir)/src/mobius/framework -lmobius_framework -lmobius

toolsdir = $(datadir)/$(PACKAGE)/tools

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# tools
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
tools_PROGRAMS = 							\
	shareaza_library_dat						\
	shareaza_sd							\
	shareaza_searches_dat

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# tool: shareaza_library_dat
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
shareaza_library_dat_SOURCES =						\
	tools/shareaza_library_dat.cc					\
	CLibrary.cc							\
	CLibraryFolder.cc						\
	CLibraryFile.cc							\
	CSharedSource.cc						\
	CXMLElement.cc

shareaza_library_dat_CXXFLAGS = -I$(top_srcdir)/src
shareaza_library_dat_LDADD =						\
	$(top_srcdir)/src/mobius/libmobius.la				\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# tool: shareaza_sd
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
shareaza_sd_SOURCES =							\
	tools/shareaza_sd.cc						\
	CBTInfo.cc							\
	CDownload.cc							\
	CDownloadSource.cc						\
	CShareaza.cc							\
	CXMLElement.cc

shareaza_sd_CXXFLAGS = -I$(top_srcdir)/src
shareaza_sd_LDADD =							\
	$(top_srcdir)/src/mobius/libmobius.la				\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# tool: shareaza_searches_dat
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
shareaza_searches_dat_SOURCES =						\
	tools/shareaza_searches_dat.cc					\
	CManagedSearch.cc						\
	CMatchFile.cc							\
	CMatchList.cc							\
	CQueryHit.cc							\
	CQuerySearch.cc							\
	CShareaza.cc							\
	CWndSearch.cc							\
	CXMLElement.cc

shareaza_searches_dat_CXXFLAGS = -I$(top_srcdir)/src
shareaza_searches_dat_LDADD =						\
	$(top_srcdir)/src/mobius/libmobius.la				\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la

