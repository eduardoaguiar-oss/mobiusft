extensionsdir = $(datadir)/$(PACKAGE)/extensions
EXTENSIONS_BUILD_DIR = build

# Build rule
all-local:
	@mkdir -p $(EXTENSIONS_BUILD_DIR)
	@cd $(EXTENSIONS_BUILD_DIR) && \
	echo "CMake command: $(CMAKE) -DCMAKE_INSTALL_PREFIX=$(prefix) -DLIBMOBIUS_PATH=$(top_builddir)/src/libmobius $(srcdir)" && \
		$(CMAKE) -DCMAKE_INSTALL_PREFIX=$(prefix) \
		         .. && \
		$(MAKE)

# Install rule
install-exec-local:
	@cd $(EXTENSIONS_BUILD_DIR) && $(MAKE) install DESTDIR=$(DESTDIR)

# Clean rule
clean-local:
	@rm -rf $(EXTENSIONS_BUILD_DIR)

# Distribution files (for make dist)
# @todo Can be removed when fully migrated to CMake
EXTRA_DIST =								\
	CMakeLists.txt							\
	app/ares/CMakeLists.txt						\
	app/ares/*.hpp							\
	app/ares/*.cpp							\
	app/emule/CMakeLists.txt					\
	app/emule/*.hpp							\
	app/emule/*.cpp							\
	app/emule/tools/*.cpp						\
	app/emuletorrent/CMakeLists.txt					\
	app/emuletorrent/*.hpp						\
	app/emuletorrent/*.cpp						\
	app/shareaza/CMakeLists.txt					\
	app/shareaza/*.hpp						\
	app/shareaza/*.cpp						\
	app/shareaza/tools/*.cpp					\
	attribute-viewer/CMakeLists.txt					\
	attribute-viewer/*.py						\
	attribute-viewer/icon.png					\
	category-manager/CMakeLists.txt					\
	category-manager/*.py						\
	category-manager/icon.png					\
	datasource-viewer/CMakeLists.txt				\
	datasource-viewer/*.py						\
	datasource-viewer/icon.png					\
	datasource-ufdr/CMakeLists.txt					\
	datasource-ufdr/*.py						\
	datasource-ufdr/icon.png					\
	datasource-ufdr/resource/*.png					\
	datasource-vfs/CMakeLists.txt					\
	datasource-vfs/*.py						\
	datasource-vfs/icon.png						\
	datasource-vfs/resource/*.png					\
	date-code/CMakeLists.txt					\
	date-code/main.cpp						\
	event-viewer/CMakeLists.txt					\
	event-viewer/*.py						\
	event-viewer/icon.png						\
	evidence-password-hashes/CMakeLists.txt				\
	evidence-password-hashes/*.py					\
	evidence-password-hashes/icon.png				\
	evidence-password-hashes/resource/*.png				\
	evidence-viewer/CMakeLists.txt					\
	evidence-viewer/*.py						\
	evidence-viewer/icon.png					\
	evidence-viewer/resource/*.png					\
	extension-manager/CMakeLists.txt				\
	extension-manager/*.py						\
	extension-manager/icon.png					\
	file-explorer/CMakeLists.txt					\
	file-explorer/*.py						\
	file-explorer/icon.png						\
	file-explorer/resource/*.png					\
	file-explorer/resource/*.svg					\
	floppy-imager/CMakeLists.txt					\
	floppy-imager/*.py						\
	floppy-imager/icon.png						\
	gigatribe-agent/CMakeLists.txt					\
	gigatribe-agent/*.py						\
	gigatribe-agent/icon.png					\
	gtk-ui/CMakeLists.txt						\
	gtk-ui/*.py							\
	gtk-ui/icon.png							\
	gtk-ui/resource/*.png						\
	hive/CMakeLists.txt						\
	hive/*.py							\
	hive/icon.png							\
	hive/resource/*.png						\
	hive-report/CMakeLists.txt					\
	hive-report/*.py						\
	hive-report/icon.png						\
	hive-report/report/*.py						\
	ice/CMakeLists.txt						\
	ice/*.py							\
	ice/icon.png							\
	ice/resource/*.png						\
	ice/resource/*.svg						\
	ice/resource/*.txt						\
	imagefile-dnd/CMakeLists.txt					\
	imagefile-dnd/*.py						\
	imagefile-dnd/icon.png						\
	iped-frontend/CMakeLists.txt					\
	iped-frontend/*.py						\
	iped-frontend/icon.png						\
	iped-frontend/resource/*.png					\
	iped-frontend/resource/*.zip					\
	kff-manager/CMakeLists.txt					\
	kff-manager/*.py						\
	kff-manager/icon.png						\
	lshw-agent/CMakeLists.txt					\
	lshw-agent/*.py							\
	lshw-agent/icon.png						\
	part-catalogue/CMakeLists.txt					\
	part-catalogue/*.py						\
	part-catalogue/icon.png						\
	part-catalogue/resource/*.svg					\
	part-model/CMakeLists.txt					\
	part-model/*.py							\
	part-model/icon.png						\
	report-model/CMakeLists.txt					\
	report-model/*.py						\
	report-model/icon.png						\
	report-wizard/CMakeLists.txt					\
	report-wizard/*.py						\
	report-wizard/icon.png						\
	sdi-window-manager/CMakeLists.txt				\
	sdi-window-manager/*.py						\
	sdi-window-manager/icon.png					\
	skype-agent/CMakeLists.txt					\
	skype-agent/*.py						\
	skype-agent/icon.png						\
	skype-agent/resource/*.png					\
	skype-agent-sqlite/CMakeLists.txt				\
	skype-agent-sqlite/*.py						\
	skype-agent-sqlite/icon.png					\
	turing/CMakeLists.txt						\
	turing/*.py							\
	turing/icon.png							\
	turing/resource/*.png						\
	ui/gtk3/CMakeLists.txt						\
	ui/gtk3/*.hpp							\
	ui/gtk3/*.cpp							\
	vfs-block-view-bitlocker/CMakeLists.txt				\
	vfs-block-view-bitlocker/*.py					\
	vfs-block-view-bitlocker/icon.png				\
	vfs-block-view-bitlocker/resource/*.png				\
	vfs-block-view-partition-system/CMakeLists.txt			\
	vfs-block-view-partition-system/*.py				\
	vfs-block-view-partition-system/icon.png			\
	vfs-block-view-partition-system/resource/*.png			\
	vfs-item-attributes/CMakeLists.txt				\
	vfs-item-attributes/*.py					\
	vfs-item-attributes/icon.png					\
	vfs/block/*/CMakeLists.txt					\
	vfs/block/*/*.cpp						\
	vfs/filesystem/*/CMakeLists.txt					\
	vfs/filesystem/*/*.hpp						\
	vfs/filesystem/*/*.cpp						\
	vfs/imagefile/*/CMakeLists.txt					\
	vfs/imagefile/*/*.hpp						\
	vfs/imagefile/*/*.cpp						\
	vfs/imagefile/ewf/tools/*.cpp
