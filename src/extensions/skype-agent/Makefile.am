built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = skype-agent.mobius
dist_noinst_DATA = *.py icon.png resource/*.png

skype-agent.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
