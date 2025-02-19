built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = evidence-viewer.mobius
dist_noinst_DATA = *.py icon.png resource/*.png

evidence-viewer.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
