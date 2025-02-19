noinst_LTLIBRARIES = libpymobius-ui.la

libpymobius_ui_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(GTK3_CXXFLAGS) $(PYGOBJECT_CXXFLAGS) $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_ui_la_LDFLAGS = $(PY_LIBS)

libpymobius_ui_la_SOURCES =						\
	*.h								\
	module.cc							\
	func_ui.cc							\
	box.cc								\
	button.cc							\
	container.cc							\
	icon.cc								\
	label.cc							\
	message_dialog.cc						\
	stacked_container.cc						\
	widget.cc							\
	window.cc
