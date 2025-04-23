# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#pkgpyexecdir = $(pyexecdir)
#pkgpyexec_LTLIBRARIES = mobius.la
#mobiusdir = $(pkgpyexecdir)

CMAKE_BUILD_DIR = build

all-local:
	@if [ -f CMakeLists.txt ]; then \
		mkdir -p $(CMAKE_BUILD_DIR); \
		cd $(CMAKE_BUILD_DIR) && \
		$(CMAKE) -DCMAKE_INSTALL_PREFIX=$(prefix) -DCMAKE_BUILD_TYPE=Release .. && \
		$(MAKE); \
	fi

install-exec-local:
	@if [ -d $(CMAKE_BUILD_DIR) ]; then \
		cd $(CMAKE_BUILD_DIR) && $(MAKE) install DESTDIR=$(DESTDIR); \
	fi

clean-local:
	@rm -rf $(CMAKE_BUILD_DIR)

EXTRA_DIST = 								\
    api_dataholder.cpp							\
    api_dataholder.hpp							\
    api_metadata.cpp							\
    api_metadata.hpp							\
    CMakeLists.txt							\
    core/application.cpp						\
    core/application.hpp						\
    core/CMakeLists.txt							\
    core/extension.cpp							\
    core/extension.hpp							\
    core/file_decoder/decoder.cpp					\
    core/file_decoder/decoder.hpp					\
    core/file_decoder/entry.cpp						\
    core/file_decoder/entry.hpp						\
    core/file_decoder/metadata.cpp					\
    core/file_decoder/metadata.hpp					\
    core/file_decoder/module.cpp					\
    core/file_decoder/module.hpp					\
    core/file_decoder/section.cpp					\
    core/file_decoder/section.hpp					\
    core/func_logf.cpp							\
    core/func_mediator.cpp						\
    core/func_resource.cpp						\
    core/func_set_logfile_path.cpp					\
    core/log.cpp							\
    core/log.hpp							\
    core/module.cpp							\
    core/module.hpp							\
    core/resource.cpp							\
    core/resource.hpp							\
    core/thread_guard.cpp						\
    core/thread_guard.hpp						\
    crypt/cipher.cpp							\
    crypt/cipher.hpp							\
    crypt/CMakeLists.txt						\
    crypt/func_cipher.cpp						\
    crypt/func_hash_digest.cpp						\
    crypt/func_pbkdf1.cpp						\
    crypt/func_pbkdf2_hmac.cpp						\
    crypt/func_rot13.cpp						\
    crypt/hash.cpp							\
    crypt/hash.hpp							\
    crypt/hmac.cpp							\
    crypt/hmac.hpp							\
    crypt/module.cpp							\
    crypt/module.hpp							\
    database/CMakeLists.txt						\
    database/connection.cpp						\
    database/connection.hpp						\
    database/connection_set.cpp						\
    database/connection_set.hpp						\
    database/module.cpp							\
    database/module.hpp							\
    database/transaction.cpp						\
    database/transaction.hpp						\
    datasource/datasource.cpp						\
    datasource/datasource.hpp						\
    datasource/datasource_vfs.cpp					\
    datasource/datasource_vfs.hpp					\
    datasource/func_datasource.cpp					\
    datasource/module.cpp						\
    datasource/module.hpp						\
    datasource/ufdr/datasource.cpp					\
    datasource/ufdr/datasource.hpp					\
    datasource/ufdr/extraction.cpp					\
    datasource/ufdr/extraction.hpp					\
    datasource/ufdr/module.cpp						\
    datasource/ufdr/module.hpp						\
    datetime/CMakeLists.txt						\
    datetime/func_new_datetime_from_fat_time.cpp			\
    datetime/func_new_datetime_from_iso_string.cpp			\
    datetime/func_new_datetime_from_nt_timestamp.cpp			\
    datetime/func_new_datetime_from_unix_timestamp.cpp			\
    datetime/module.cpp							\
    datetime/module.hpp							\
    decoder/CMakeLists.txt						\
    decoder/data_decoder.cpp						\
    decoder/data_decoder.hpp						\
    decoder/func_base16.cpp						\
    decoder/func_base32.cpp						\
    decoder/func_base32hex.cpp						\
    decoder/func_base64.cpp						\
    decoder/func_base64url.cpp						\
    decoder/func_btencode.cpp						\
    decoder/func_get_filetype.cpp					\
    decoder/inifile.cpp							\
    decoder/inifile.hpp							\
    decoder/lnk.cpp							\
    decoder/lnk.hpp							\
    decoder/mfc.cpp							\
    decoder/mfc.hpp							\
    decoder/module.cpp							\
    decoder/module.hpp							\
    decoder/qdatastream.cpp						\
    decoder/qdatastream.hpp						\
    decoder/sgml/module.cpp						\
    decoder/sgml/module.hpp						\
    decoder/sgml/parser.cpp						\
    decoder/sgml/parser.hpp						\
    decoder/sgml/tokenizer.cpp						\
    decoder/sgml/tokenizer.hpp						\
    decoder/sourcecode.cpp						\
    decoder/sourcecode.hpp						\
    decoder/tdf.cpp							\
    decoder/tdf.hpp							\
    decoder/xml/dom.cpp							\
    decoder/xml/dom.hpp							\
    decoder/xml/element.cpp						\
    decoder/xml/element.hpp						\
    decoder/xml/module.cpp						\
    decoder/xml/module.hpp						\
    encoder/CMakeLists.txt						\
    encoder/func_base16.cpp						\
    encoder/func_base32.cpp						\
    encoder/func_base32hex.cpp						\
    encoder/func_base64.cpp						\
    encoder/func_base64url.cpp						\
    encoder/func_hexstring.cpp						\
    encoder/module.cpp							\
    encoder/module.hpp							\
    framework/attribute.cpp						\
    framework/attribute.hpp						\
    framework/category.cpp						\
    framework/category.hpp						\
    framework/CMakeLists.txt						\
    framework/evidence_loader.cpp					\
    framework/evidence_loader.hpp					\
    framework/func_config.cpp						\
    framework/module.cpp						\
    framework/module.hpp						\
    io/CMakeLists.txt							\
    io/entry.cpp							\
    io/entry.hpp							\
    io/file.cpp								\
    io/file.hpp								\
    io/folder.cpp							\
    io/folder.hpp							\
    io/func_get_current_folder.cpp					\
    io/func_join_path.cpp						\
    io/func_new_entry_by_path.cpp					\
    io/func_new_entry_by_url.cpp					\
    io/func_new_file_by_path.cpp					\
    io/func_new_file_by_url.cpp						\
    io/func_new_folder_by_path.cpp					\
    io/func_new_folder_by_url.cpp					\
    io/func_new_path_from_win.cpp					\
    io/func_new_uri_from_path.cpp					\
    io/func_reader.cpp							\
    io/func_to_win_path.cpp						\
    io/line_reader.cpp							\
    io/line_reader.hpp							\
    io/module.cpp							\
    io/module.hpp							\
    io/path.cpp								\
    io/path.hpp								\
    io/reader.cpp							\
    io/reader.hpp							\
    io/sequential_reader_adaptor.cpp					\
    io/sequential_reader_adaptor.hpp					\
    io/stream.cpp							\
    io/stream.hpp							\
    io/tempfile.cpp							\
    io/tempfile.hpp							\
    io/text_reader.cpp							\
    io/text_reader.hpp							\
    io/text_writer.cpp							\
    io/text_writer.hpp							\
    io/uri.cpp								\
    io/uri.hpp								\
    io/walker.cpp							\
    io/walker.hpp							\
    io/writer.cpp							\
    io/writer.hpp							\
    kff/CMakeLists.txt							\
    kff/hashset.cpp							\
    kff/hashset.hpp							\
    kff/kff.cpp								\
    kff/kff.hpp								\
    kff/module.cpp							\
    kff/module.hpp							\
    model/ant.cpp							\
    model/ant.hpp							\
    model/case.cpp							\
    model/case.hpp							\
    model/CMakeLists.txt						\
    model/event.cpp							\
    model/event.hpp							\
    model/evidence.cpp							\
    model/evidence.hpp							\
    model/func_case.cpp							\
    model/item.cpp							\
    model/item.hpp							\
    model/module.cpp							\
    model/module.hpp							\
    module.cpp								\
    module.hpp								\
    os/CMakeLists.txt							\
    os/module.cpp							\
    os/module.hpp							\
    os/win/credential.cpp						\
    os/win/credential.hpp						\
    os/win/dpapi/blob.cpp						\
    os/win/dpapi/blob.hpp						\
    os/win/dpapi/credhist_entry.cpp					\
    os/win/dpapi/credhist_entry.hpp					\
    os/win/dpapi/credhist_file.cpp					\
    os/win/dpapi/credhist_file.hpp					\
    os/win/dpapi/master_key.cpp						\
    os/win/dpapi/master_key_file.cpp					\
    os/win/dpapi/master_key_file.hpp					\
    os/win/dpapi/master_key.hpp						\
    os/win/dpapi/module.cpp						\
    os/win/dpapi/module.hpp						\
    os/win/func_hash_ie_entropy.cpp					\
    os/win/func_hash_lm.cpp						\
    os/win/func_hash_msdcc1.cpp						\
    os/win/func_hash_msdcc2.cpp						\
    os/win/func_hash_nt.cpp						\
    os/win/func_pbkdf2_hmac_ms.cpp					\
    os/win/module.cpp							\
    os/win/module.hpp							\
    os/win/registry/module.cpp						\
    os/win/registry/module.hpp						\
    os/win/registry/registry.cpp					\
    os/win/registry/registry_data.cpp					\
    os/win/registry/registry_data.hpp					\
    os/win/registry/registry_file.cpp					\
    os/win/registry/registry_file.hpp					\
    os/win/registry/registry.hpp					\
    os/win/registry/registry_key.cpp					\
    os/win/registry/registry_key.hpp					\
    os/win/registry/registry_value.cpp					\
    os/win/registry/registry_value.hpp					\
    os/win/trashbin/ifile.cpp						\
    os/win/trashbin/ifile.hpp						\
    os/win/trashbin/info2_entry.cpp					\
    os/win/trashbin/info2_entry.hpp					\
    os/win/trashbin/info2_file.cpp					\
    os/win/trashbin/info2_file.hpp					\
    os/win/trashbin/module.cpp						\
    os/win/trashbin/module.hpp						\
    pod/CMakeLists.txt							\
    pod/data.cpp							\
    pod/data.hpp							\
    pod/func_serialize.cpp						\
    pod/func_unserialize.cpp						\
    pod/map.cpp								\
    pod/map.hpp								\
    pod/module.cpp							\
    pod/module.hpp							\
    pycallback.cpp							\
    pycallback.hpp							\
    pydict.hpp								\
    pyfunction.cpp							\
    pyfunction.hpp							\
    pygil.hpp								\
    pylist.hpp								\
    pymobius.cpp							\
    pymobius.hpp							\
    pymodule.cpp							\
    pymodule.hpp							\
    pyobject.cpp							\
    pyobject.hpp							\
    pypredicate.hpp							\
    pyset.hpp								\
    pytypeobject.cpp							\
    pytypeobject.hpp							\
    system/CMakeLists.txt						\
    system/device.cpp							\
    system/device.hpp							\
    system/func_get_devices.cpp						\
    system/module.cpp							\
    system/module.hpp							\
    turing/CMakeLists.txt						\
    turing/module.cpp							\
    turing/module.hpp							\
    turing/turing.cpp							\
    turing/turing.hpp							\
    ui/box.cpp								\
    ui/box.hpp								\
    ui/button.cpp							\
    ui/button.hpp							\
    ui/CMakeLists.txt							\
    ui/container.cpp							\
    ui/container.hpp							\
    ui/func_ui.cpp							\
    ui/icon.cpp								\
    ui/icon.hpp								\
    ui/label.cpp							\
    ui/label.hpp							\
    ui/message_dialog.cpp						\
    ui/message_dialog.hpp						\
    ui/module.cpp							\
    ui/module.hpp							\
    ui/stacked_container.cpp						\
    ui/stacked_container.hpp						\
    ui/widget.cpp							\
    ui/widget.hpp							\
    ui/window.cpp							\
    ui/window.hpp							\
    vfs/block.cpp							\
    vfs/block.hpp							\
    vfs/CMakeLists.txt							\
    vfs/disk.cpp							\
    vfs/disk.hpp							\
    vfs/filesystem.cpp							\
    vfs/filesystem.hpp							\
    vfs/func_disk.cpp							\
    vfs/func_get_imagefile_implementations.cpp				\
    vfs/func_new_imagefile_by_path.cpp					\
    vfs/func_new_imagefile_by_url.cpp					\
    vfs/func_new_imagefile_from_file.cpp				\
    vfs/imagefile.cpp							\
    vfs/imagefile.hpp							\
    vfs/module.cpp							\
    vfs/module.hpp							\
    vfs/vfs.cpp								\
    vfs/vfs.hpp
