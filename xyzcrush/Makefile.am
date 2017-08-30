bin_PROGRAMS = xyzcrush

xyzcrush_SOURCES = src/xyzcrush.cpp \
	src/zopfli/zopfli.h \
	src/zopfli/blocksplitter.c \
	src/zopfli/blocksplitter.h \
	src/zopfli/cache.c \
	src/zopfli/cache.h \
	src/zopfli/deflate.c \
	src/zopfli/deflate.h \
	src/zopfli/gzip_container.c \
	src/zopfli/gzip_container.h \
	src/zopfli/hash.c \
	src/zopfli/hash.h \
	src/zopfli/katajainen.c \
	src/zopfli/katajainen.h \
	src/zopfli/lz77.c \
	src/zopfli/lz77.h \
	src/zopfli/squeeze.c \
	src/zopfli/squeeze.h \
	src/zopfli/symbols.h \
	src/zopfli/tree.c \
	src/zopfli/tree.h \
	src/zopfli/util.c \
	src/zopfli/util.h \
	src/zopfli/zlib_container.c \
	src/zopfli/zlib_container.h \
	src/zopfli/zopfli_lib.c

xyzcrush_CXXFLAGS = \
	$(ZLIB_CFLAGS)
xyzcrush_LDADD = \
	$(ZLIB_LIBS)

EXTRA_DIST = README.md
