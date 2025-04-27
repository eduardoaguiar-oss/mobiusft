mobius_io_includedir = $(includedir)/mobius/io
nobase_mobius_io_include_HEADERS = 		\
	*.h					\
       	local/*.h

noinst_LTLIBRARIES = libmobius-io.la

libmobius_io_la_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src

libmobius_io_la_SOURCES =			\
	bytearray_io.cc				\
	entry.cc				\
	file.cc					\
	folder.cc				\
	line_reader.cc				\
	path.cc					\
	reader.cc				\
	reader_impl_base.cc			\
	reader_impl_bytearray.cc		\
	reader_impl_slice.cc			\
	sector_reader_adaptor.cc		\
	sequential_reader_adaptor.cc		\
	stream.cc				\
	tempfile.cc				\
	text_reader.cc				\
	text_writer.cc				\
	walker.cc				\
	writer.cc				\
	writer_impl_base.cc			\
	writer_impl_bytearray.cc		\
	uri.cc					\
	local/file_impl.cc			\
	local/folder_impl.cc			\
	local/reader_impl.cc			\
	local/writer_impl.cc			\
	local/get_current_folder.cc		\
	local/new_entry_by_path.cc

libmobius_io_la_LDFLAGS = -avoid-version

if HAVE_LIBSMBCLIENT
libmobius_io_la_CXXFLAGS += $(LIBSMBCLIENT_CFLAGS)
libmobius_io_la_LDFLAGS += $(LIBSMBCLIENT_LIBS)

nobase_mobius_io_include_HEADERS += 		\
       	smb/*.h

libmobius_io_la_SOURCES +=			\
	smb/file_impl.cc			\
	smb/folder_impl.cc			\
	smb/reader_impl.cc			\
	smb/writer_impl.cc			\
	smb/init.cc				\
	smb/new_entry_by_url.cc
endif
