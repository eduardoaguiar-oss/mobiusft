built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = file-explorer.mobius
dist_noinst_DATA = *.py icon.png resource/*.png resource/*.svg

file-explorer.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
