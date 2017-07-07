bin_PROGRAMS = xyzcrush

xyzcrush_SOURCES = src/xyzcrush.cpp \
	src/zopfli/blocksplitter.c \
	src/zopfli/cache.c \
	src/zopfli/deflate.c \
	src/zopfli/gzip_container.c \
	src/zopfli/hash.c \
	src/zopfli/katajainen.c \
	src/zopfli/lz77.c \
	src/zopfli/squeeze.c \
	src/zopfli/tree.c \
	src/zopfli/util.c \
	src/zopfli/zlib_container.c \
	src/zopfli/zopfli_lib.c

xyzcrush_CXXFLAGS = \
	$(ZLIB_CFLAGS)
xyzcrush_LDADD = \
	$(ZLIB_LIBS)

EXTRA_DIST = README.md
