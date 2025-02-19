built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = floppy-imager.mobius
dist_noinst_DATA = *.py icon.png

floppy-imager.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip -r $@ $^
