built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = imagefile-dnd.mobius
dist_noinst_DATA = *.py icon.png

imagefile-dnd.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip -r $@ $^
