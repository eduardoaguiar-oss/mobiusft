built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = attribute-viewer.mobius
dist_noinst_DATA = *.py icon.png

attribute-viewer.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
