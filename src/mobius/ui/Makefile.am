mobius_ui_includedir = $(includedir)/mobius/ui
nobase_mobius_ui_include_HEADERS = *.h

noinst_LTLIBRARIES = libmobius-ui.la
libmobius_ui_la_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
libmobius_ui_la_LDFLAGS = -avoid-version

libmobius_ui_la_SOURCES =						\
	box.cc								\
	button.cc							\
	container.cc							\
	icon.cc								\
	label.cc							\
	message_dialog.cc						\
	stacked_container.cc						\
	ui.cc								\
	widget.cc							\
	window.cc
