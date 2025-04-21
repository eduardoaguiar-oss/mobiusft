noinst_LTLIBRARIES = libpymobius-os-win.la
  
libpymobius_os_win_la_CXXFLAGS = -I$(top_srcdir)/src -I../.. $(PY_CXXFLAGS) -fno-strict-aliasing

libpymobius_os_win_la_LDFLAGS = $(PY_LIBS)

libpymobius_os_win_la_LIBADD =

libpymobius_os_win_la_SOURCES =					\
	module.hpp						\
	module.cpp						\
	credential.hpp						\
	credential.cpp						\
	func_hash_ie_entropy.cpp					\
	func_hash_lm.cpp						\
	func_hash_nt.cpp						\
	func_hash_msdcc1.cpp					\
	func_hash_msdcc2.cpp					\
	func_pbkdf2_hmac_ms.cpp					\
	dpapi/module.hpp						\
	dpapi/module.cpp						\
	dpapi/blob.hpp						\
	dpapi/blob.cpp						\
	dpapi/credhist_entry.hpp					\
	dpapi/credhist_entry.cpp					\
	dpapi/credhist_file.hpp					\
	dpapi/credhist_file.cpp					\
	dpapi/master_key.hpp					\
	dpapi/master_key.cpp					\
	dpapi/master_key_file.hpp					\
	dpapi/master_key_file.cpp				\
	registry/module.hpp					\
	registry/module.cpp					\
	registry/registry.hpp					\
	registry/registry.cpp					\
	registry/registry_file.hpp				\
	registry/registry_file.cpp				\
	registry/registry_key.hpp					\
	registry/registry_key.cpp				\
	registry/registry_value.hpp				\
	registry/registry_value.cpp				\
	registry/registry_data.hpp				\
	registry/registry_data.cpp				\
	trashbin/module.hpp					\
	trashbin/module.cpp					\
	trashbin/ifile.hpp					\
	trashbin/ifile.cpp					\
	trashbin/info2_file.hpp					\
	trashbin/info2_file.cpp					\
	trashbin/info2_entry.hpp					\
	trashbin/info2_entry.cpp
