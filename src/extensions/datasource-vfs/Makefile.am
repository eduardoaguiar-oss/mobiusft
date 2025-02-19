built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = datasource-vfs.mobius
dist_noinst_DATA = *.py icon.png resource/*.png

datasource-vfs.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
