built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = skype-agent-sqlite.mobius
dist_noinst_DATA = *.py icon.png

skype-agent-sqlite.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
