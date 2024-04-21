EXTRA_DIST = README.md \
	CMakeLists.txt CMakeModules/ConfigureWindows.cmake \
	src/external/zopfli/COPYING

bin_PROGRAMS = xyzcrush
xyzcrush_SOURCES = \
	src/xyzcrush.cpp \
	src/external/zopfli/zopfli.h \
	src/external/zopfli/blocksplitter.c \
	src/external/zopfli/blocksplitter.h \
	src/external/zopfli/cache.c \
	src/external/zopfli/cache.h \
	src/external/zopfli/deflate.c \
	src/external/zopfli/deflate.h \
	src/external/zopfli/hash.c \
	src/external/zopfli/hash.h \
	src/external/zopfli/katajainen.c \
	src/external/zopfli/katajainen.h \
	src/external/zopfli/lz77.c \
	src/external/zopfli/lz77.h \
	src/external/zopfli/squeeze.c \
	src/external/zopfli/squeeze.h \
	src/external/zopfli/symbols.h \
	src/external/zopfli/tree.c \
	src/external/zopfli/tree.h \
	src/external/zopfli/util.c \
	src/external/zopfli/util.h \
	src/external/zopfli/zlib_container.c \
	src/external/zopfli/zlib_container.h
xyzcrush_CXXFLAGS = $(ZLIB_CFLAGS) -Isrc/external/zopfli
xyzcrush_LDADD = $(ZLIB_LIBS)
