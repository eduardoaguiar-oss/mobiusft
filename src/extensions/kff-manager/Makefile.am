built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = kff-manager.mobius
dist_noinst_DATA = *.py icon.png

kff-manager.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip -r $@ $^
