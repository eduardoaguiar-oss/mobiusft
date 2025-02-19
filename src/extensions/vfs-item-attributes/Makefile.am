built_extensiondir = $(datadir)/$(PACKAGE)/extensions
built_extension_DATA = vfs-item-attributes.mobius
dist_noinst_DATA = *.py icon.png

vfs-item-attributes.mobius: $(dist_noinst_DATA)
	$(RM) -f $@
	zip -r $@ $^
