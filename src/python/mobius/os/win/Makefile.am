noinst_LTLIBRARIES = libpymobius-os-win.la
  
libpymobius_os_win_la_CXXFLAGS = -I$(top_srcdir)/src -I../.. $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_os_win_la_LDFLAGS = $(PY_LIBS)

libpymobius_os_win_la_LIBADD =

libpymobius_os_win_la_SOURCES =					\
	module.h						\
	module.cc						\
	credential.h						\
	credential.cc						\
	func_hash_ie_entropy.cc					\
	func_hash_lm.cc						\
	func_hash_nt.cc						\
	func_hash_msdcc1.cc					\
	func_hash_msdcc2.cc					\
	func_pbkdf2_hmac_ms.cc					\
	dpapi/module.h						\
	dpapi/module.cc						\
	dpapi/blob.h						\
	dpapi/blob.cc						\
	dpapi/credhist_entry.h					\
	dpapi/credhist_entry.cc					\
	dpapi/credhist_file.h					\
	dpapi/credhist_file.cc					\
	dpapi/master_key.h					\
	dpapi/master_key.cc					\
	dpapi/master_key_file.h					\
	dpapi/master_key_file.cc				\
	registry/module.h					\
	registry/module.cc					\
	registry/registry.h					\
	registry/registry.cc					\
	registry/registry_file.h				\
	registry/registry_file.cc				\
	registry/registry_key.h					\
	registry/registry_key.cc				\
	registry/registry_value.h				\
	registry/registry_value.cc				\
	registry/registry_data.h				\
	registry/registry_data.cc				\
	trashbin/module.h					\
	trashbin/module.cc					\
	trashbin/ifile.h					\
	trashbin/ifile.cc					\
	trashbin/info2_file.h					\
	trashbin/info2_file.cc					\
	trashbin/info2_entry.h					\
	trashbin/info2_entry.cc
