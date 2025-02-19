built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = p2p-viewer.mobius
dist_noinst_DATA = *.py icon.png resource/*.png

p2p-viewer.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
