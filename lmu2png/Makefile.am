bin_PROGRAMS = lmu2png
lmu2png_SOURCES = \
	src/main.cpp \
	src/chipset.cpp \
	src/sdlxyz.cpp \
	src/sdlxyz.h \
	src/chipset.h
lmu2png_CXXFLAGS = \
	-std=c++11 \
	$(LCF_CFLAGS) \
	$(SDL2_IMAGE_CFLAGS) \
	$(ZLIB_CFLAGS)
lmu2png_LDADD = \
	$(LCF_LIBS) \
	$(SDL2_IMAGE_LIBS) \
	$(ZLIB_LIBS)

EXTRA_DIST = README.md
