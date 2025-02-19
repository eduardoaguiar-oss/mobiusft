built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = report-model.mobius
dist_noinst_DATA = *.py icon.png

report-model.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip -r $@ $^
