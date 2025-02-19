built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = event-viewer.mobius
dist_noinst_DATA = *.py icon.png

event-viewer.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
