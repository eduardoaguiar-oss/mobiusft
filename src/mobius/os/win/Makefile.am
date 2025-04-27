mobius_os_win_includedir = $(includedir)/mobius/os/win
nobase_mobius_os_win_include_HEADERS = 			\
	*.h						\
       	dpapi/*.h					\
	registry/*.h					\
	trashbin/*.h

noinst_LTLIBRARIES = libmobius-os-win.la
libmobius_os_win_la_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
libmobius_os_win_la_LDFLAGS = -avoid-version

libmobius_os_win_la_SOURCES =				\
	credential.cc					\
	hash_ie_entropy.cc				\
	hash_lm.cc					\
	hash_msdcc1.cc					\
	hash_msdcc2.cc					\
	hash_nt.cc					\
	pbkdf2_hmac_ms.cc				\
	dpapi/blob.cc					\
	dpapi/cipher_info.cc				\
	dpapi/credhist_entry.cc				\
	dpapi/credhist_file.cc				\
	dpapi/hash_info.cc				\
	dpapi/master_key.cc				\
	dpapi/master_key_file.cc			\
	registry/hive_file.cc				\
	registry/hive_key.cc				\
	registry/hive_value.cc				\
	registry/hive_data.cc				\
	registry/hive_decoder.cc			\
	registry/registry.cc				\
	registry/registry_file.cc			\
	registry/registry_key.cc			\
	registry/registry_key_impl_hkey.cc		\
	registry/registry_key_impl_container.cc		\
	registry/registry_key_impl_lsa.cc		\
	registry/registry_key_impl_link.cc		\
	registry/registry_key_impl_msdcc.cc		\
	registry/registry_key_impl_pssp.cc		\
	registry/registry_key_impl_user_assist.cc	\
	registry/registry_key_list.cc			\
	registry/registry_value.cc			\
	registry/registry_value_impl_container.cc	\
	registry/registry_value_impl_hvalue.cc		\
	registry/registry_data.cc			\
	registry/registry_data_impl_hdata.cc		\
	registry/registry_data_impl_lsa_polseckey.cc	\
	registry/registry_data_impl_lsa_poleklist.cc	\
	registry/registry_data_impl_msdcc.cc		\
	registry/registry_data_impl_pssp.cc		\
	registry/registry_set_lsa_keys.cc		\
	registry/registry_set_msdcc_key.cc		\
	registry/registry_set_pssp_keys.cc		\
	registry/registry_set_user_assist_keys.cc	\
	registry/pssp_data2.cc				\
	trashbin/ifile.cc				\
	trashbin/info2_file.cc				\
	trashbin/info2_entry.cc
