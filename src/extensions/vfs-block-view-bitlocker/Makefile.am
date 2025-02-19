built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = vfs-block-view-bitlocker.mobius
dist_noinst_DATA = *.py icon.png resource/*.png

vfs-block-view-bitlocker.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
