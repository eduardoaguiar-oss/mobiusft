built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = datasource-viewer.mobius
dist_noinst_DATA = *.py icon.png

datasource-viewer.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
