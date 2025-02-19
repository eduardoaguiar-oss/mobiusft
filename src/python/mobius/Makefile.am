pkgpyexecdir = $(pyexecdir)
pkgpyexec_LTLIBRARIES = mobius.la
mobiusdir=$(pkgpyexecdir)

SUBDIRS =								\
	core								\
	crypt								\
	database							\
	datasource							\
	datetime							\
	decoder								\
	encoder								\
	decoder								\
	framework							\
	kff								\
	io								\
	model								\
	os								\
	pod								\
	system								\
	turing								\
	ui								\
	vfs

mobius_la_SOURCES =							\
	*.h								\
	module.cc							\
	pycallback.cc							\
	pyfunction.cc							\
	pymodule.cc							\
	pyobject.cc							\
	pymobius.cc							\
	pytypeobject.cc							\
	api_dataholder.cc						\
	api_metadata.cc

mobius_la_CXXFLAGS = $(PY_CXXFLAGS) -I$(top_srcdir)/src -I$(top_srcdir)/src/python/mobius -fno-strict-aliasing

mobius_la_LDFLAGS = $(PY_LIBS) -module -avoid-version -shared -L$(top_srcdir)/src/mobius -L$(top_srcdir)/src/mobius/framework -lmobius -lmobius_framework -export-symbols-regex PyInit_mobius

mobius_la_LIBADD =								\
		core/libpymobius-core.la					\
		crypt/libpymobius-crypt.la					\
		database/libpymobius-database.la				\
		datasource/libpymobius-datasource.la				\
		datetime/libpymobius-datetime.la				\
		decoder/libpymobius-decoder.la					\
		encoder/libpymobius-encoder.la					\
		framework/libpymobius-framework.la				\
		kff/libpymobius-kff.la						\
		io/libpymobius-io.la						\
		model/libpymobius-model.la					\
		os/libpymobius-os.la						\
		pod/libpymobius-pod.la						\
		system/libpymobius-system.la					\
		turing/libpymobius-turing.la					\
		ui/libpymobius-ui.la						\
		vfs/libpymobius-vfs.la
