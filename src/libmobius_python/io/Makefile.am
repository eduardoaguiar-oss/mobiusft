noinst_LTLIBRARIES = libpymobius-io.la

libpymobius_io_la_CXXFLAGS = -I$(top_srcdir)/src -I.. $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_io_la_LDFLAGS = $(PY_LIBS)

libpymobius_io_la_SOURCES =						\
	*.hpp								\
	module.cpp							\
	entry.cpp							\
	file.cpp								\
	folder.cpp							\
	line_reader.cpp							\
	path.cpp								\
	reader.cpp							\
	sequential_reader_adaptor.cpp					\
	stream.cpp							\
	tempfile.cpp							\
	text_reader.cpp							\
	text_writer.cpp							\
	uri.cpp								\
	walker.cpp							\
	writer.cpp							\
	func_get_current_folder.cpp					\
	func_join_path.cpp						\
	func_new_entry_by_path.cpp					\
	func_new_entry_by_url.cpp					\
	func_new_file_by_path.cpp					\
	func_new_file_by_url.cpp						\
	func_new_folder_by_path.cpp					\
	func_new_folder_by_url.cpp					\
	func_new_path_from_win.cpp					\
	func_new_uri_from_path.cpp					\
	func_reader.cpp							\
	func_to_win_path.cpp
