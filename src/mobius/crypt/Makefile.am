mobius_crypt_includedir = $(includedir)/mobius/crypt
nobase_mobius_crypt_include_HEADERS = *.h gcrypt/*.h

noinst_LTLIBRARIES = libmobius-crypt.la

libmobius_crypt_la_CXXFLAGS =						\
	-I$(top_srcdir)/src						\
	$(LIBGCRYPT_CFLAGS)

libmobius_crypt_la_SOURCES =						\
	cipher.cc							\
	cipher_impl_block.cc						\
	cipher_impl_stream.cc						\
	cipher_impl_ige.cc						\
	cipher_impl_zip.cc						\
	crc32.cc							\
	hash.cc								\
	hash_impl_adler32.cc						\
	hash_impl_ed2k.cc						\
	hash_impl_zip.cc						\
	hash_digest.cc							\
	hmac.cc								\
	hmac_impl_default.cc						\
	pkcs5.cc							\
	rot13.cc							\
	gcrypt/cipher_impl.cc						\
	gcrypt/hash_impl.cc						\
	gcrypt/hmac_impl.cc						\
	gcrypt/util.cc

libmobius_crypt_la_LDFLAGS = -avoid-version $(LIBGCRYPT_LIBS)
