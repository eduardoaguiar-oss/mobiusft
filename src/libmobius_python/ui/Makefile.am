noinst_LTLIBRARIES = libpymobius-ui.la

libpymobius_ui_la_CXXFLAGS = -I$(top_srcdir)/src -I.. -I../api $(GTK3_CXXFLAGS) $(PYGOBJECT_CXXFLAGS) $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_ui_la_LDFLAGS = $(PY_LIBS)

libpymobius_ui_la_SOURCES =						\
	*.hpp								\
	module.cpp							\
	func_ui.cpp							\
	box.cpp								\
	button.cpp							\
	container.cpp							\
	icon.cpp								\
	label.cpp							\
	message_dialog.cpp						\
	stacked_container.cpp						\
	widget.cpp							\
	window.cpp
