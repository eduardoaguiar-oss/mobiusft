built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = extension-manager.mobius
dist_noinst_DATA = *.py icon.png

extension-manager.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip -r $@ $^
