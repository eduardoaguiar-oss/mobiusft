built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = category-manager.mobius
dist_noinst_DATA = *.py icon.png

category-manager.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip -r $@ $^
