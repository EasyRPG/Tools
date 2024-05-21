argparsedir = src/external/argparse

EXTRA_DIST = README.md AUTHORS.md \
	CMakeLists.txt \
	CMakeModules/ConfigureWindows.cmake \
	CMakeModules/FindICU.cmake \
	CMakeModules/FindSDL2.cmake \
	CMakeModules/FindSDL2_image.cmake \
	$(argparsedir)

bin_PROGRAMS = lmu2png
lmu2png_SOURCES = \
	src/main.h \
	src/main.cpp \
	src/chipset.h \
	src/chipset.cpp \
	src/sdlxyz.h \
	src/sdlxyz.cpp \
	src/utils.h \
	src/utils.cpp \
	$(argparsedir)/argparse.hpp
lmu2png_CXXFLAGS = \
	-std=c++17 \
	-I$(srcdir)/$(argparsedir) \
	$(LCF_CFLAGS) \
	$(SDL2_IMAGE_CFLAGS) \
	$(ZLIB_CFLAGS)
lmu2png_LDADD = \
	$(LCF_LIBS) \
	$(SDL2_IMAGE_LIBS) \
	$(ZLIB_LIBS)
