built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = gtk-ui.mobius
dist_noinst_DATA = *.py icon.png resource/*.png

gtk-ui.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
