built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = iped-frontend.mobius
dist_noinst_DATA = *.py icon.png resource/*.png resource/*.zip

iped-frontend.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip -r $@ $^
