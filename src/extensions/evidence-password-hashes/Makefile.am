built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = evidence-password-hashes.mobius
dist_noinst_DATA = *.py icon.png resource/*.png

evidence-password-hashes.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip -r $@ $^
