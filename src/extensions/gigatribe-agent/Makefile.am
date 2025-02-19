built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = gigatribe-agent.mobius
dist_noinst_DATA = *.py icon.png

gigatribe-agent.mobius:	$(dist_noinst_DATA)
	$(RM) -f $@
	zip $@ $^
