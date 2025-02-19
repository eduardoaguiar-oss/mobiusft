built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = lshw-agent.mobius
dist_noinst_DATA = *.py icon.png

lshw-agent.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip -r $@ $^
