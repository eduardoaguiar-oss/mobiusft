noinst_LTLIBRARIES = libpymobius-io.la

libpymobius_io_la_CXXFLAGS = -I$(top_srcdir)/src -I.. $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_io_la_LDFLAGS = $(PY_LIBS)

libpymobius_io_la_SOURCES =						\
	*.h								\
	module.cc							\
	entry.cc							\
	file.cc								\
	folder.cc							\
	line_reader.cc							\
	path.cc								\
	reader.cc							\
	sequential_reader_adaptor.cc					\
	stream.cc							\
	tempfile.cc							\
	text_reader.cc							\
	text_writer.cc							\
	uri.cc								\
	walker.cc							\
	writer.cc							\
	func_get_current_folder.cc					\
	func_join_path.cc						\
	func_new_entry_by_path.cc					\
	func_new_entry_by_url.cc					\
	func_new_file_by_path.cc					\
	func_new_file_by_url.cc						\
	func_new_folder_by_path.cc					\
	func_new_folder_by_url.cc					\
	func_new_path_from_win.cc					\
	func_new_uri_from_path.cc					\
	func_reader.cc							\
	func_to_win_path.cc
