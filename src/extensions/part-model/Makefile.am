built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = part-model.mobius
dist_noinst_DATA = *.py icon.png

part-model.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
