built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = turing.mobius
dist_noinst_DATA = *.py icon.png resource/*.png

turing.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
