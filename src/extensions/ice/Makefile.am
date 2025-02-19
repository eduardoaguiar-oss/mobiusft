built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = ice.mobius
dist_noinst_DATA = *.py icon.png resource/gpl.txt resource/*.png resource/*.svg

ice.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
