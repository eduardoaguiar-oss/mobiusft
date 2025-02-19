built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = sdi-window-manager.mobius
dist_noinst_DATA = *.py icon.png

sdi-window-manager.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip -r $@ $^
