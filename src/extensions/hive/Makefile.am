built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = hive.mobius
dist_noinst_DATA = *.py icon.png resource/*.png

hive.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
