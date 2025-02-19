built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = part-catalogue.mobius
dist_noinst_DATA = *.py icon.png resource/*.svg

part-catalogue.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
