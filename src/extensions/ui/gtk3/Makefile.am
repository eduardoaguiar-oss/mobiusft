ui_gtk3dir = $(datadir)/$(PACKAGE)/extensions
ui_gtk3_LTLIBRARIES = ui-gtk3.la

#ui_gtk3_includedir = $(includedir)/extensions/ui/gtk3
#ui_gtk3_include_HEADERS = *.h

ui_gtk3_la_CXXFLAGS = -I$(top_srcdir)/src $(GTK3_CXXFLAGS)

ui_gtk3_la_SOURCES =							\
	*.h								\
	main.cc								\
	box_impl.cc							\
	button_impl.cc							\
	icon_impl.cc							\
	label_impl.cc							\
	message_dialog_impl.cc						\
	ui_impl.cc							\
	widget_impl.cc							\
	window_impl.cc

ui_gtk3_la_LDFLAGS = -module -avoid-version -shared -L$(top_srcdir)/src/mobius -lmobius $(GTK3_LDFLAGS)
