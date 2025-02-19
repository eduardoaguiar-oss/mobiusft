benchmark_PROGRAMS = benchmark
benchmarkdir = $(datadir)/$(PACKAGE)/tools

benchmark_SOURCES =			\
	benchmark.cc			\
	crypt.cc			\
	decoder.cc			\
	imagefile.cc

benchmark_CXXFLAGS = -I$(top_srcdir)/src
benchmark_LDADD = $(top_srcdir)/src/mobius/libmobius.la			\
	$(top_srcdir)/src/mobius/framework/libmobius_framework.la
