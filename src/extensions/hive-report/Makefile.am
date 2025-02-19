built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = hive-report.mobius
dist_noinst_DATA = *.py icon.png report/*.py

hive-report.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
